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
#include <wx/string.h>
#include <vector>
#include <list>

namespace Athenasub {

	// Strings
	typedef wxString String;
	typedef std::vector<String> StringArray;


	// Prototypes
	class Range;
	class ISelection;
	class IController;
	class IView;
	class IFormatHandler;
	class ITime;
	class IColour;
	class IEntry;
	class IDialogue;
	class IStyle;
	class IFormat;
	class IModel;
	class IActionList;
	class INotification;
	class ISection;
	class IDeltaCoder;
	class CController;


	// Smart pointers
	typedef shared_ptr<void> VoidPtr;
	typedef shared_ptr<ISelection> Selection;
	typedef shared_ptr<IController> Controller;
	typedef shared_ptr<IView> View;
	typedef shared_ptr<IFormatHandler> FormatHandler;
	typedef shared_ptr<ITime> Time;
	typedef shared_ptr<IColour> Colour;
	typedef shared_ptr<IEntry> Entry;
	typedef shared_ptr<IDialogue> Dialogue;
	typedef shared_ptr<IStyle> Style;
	typedef shared_ptr<IFormat> Format;
	typedef shared_ptr<IModel> Model;
	typedef shared_ptr<IActionList> ActionList;
	typedef shared_ptr<INotification> Notification;
	typedef shared_ptr<ISection> Section;
	typedef shared_ptr<IDeltaCoder> DeltaCoder;


	// Const smart pointers
	typedef shared_ptr<const IEntry> ConstEntry;
	typedef shared_ptr<const IDialogue> ConstDialogue;
	typedef shared_ptr<const IStyle> ConstStyle;
	typedef shared_ptr<const IModel> ConstModel;


	// Lists
	typedef std::list<View> ViewList;
	typedef std::list<ActionList> ActionStack;


	// Model
	class IModel {
		friend class CFormatHandler;
		friend class CActionList;
		friend class CController;
		friend class IAction;

	protected:
		virtual void ProcessActionList(ActionList actionList,int type=0) = 0;

		virtual String GetUndoMessage(const String owner=L"") const = 0;
		virtual String GetRedoMessage(const String owner=L"") const = 0;
		virtual bool CanUndo(const String owner=L"") const = 0;
		virtual bool CanRedo(const String owner=L"") const = 0;
		virtual void Undo(const String owner=L"") = 0;
		virtual void Redo(const String owner=L"") = 0;
		virtual void ActivateStack(ActionStack stack,bool isUndo,const String &owner) = 0;

		virtual void DispatchNotifications(Notification notification) const = 0;

		virtual void Clear() = 0;
		virtual void Load(wxInputStream &input,Format format=Format(),const String encoding=L"") = 0;
		virtual void Save(wxOutputStream &output,Format format=Format(),const String encoding=L"UTF-8") = 0;

		virtual void AddSection(String name) = 0;
		virtual Section GetSection(String name) const = 0;
		virtual Section GetSectionByIndex(size_t index) const = 0;
		virtual size_t GetSectionCount() const = 0;

	public:
		virtual ~IModel();

		virtual Controller CreateController()=0;
		virtual Format GetFormat() const=0;
		virtual void AddListener(View listener)=0;
	};


	// View
	class IView {
	public:
		virtual ~IView() {}
	};


	// Controller
	class IController {
	public:
		virtual ~IController() {}

		virtual ActionList CreateActionList(const String title,const String owner=L"",bool undoAble=true) = 0;
		virtual Selection CreateSelection() = 0;

		virtual void LoadFile(const String filename,const String encoding=L"") = 0;
		virtual void SaveFile(const String filename,const String encoding=L"UTF-8") = 0;

		virtual bool CanUndo(const String owner=L"") const = 0;
		virtual bool CanRedo(const String owner=L"") const = 0;
		virtual void Undo(const String owner=L"") = 0;
		virtual void Redo(const String owner=L"") = 0;

		virtual Dialogue CreateDialogue() const = 0;
		virtual Style CreateStyle() const = 0;

		virtual ConstDialogue GetDialogue(size_t n) const = 0;
		virtual ConstStyle GetStyle(size_t n) const = 0;
		virtual ConstStyle GetStyle(String name) const = 0;
		virtual ConstEntry GetEntry(size_t n,String section) const = 0;

