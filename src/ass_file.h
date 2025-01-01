// Copyright (c) 2005, Rodrigo Braz Monteiro
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of the Aegisub Group nor the names of its contributors
//     may be used to endorse or promote products derived from this software
//     without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// Aegisub Project http://www.aegisub.org/

#include "ass_entry.h"

#include <libaegisub/fs.h>
#include <libaegisub/signal.h>

#include <boost/intrusive/list.hpp>
#include <map>
#include <set>
#include <vector>

class AssAttachment;
class AssDialogue;
class AssInfo;
class AssStyle;
class wxString;

template<typename T>
using EntryList = typename boost::intrusive::make_list<T, boost::intrusive::constant_time_size<false>, boost::intrusive::base_hook<AssEntryListHook>>::type;

struct ExtradataEntry {
	uint32_t id;
	std::string key;
	std::string value;
};

struct AssFileCommit {
	wxString const& message;
	int *commit_id;
	AssDialogue *single_line;
};

struct ProjectProperties {
	std::string automation_scripts;
	std::string export_filters;
	std::string export_encoding;
	std::string style_storage;
	std::string audio_file;
	std::string video_file;
	std::string timecodes_file;
	std::string keyframes_file;
	std::map<std::string, std::string> automation_settings;

	// UI State
	double video_zoom = 0.;
	double ar_value = 0.;
	int scroll_position = 0;
	int active_row = 0;
	int ar_mode = 0;
	int video_position = 0;
};

class AssFile {
	/// A set of changes has been committed to the file (AssFile::COMMITType)
	agi::signal::Signal<int, const AssDialogue*> AnnounceCommit;
	agi::signal::Signal<AssFileCommit> PushState;
public:
	/// The lines in the file
	std::vector<AssInfo> Info;
	EntryList<AssStyle> Styles;
	EntryList<AssDialogue> Events;
	std::vector<AssAttachment> Attachments;
	std::vector<ExtradataEntry> Extradata;
	ProjectProperties Properties;

	uint32_t next_extradata_id = 0;

	AssFile();
	AssFile(const AssFile &from);
	AssFile& operator=(AssFile from);
	~AssFile();

	EntryList<AssDialogue>::iterator iterator_to(AssDialogue& line);

	/// @brief Load default file
	/// @param defline Add a blank line to the file
	/// @param style_catalog Style catalog name to fill styles from, blank to use default style
	void LoadDefault(bool defline = true, std::string const& style_catalog = std::string());
	/// Attach a file to the ass file
	void InsertAttachment(agi::fs::path const& filename);
	/// Get the names of all of the styles available
	std::vector<std::string> GetStyles() const;
	/// @brief Get a style by name
	/// @param name Style name
	/// @return Pointer to style or nullptr
	AssStyle *GetStyle(std::string const& name);

	void swap(AssFile &) throw();

	/// @brief Get the script resolution
	/// @param[out] w Width
	/// @param[in] h Height
	void GetResolution(int &w,int &h) const;
	/// Get the value in a [Script Info] key as int, or 0 if it is not present
	int GetScriptInfoAsInt(std::string_view key) const;
	/// Get the value in a [Script Info] key as string.
	std::string_view GetScriptInfo(std::string_view key) const;
	/// Set the value of a [Script Info] key. Adds it if it doesn't exist.
	void SetScriptInfo(std::string_view key, std::string_view value);

	/// @brief Add a new extradata entry
	/// @param key Class identifier/owner for the extradata
	/// @param value Data for the extradata
	/// @return ID of the created entry
	uint32_t AddExtradata(std::string_view key, std::string_view value);
	/// Fetch all extradata entries from a list of IDs
	std::vector<ExtradataEntry> GetExtradata(std::vector<uint32_t> const& id_list) const;
	/// Remove unreferenced extradata entries
	void CleanExtradata();

	/// Type of changes made in a commit
	enum CommitType {
		/// Potentially the entire file has been changed; any saved information
		/// should be discarded. Note that the active line and selected set
		/// should not be touched in handlers for this, as they may not have
		/// been updated yet
		/// Note that it is intentional that this cannot be combined with
		/// other commit types
		COMMIT_NEW         = 0,
		/// The order of lines in the file has changed
		COMMIT_ORDER       = 0x1,
		/// The script info section has changed in some way
		COMMIT_SCRIPTINFO  = 0x2,
		/// The styles have changed in some way
		COMMIT_STYLES      = 0x4,
		/// The attachments have changed in some way
		COMMIT_ATTACHMENT  = 0x8,
		/// Dialogue lines have been added or removed
		/// Note that if the active dialogue line was removed, the active line
		/// should be updated BEFORE committing
		COMMIT_DIAG_ADDREM = 0x10,
		/// The metadata fields of existing dialogue lines have changed
		COMMIT_DIAG_META   = 0x20,
		/// The start and/or end times of existing dialogue lines have changed
		COMMIT_DIAG_TIME   = 0x40,
		/// The text of existing dialogue lines have changed
		COMMIT_DIAG_TEXT   = 0x80,
		COMMIT_DIAG_FULL   = COMMIT_DIAG_META | COMMIT_DIAG_TIME | COMMIT_DIAG_TEXT,
		/// Extradata entries were added/modified/removed
		COMMIT_EXTRADATA   = 0x100,
	};

	DEFINE_SIGNAL_ADDERS(AnnounceCommit, AddCommitListener)
	DEFINE_SIGNAL_ADDERS(PushState, AddUndoManager)

	/// @brief Flag the file as modified and push a copy onto the undo stack
	/// @param desc        Undo description
	/// @param type        Type of changes made to the file in this commit
	/// @param commitId    Commit to amend rather than pushing a new commit
	/// @param single_line Line which was changed, if only one line was
	/// @return Unique identifier for the new undo group
	int Commit(wxString const& desc, int type, int commitId = -1, AssDialogue *single_line = nullptr);

	/// Comparison function for use when sorting
	typedef bool (*CompFunc)(AssDialogue const& lft, AssDialogue const& rgt);

	/// Compare based on start time
	static bool CompStart(AssDialogue const& lft, AssDialogue const& rgt);
	/// Compare based on end time
	static bool CompEnd(AssDialogue const& lft, AssDialogue const& rgt);
	/// Compare based on style name
	static bool CompStyle(AssDialogue const& lft, AssDialogue const& rgt);
	/// Compare based on actor name
	static bool CompActor(AssDialogue const& lft, AssDialogue const& rgt);
	/// Compare based on effect
	static bool CompEffect(AssDialogue const& lft, AssDialogue const& rgt);
	/// Compare based on layer
	static bool CompLayer(AssDialogue const& lft, AssDialogue const& rgt);

	/// @brief Sort the dialogue lines in this file
	/// @param comp Comparison function to use. Defaults to sorting by start time.
	/// @param limit If non-empty, only lines in this set are sorted
	void Sort(CompFunc comp = CompStart, std::set<AssDialogue*> const& limit = std::set<AssDialogue*>());
	/// @brief Sort the dialogue lines in the given list
	/// @param comp Comparison function to use. Defaults to sorting by start time.
	/// @param limit If non-empty, only lines in this set are sorted
	static void Sort(EntryList<AssDialogue>& lst, CompFunc comp = CompStart, std::set<AssDialogue*> const& limit = std::set<AssDialogue*>());
};
