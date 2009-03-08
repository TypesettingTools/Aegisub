// Copyright (c) 2008, Rodrigo Braz Monteiro
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
// -----------------------------------------------------------------------------
//
// AEGISUB/ATHENASUB
//
// Website: http://www.aegisub.net
// Contact: mailto:amz@aegisub.net
//

#pragma once

#include "tr1.h"
#include "athenatime.h"
#include "athenastring.h"
#include "colour.h"
#include <vector>
#include <list>

class wxInputStream;
class wxOutputStream;

namespace Athenasub {

	// Forward references
	class Range;
	class ISelection;
	class IController;
	class IView;
	class IFormatHandler;
	class IEntry;
	class IDialogue;
	class IStyle;
	class IFormat;
	class IModel;
	class IActionList;
	class INotification;
	class ISection;
	class IDeltaCoder;
	class IAction;
	class IReader;
	class IWriter;


	// Smart pointers
	typedef shared_ptr<void> VoidPtr;
	typedef shared_ptr<ISelection> Selection;
	typedef shared_ptr<IController> Controller;
	typedef shared_ptr<IView> View;
	typedef shared_ptr<IFormatHandler> FormatHandler;
	typedef shared_ptr<IEntry> Entry;
	typedef shared_ptr<IDialogue> Dialogue;
	typedef shared_ptr<IStyle> Style;
	typedef shared_ptr<IFormat> Format;
	typedef shared_ptr<IModel> Model;
	typedef shared_ptr<IActionList> ActionList;
	typedef shared_ptr<INotification> Notification;
	typedef shared_ptr<ISection> Section;
	typedef shared_ptr<IDeltaCoder> DeltaCoder;
	typedef shared_ptr<IAction> Action;
	typedef shared_ptr<IReader> Reader;
	typedef shared_ptr<IWriter> Writer;


	// Const smart pointers
	typedef shared_ptr<const IEntry> ConstEntry;
	typedef shared_ptr<const IDialogue> ConstDialogue;
	typedef shared_ptr<const IStyle> ConstStyle;
	typedef shared_ptr<const IModel> ConstModel;
	typedef shared_ptr<const ISection> ConstSection;


	// Lists
	typedef std::list<View> ViewList;
	typedef std::list<ActionList> ActionStack;


	// Model
	class IModel {
	public:
		virtual ~IModel() {}

		virtual Controller CreateController() = 0;
		virtual void AddListener(View listener) = 0;

		virtual void Save(Writer output,Format format=Format()) const = 0;

		virtual String GetUndoMessage(const String owner="") const = 0;
		virtual String GetRedoMessage(const String owner="") const = 0;
		virtual bool CanUndo(const String owner="") const = 0;
		virtual bool CanRedo(const String owner="") const = 0;

		virtual ConstSection GetSection(String name) const = 0;
		virtual ConstSection GetSectionByIndex(size_t index) const = 0;
		virtual size_t GetSectionCount() const = 0;

		virtual Format GetFormat() const = 0;
	};


	// Controller
	class IController {
	public:
		virtual ~IController() {}

		virtual ActionList CreateActionList(const String title,const String owner="",bool undoAble=true) = 0;
		virtual Selection CreateSelection() = 0;

		virtual void LoadFile(const String filename,const String encoding="") = 0;
		virtual void SaveFile(const String filename,const String encoding="UTF-8") = 0;

		virtual bool CanUndo(const String owner="") const = 0;
		virtual bool CanRedo(const String owner="") const = 0;
		virtual void Undo(const String owner="") = 0;
		virtual void Redo(const String owner="") = 0;

		virtual Dialogue CreateDialogue() const = 0;
		virtual Style CreateStyle() const = 0;

		virtual ConstDialogue GetDialogue(size_t n) const = 0;
		virtual ConstStyle GetStyle(size_t n) const = 0;
		virtual ConstStyle GetStyle(String name) const = 0;
		virtual ConstEntry GetEntry(size_t n,String section) const = 0;

		virtual const Format GetFormat() const = 0;
	};


	// View
	class IView {
	public:
		virtual ~IView() {}