		virtual const Format GetFormat() const = 0;
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


	// Time
	class ITime {
	public:
		virtual ~ITime() {}

		virtual void SetMS(int milliseconds) = 0;
		virtual int GetMS() const = 0;

		virtual String GetString(int ms_precision,int h_precision) const = 0;
		virtual void ParseString(const String &data) = 0;

		virtual Time Clone() const = 0;
	};
	

	// Color
	class IColour {
	public:
		virtual ~IColour() {}

		virtual void SetRed(int red) = 0;
		virtual void SetGreen(int green) = 0;
		virtual void SetBlue(int blue) = 0;
		virtual void SetAlpha(int alpha) = 0;

		virtual int GetRed() const = 0;
		virtual int GetGreen() const = 0;
		virtual int GetBlue() const = 0;
		virtual int GetAlpha() const = 0;

		virtual void Parse(String str,bool reverse) = 0;
		virtual String GetVBHex(bool withAlpha=false,bool withHeader=true,bool withFooter=true) const = 0;
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
		virtual String GetIndexName() const { return L""; }
		virtual String GetDefaultGroup() const = 0;

		virtual Entry Clone() const = 0;

		/*
		static PlainTextPtr GetAsPlain(EntryPtr ptr);
		static DialoguePtr GetAsDialogue(EntryPtr ptr);
		static DialogueConstPtr GetAsDialogue(EntryConstPtr ptr);
		static StylePtr GetAsStyle(EntryPtr ptr);
		static AttachmentPtr GetAsFile(EntryPtr ptr);
		static RawEntryPtr GetAsRaw(EntryPtr ptr);
		*/
	};


	// Dialogue
	class IDialogue : public IEntry {
	public:
		// Destructor
		virtual ~IDialogue() {}

		// Type
		SectionEntryType GetType() const { return SECTION_ENTRY_DIALOGUE; }
		Dialogue GetAsDialogue() { return Dialogue(this); }

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
		virtual const ITime& GetStartTime() const = 0;
		virtual const ITime& GetEndTime() const = 0;
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
		virtual void SetStartTime(Time start) = 0;
		virtual void SetEndTime(Time end) = 0;
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
		Style GetAsStyle() { return Style(this); }

		// Read accessors
		virtual String GetName() const = 0;
		virtual String GetFontName() const = 0;
		virtual float GetFontSize() const = 0;
		virtual const IColour& GetColour(int n) const = 0;
		virtual int GetMargin(int n) const = 0;
	};


	// Format
	class IFormat {
	public:
		virtual ~IFormat() {}

		virtual String GetName() const = 0;
		virtual StringArray GetReadExtensions() const = 0;
		virtual StringArray GetWriteExtensions() const = 0;
		//virtual FormatHandler GetHandler(Model &model) const = 0;

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
	};


	// Format handler
	class IFormatHandler {
	public:
		virtual ~IFormatHandler() {}
	};


	// Action interface
	class IAction;
	typedef shared_ptr<IAction> Action;
	class IAction {
	public:
		virtual ~IAction() {}
		virtual Action GetAntiAction(ConstModel model) const = 0;
		virtual void Execute(Model model) = 0;

		Section GetSection(Model model,const String &name) const { return model->GetSection(name); }
	};


	// Action list
	class IActionList {
	public:
		virtual ~IActionList() {}

		virtual String GetName() const = 0;
		virtual String GetOwner() const = 0;

		virtual void AddAction(const Action action) = 0;
		virtual void Finish() = 0;

		virtual void InsertLine(Entry line,int position=-1,const String section=L"") = 0;
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
		virtual Entry& GetEntryRef(size_t index) const = 0;
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


	// Library
	class ILibAthenaSub {
	public:
		virtual ~ILibAthenaSub() {}

		virtual Model CreateModel()=0;
	};
	typedef shared_ptr<ILibAthenaSub> LibAthenaSub;


	// Operators
	Time operator+(const ITime& p1,int p2) { Time res = p1.Clone(); res->SetMS(res->GetMS()+p2); return res; }
	Time operator-(const ITime& p1,int p2) { Time res = p1.Clone(); res->SetMS(res->GetMS()-p2); return res; }
	bool operator==(const ITime& p1,const ITime& p2) { return p1.GetMS() == p2.GetMS(); }

}
