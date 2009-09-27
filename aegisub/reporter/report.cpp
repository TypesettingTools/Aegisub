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

#include "report.h"
#include "platform.h"
#include "aegisub.h"

/// @brief Contstructor
Report::Report() {
	ReportCreate();
}

/// @brief Create report layout and add contents.
/// @return Document.
Report::XMLReport Report::ReportCreate() {

	// So we can use GetString() below.
	locale = new wxLocale();

	doc.doc = new wxXmlDocument();

	doc.report = new wxXmlNode(wxXML_ELEMENT_NODE, "report");
	doc.doc->SetRoot(doc.report);
    Platform *p = Platform::GetPlatform();

	doc.general = new wxXmlNode(doc.report, wxXML_ELEMENT_NODE, "general");
	Add(doc.general, "signature", p->Signature());
	Add(doc.general, "date", p->Date());
	Add(doc.general, "arch", p->ArchName());
	Add(doc.general, "osfamily", p->OSFamily());
	Add(doc.general, "osname", p->OSName());
	Add(doc.general, "osendian", p->Endian());
	Add(doc.general, "osversion", p->OSVersion());
	Add(doc.general, "wxversion", p->wxVersion());
	Add(doc.general, "locale", p->Locale());
	Add(doc.general, "lang", p->Language());
	Add(doc.general, "syslang", p->SystemLanguage());

	doc.aegisub = new wxXmlNode(wxXML_ELEMENT_NODE, "aegisub");
	doc.report->AddChild(doc.aegisub);

	Aegisub *config = new Aegisub::Aegisub();
	Add(doc.aegisub, "lastversion", config->Read("Config/last version"));
	Add(doc.aegisub, "spelllang", config->Read("Config/spell checker language"));
	Add(doc.aegisub, "thesauruslang", config->Read("Config/thesaurus language"));
	Add(doc.aegisub, "audioplayer", config->Read("Config/audio player"));
	Add(doc.aegisub, "audioprovider", config->Read("Config/audio provider"));
	Add(doc.aegisub, "videoprovider", config->Read("Config/video provider"));
	Add(doc.aegisub, "subtitleprovider", config->Read("Config/subtitles provider"));
	Add(doc.aegisub, "savecharset", config->Read("Config/save charset"));
	Add(doc.aegisub, "gridfontsize", config->Read("Config/grid font size"));
	Add(doc.aegisub, "editfontsize", config->Read("Config/edit font size"));
	Add(doc.aegisub, "spectrum", config->Read("Config/audio spectrum"));
	Add(doc.aegisub, "spectrumqual", config->Read("Config/audio spectrum quality"));
	Add(doc.aegisub, "calltips", config->Read("Config/call tips enabled"));
	Add(doc.aegisub, "medusakeys", config->Read("Config/audio medusa timing hotkeys"));

	doc.hardware = new wxXmlNode(wxXML_ELEMENT_NODE, "hardware");
	doc.report->AddChild(doc.hardware);
	Add(doc.hardware, "memory", p->Memory());

		wxXmlNode *cpu = new wxXmlNode(wxXML_ELEMENT_NODE, "cpu");
		doc.hardware->AddChild(cpu);
		Add(cpu, "id", p->CPUId());
		Add(cpu, "speed", p->CPUSpeed());
		Add(cpu, "count", p->CPUCount());
		Add(cpu, "cores", p->CPUCores());
		Add(cpu, "features", p->CPUFeatures());
		Add(cpu, "features2", p->CPUFeatures2());

		wxXmlNode *display = new wxXmlNode(wxXML_ELEMENT_NODE, "display");
		doc.hardware->AddChild(display);
		Add(display, "vendor", p->VideoVendor());
		Add(display, "renderer", p->VideoRenderer());
		Add(display, "version", p->VideoVersion());
		Add(display, "extensions", p->VideoExt());
		Add(display, "depth", p->DisplayDepth());
		Add(display, "colour", p->DisplayColour());
		Add(display, "size", p->DisplaySize());
		Add(display, "ppi", p->DisplayPPI());

#ifdef __WINDOWS__
	doc.windows = new wxXmlNode(wxXML_ELEMENT_NODE, "windows");
	doc.report->AddChild(doc.windows);
	Add(doc.windows, "sp", p->ServicePack());
	Add(doc.windows, "graphicsver", p->DriverGraphicsVersion());
	Add(doc.windows, "dshowfilter", p->DirectShowFilters());
	Add(doc.windows, "antivirus", p->());
	Add(doc.windows, "firewall", p->());
	Add(doc.windows, "dll", p->DLLVersions());
#endif

#ifdef __UNIX__
	doc.unixx = new wxXmlNode(wxXML_ELEMENT_NODE, "unix");
	doc.report->AddChild(doc.unixx);
	Add(doc.unixx, "desktopenv", p->DesktopEnvironment());
	Add(doc.unixx, "lib", p->UnixLibraries());
#endif

#ifdef __APPLE__
	doc.osx = new wxXmlNode(wxXML_ELEMENT_NODE, "osx");
	doc.report->AddChild(doc.osx);
	Add(doc.osx, "patch", p->PatchLevel());
	Add(doc.osx, "quicktimeext", p->QuickTimeExt());
	Add(doc.osx, "model", p->HardwareModel());

#endif

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
			int font_size = 15 - (round(depth * 2.5));
			int bgcolour = 185 + (depth * 25);
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