		virtual void OnNotify(Notification notification) = 0;
	};


	// Selection
	class ISelection {
	public:
		virtual ~ISelection() {}

		virtual void AddLine(size_t line) = 0;
		virtual void RemoveLine(size_t line) = 0;

		virtual void AddRange(const Range &range) = 0;
		virtual void RemoveRange(const Range &range) = 0;

		virtual void AddSelection(const Selection &param) = 0;
		virtual void RemoveSelection(const Selection &param) = 0;

		virtual void NormalizeRanges() = 0;

		virtual size_t GetCount() const = 0;
		virtual size_t GetRanges() const = 0;
		virtual size_t GetLine(size_t n) const = 0;
		virtual size_t GetLineInRange(size_t n,size_t range) const = 0;
		virtual size_t GetLinesInRange(size_t range) const = 0;
		virtual bool IsContiguous() const = 0;
	};


	// Types
	enum SectionEntryType {
		SECTION_ENTRY_PLAIN,
		SECTION_ENTRY_DIALOGUE,
		SECTION_ENTRY_STYLE,
		SECTION_ENTRY_FILE,
		SECTION_ENTRY_RAW
	};


	// Entry
	class IEntry {
	public:
		virtual ~IEntry() {}

		virtual DeltaCoder GetDeltaCoder() const { return DeltaCoder(); }

		virtual bool IsIndexable() const { return false; }
		virtual String GetIndexName() const { return ""; }
		virtual String GetDefaultGroup() const = 0;

		virtual Entry Clone() const = 0;
	};


	// Dialogue
	class IDialogue : public IEntry {
	public:
		// Destructor
		virtual ~IDialogue() {}

		// Type
		SectionEntryType GetType() const { return SECTION_ENTRY_DIALOGUE; }

		// Capabilities
		virtual bool HasText() const { return false; }
		virtual bool HasImage() const { return false; }
		virtual bool HasTime() const { return false; }
		virtual bool HasFrame() const { return false; }
		virtual bool HasStyle() const { return false; }
		virtual bool HasActor() const { return false; }
		virtual bool HasMargins() const { return false; }

		// Read accessors
		virtual const String& GetText() const = 0;
		virtual const Time& GetStartTime() const = 0;
		virtual const Time& GetEndTime() const = 0;
		virtual int GetStartFrame() const = 0;
		virtual int GetEndFrame() const = 0;
		virtual bool IsComment() const = 0;
		virtual int GetLayer() const = 0;
		virtual int GetMargin(int n) const = 0;
		virtual const String& GetStyle() const = 0;
		virtual const String& GetActor() const = 0;
		virtual const String& GetUserField() const = 0;

		// Write accessors
		virtual void SetText(const String& text) = 0;
		virtual void SetStartTime(const Time& start) = 0;
		virtual void SetEndTime(const Time& end) = 0;
		virtual void SetStartFrame(int start) = 0;
		virtual void SetEndFrame(int end) = 0;
		virtual void SetComment(bool isComment) = 0;
		virtual void SetLayer(int layer) = 0;
		virtual void SetMargin(int margin,int value) = 0;
		virtual void SetStyle(const String& style) = 0;
		virtual void SetActor(const String& actor) = 0;
		virtual void SetUserField(const String& userField) = 0;
	};


	// Style
	class IStyle : public IEntry {
	public:
		virtual ~IStyle() {}

		// Type
		SectionEntryType GetType() const { return SECTION_ENTRY_STYLE; }

		// Read accessors
		virtual String GetName() const = 0;
		virtual String GetFontName() const = 0;
		virtual float GetFontSize() const = 0;
		virtual const Colour& GetColour(int n) const = 0;
		virtual int GetMargin(int n) const = 0;
	};


	// Format
	class IFormat {
	public:
		virtual ~IFormat() {}

		virtual String GetName() const = 0;
		virtual StringArray GetReadExtensions() const = 0;
		virtual StringArray GetWriteExtensions() const = 0;
		virtual FormatHandler GetHandler() const = 0;

