// Copyright (c) 2009, Amar Takhar <verm@aegisub.org>
//
// Permission to use, copy, modify, and distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//
// $Id$

/// @file report.cpp
/// @brief Generation and manipulation of reports.
/// @ingroup base

#ifndef R_PRECOMP
#include <wx/intl.h>
#include <wx/log.h>
#endif

#include <libaegisub/cajun/elements.h>
#include <libaegisub/cajun/writer.h>


#include "report.h"
#include "include/platform.h"
#include "aegisub.h"

/// @brief Contstructor
Report::Report() {
	ReportCreate();
}

/// @brief Create report layout and add contents.
/// @return Document.
Report::XMLReport Report::ReportCreate() {


	json::Object root;

	Platform *p = Platform::GetPlatform();

	json::Object general;
	general["Signature"] = json::String(p->Signature());
	general["Date"] = json::String(p->Date());
	general["Architecture"] = json::String(p->ArchName());
	general["OS Family"] = json::String(p->OSFamily());
	general["OS Name"] = json::String(p->OSName());
	general["Endian"] = json::String(p->Endian());
	general["OS Version"] = json::String(p->OSVersion());
	general["wx Version"] = json::String(p->wxVersion());
	general["Locale"] = json::String(p->Locale());
	general["Language"] = json::String(p->Language());
	general["System Language"] = json::String(p->SystemLanguage());



	json::Object aegisub;

	/// I'll fix these at the end.
	/*
	Last Version
	Spelling Language
	Thesaurus Language
	Audio Player
	Audio Provider
	Video Provider
	Subtitles Provider
	Save Charset
	Grid Font Size
	Edit Font Size
	Spectrum Enabled
	Spectrum Quality
	Call Tips Enabled
	Medusa Hotkeys Enabled
	*/


	json::Object hardware;
	hardware["Memory Size"] = json::Number();


	json::Object cpu;
	cpu["Id"] = json::String();
	cpu["Speed"] = json::String();
	cpu["Count"] = json::String();
	cpu["Cores"] = json::String();
	cpu["Features"] = json::String();
	cpu["Features2"] = json::String();



/*
	wxXmlNode *cpu = new wxXmlNode(wxXML_ELEMENT_NODE, CPU);
	doc.hardware->AddChild(cpu);
	Add(cpu, Id, p->CPUId());
	Add(cpu, Speed, p->CPUSpeed());
	Add(cpu, Count, p->CPUCount());
	Add(cpu, Cores, p->CPUCores());
	Add(cpu, Features, p->CPUFeatures());
	Add(cpu, Features2, p->CPUFeatures2());

	wxXmlNode *display = new wxXmlNode(wxXML_ELEMENT_NODE, Display);
	doc.hardware->AddChild(display);
	Add(display, Depth, p->DisplayDepth());
	Add(display, Colour Screen, p->DisplayColour());
	Add(display, Size, p->DisplaySize());
	Add(display, Pixels Per Inch, p->DisplayPPI());

	wxXmlNode *display_gl = new wxXmlNode(wxXML_ELEMENT_NODE, OpenGL);
	display->AddChild(display_gl);

	Add(display_gl, Vendor, p->OpenGLVendor());
	Add(display_gl, Renderer, p->OpenGLRenderer());
	Add(display_gl, Version, p->OpenGLVersion());
	Add(display_gl, Extensions, p->OpenGLExt());

#ifdef __WINDOWS__
	doc.windows = new wxXmlNode(wxXML_ELEMENT_NODE, Windows);
	doc.report->AddChild(doc.windows);
	Add(doc.windows, Service Pack, p->ServicePack());
	Add(doc.windows, Graphics Driver Version, p->DriverGraphicsVersion());
	Add(doc.windows, DirectShow Filters, p->DirectShowFilters());
	Add(doc.windows, AntiVirus Installed, p->AntiVirus());
	Add(doc.windows, Firewall Installed, p->Firewall());
	Add(doc.windows, DLL, p->DLLVersions());
#endif

#ifdef __UNIX__
	doc.unixx = new wxXmlNode(wxXML_ELEMENT_NODE, Unix);
	doc.report->AddChild(doc.unixx);
	Add(doc.unixx, Desktop Environment, p->DesktopEnvironment());
	Add(doc.unixx, Libraries, p->UnixLibraries());
#endif

#ifdef __APPLE__
	doc.osx = new wxXmlNode(wxXML_ELEMENT_NODE, OS X);
	doc.report->AddChild(doc.osx);
	Add(doc.osx, Patch, p->PatchLevel());
	Add(doc.osx, QuickTime Extensions, p->QuickTimeExt());
	Add(doc.osx, Model, p->HardwareModel());

#endif
*/
	return doc;
}

/// @brief Add a new XML node.
/// @param parent Parent nodee
/// @param node Name of the new node
/// @param text Contents
void Report::Add(wxXmlNode *parent, wxString node, wxString text) {
	// Using AddChild() keeps the nodes in their natural order. It's slower but our
	// document is pretty small.  Doing it the faster way results in reverse-ordered nodes.
	wxXmlNode *tmp = new wxXmlNode(wxXML_ELEMENT_NODE, node);
	tmp->AddChild(new wxXmlNode(wxXML_TEXT_NODE, node, text));
	parent->AddChild(tmp);
}

/// @brief Recursive function to populate listView and text-based for Clipboard.
/// @param node Node to parse.
/// @param listView wxListView to populate.
void Report::ProcessNode(wxXmlNode *node, wxString *text, wxListView *listView) {
	wxString node_name;
	nameMap::iterator names;
	nameMap nMap = HumanNames();

	wxXmlNode *child = node->GetChildren();

	while (child) {
		wxString name = child->GetName();

		if ((names = nMap.find(name)) != nMap.end()) {
			node_name = locale->GetString(names->second);
		} else {
			wxLogDebug("Report::ProcessNode Unknown node found: \"%s\" (add it to nMap)\n", name);
			node_name = name;
		}

		wxListItem column;
		int row = listView->GetItemCount();
		int depth = child->GetDepth();

		if (child->GetChildren()->GetType() == wxXML_ELEMENT_NODE) {
			int font_size = 15 - floor(depth * 2 + 0.5);
			int bgcolour = 155 + (depth * 20);
			listView->InsertItem(row,node_name);
			listView->SetItemFont(row, wxFont(font_size, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
			listView->SetItemBackgroundColour(row, wxColour(bgcolour,bgcolour,bgcolour, wxALPHA_OPAQUE));
			if (depth == 1) text->Append("\n");
			text->Append(wxString::Format("%s\n", node_name.Pad((depth*2)-2, ' ', 0)));
			ProcessNode(child, text, listView);
		} else {
			wxString content = child->GetNodeContent();
			listView->InsertItem(row,node_name);
			listView->SetItem(row,1, content);
			text->Append(wxString::Format("%-22s: %s\n", node_name.Pad((depth*2)-2, ' ', 0), content));
		}

		child = child->GetNext();
	}
}


void Report::Fill(wxString *text, wxListView *listView) {

	listView->InsertColumn(0, _("Entry"), wxLIST_FORMAT_RIGHT);
	listView->InsertColumn(1, _("Text"), wxLIST_FORMAT_LEFT, 100);

	ProcessNode(doc.report, text, listView);

	listView->SetColumnWidth(0, wxLIST_AUTOSIZE);
	listView->SetColumnWidth(1, wxLIST_AUTOSIZE);

}

/// @brief Return Report as Text for the Clipboard.
void Report::Save(wxString file) {
	doc.doc->Save(file);
}