		virtual bool CanStoreText() const = 0;
		virtual bool CanStoreImages() const = 0;
		virtual bool CanUseTime() const = 0;
		virtual bool CanUseFrames() const = 0;

		virtual bool HasStyles() const = 0;
		virtual bool HasMargins() const = 0;
		virtual bool HasActors() const = 0;
		virtual bool HasUserField() const = 0;
		virtual String GetUserFieldName() const = 0;

		virtual int GetTimingPrecision() const = 0;	// In milliseconds
		virtual int GetMaxTime() const = 0;	// In milliseconds

		virtual Dialogue CreateDialogue() const = 0;
		virtual Style CreateStyle() const = 0;

		virtual bool IsBinary() const = 0;
		virtual float CanReadFile(Reader &reader) const = 0;	// Should return a float from 0.0f to 1.0f indicating how certain it is that it can read it
	};


	// Format handler
	class IFormatHandler {
	public:
		virtual ~IFormatHandler() {}

		virtual void Load(IModel &model,Reader file) = 0;
		virtual void Save(const IModel &model,Writer file) const = 0;
	};


	// Action interface
	class IAction {
	public:
		virtual ~IAction() {}
		virtual Action GetAntiAction() const = 0;
		virtual void Execute() = 0;
	};


	// Action list
	class IActionList {
	public:
		virtual ~IActionList() {}

		virtual String GetName() const = 0;
		virtual String GetOwner() const = 0;
		virtual Model GetModel() const = 0;
		virtual bool CanUndo() const = 0;

		virtual void AddAction(Action action) = 0;
		virtual void Finish() = 0;

		virtual void InsertLine(Entry line,int position=-1,const String section="") = 0;
		virtual void RemoveLine(int position,const String section) = 0;
		virtual Entry ModifyLine(int position,const String section) = 0;
		virtual std::vector<Entry> ModifyLines(Selection selection,const String section) = 0;
	};


	// Section
	class ISection {
	public:
		virtual ~ISection() {}

		// Section name
		virtual void SetName(const String& newName) = 0;
		virtual const String& GetName() const = 0;

		// Script properties
		virtual void SetProperty(const String &key,const String &value) = 0;
		virtual void UnsetProperty(const String &key) = 0;
		virtual String GetProperty(const String &key) const = 0;
		virtual bool HasProperty(const String &key) const = 0;
		virtual size_t GetPropertyCount() const = 0;
		virtual String GetPropertyName(size_t index) const = 0;

		// Indexed
		virtual Entry GetFromIndex(String key) const = 0;

		// Entries
		virtual void AddEntry(Entry entry,int pos=-1) = 0;
		virtual void RemoveEntryByIndex(size_t index) = 0;
		virtual void RemoveEntry(Entry entry) = 0;
		virtual Entry GetEntry(size_t index) const = 0;
		virtual Entry& GetEntryRef(size_t index) = 0;
		virtual size_t GetEntryCount() const = 0;
	};


	// Delta coder
	class IDeltaCoder {
	public:
		virtual ~IDeltaCoder() {}
		virtual VoidPtr EncodeDelta(VoidPtr from,VoidPtr to,bool withTextFields=true) const = 0;
		virtual VoidPtr EncodeReverseDelta(VoidPtr delta,VoidPtr object) const = 0;
		virtual void ApplyDelta(VoidPtr delta,VoidPtr object) const = 0;
	};


	// File reader
	class IReader {
	public:
		virtual ~IReader() {}

		virtual String GetFileName() = 0;
		virtual void Rewind() = 0;

		virtual bool HasMoreLines() = 0;
		virtual String ReadLineFromFile() = 0;
		virtual String GetCurrentEncoding() = 0;
	};


	// File writer
	class IWriter {
	public:
		virtual ~IWriter() {}
		virtual void WriteLineToFile(String line,bool addLineBreak=true) = 0;
		virtual void Flush() = 0;
	};


	// Library
	class ILibAthenaSub {
	public:
		virtual ~ILibAthenaSub() {}
		virtual Model CreateModel()=0;
	};
	typedef shared_ptr<ILibAthenaSub> LibAthenaSub;
}
