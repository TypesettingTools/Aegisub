// Copyright (c) 2022, Totto
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

/* #include "libaegisub/fs.h"
#include "libaegisub/io.h"
#include "libaegisub/json.h" */

#pragma once
#ifndef _WAKATIME_H_
#define _WAKATIME_H_

#include "libaegisub/path.h"
#include "options.h"
#include "version.h"
#include "frame_main.h"

#include <chrono>
#include <functional>

#include <wx/string.h>
#include <wx/arrstr.h>



namespace wakatime {


void init();

void clear();

void update(bool isWrite, agi::fs::path const* filename = nullptr);

void setUpdateFunction(std::function<void ()> updateFunction);

wxString getTime();


using namespace std::chrono;

    typedef struct  {
        const wxString program;
        const wxString plugin_name;
        const wxString short_type;
        const wxString long_type;
        const wxString plugin_version;
        wxString* aegisub_version;
    } Plugin;

    typedef struct  {
        bool ok;
        wxString* error_string; // is nullptr if no error occured
        wxString* output_string; // can be empty and is nullptr if an error occured
    } CLIResponse;


        std::ostream& operator<<(std::ostream& os,  const wakatime::CLIResponse& arg);

        std::ostream& operator<<(std::ostream& os, const wakatime::CLIResponse * arg);

    typedef struct {
        wxString* project_name;
        wxString* file_name;
        bool changed;
    } ProjectInfo;


/// @class cli
/// the wakatime cli class, it has the needed methods
    class cli {
    public:
        cli();

        bool initialize();
        void change_project(wxString* new_file, wxString* project_name);
        void change_api_key(wxString* key);
        void send_heartbeat(bool isWrite);
        ProjectInfo project_info = {
            project_name : nullptr,
            file_name: nullptr,
            changed: false
        };

        wxString* currentTime;
        std::function<void ()> updateFunction;

    private:
        wxString* key;
        seconds last_heartbeat;
        bool debug;
        Plugin plugin_info = {
            program: "Aegisub",
            plugin_name: "aegisub-wakatime",
            short_type: "ASS",
            long_type:"Advanced SubStation Alpha",
            plugin_version: "1.1.3"
        };
        wxString* cli_path;
        bool cliInstalled;

        void get_time_today();
        bool handle_cli();
        bool is_cli_present();
        bool download_cli();
        bool is_key_valid(wxString key);
        void getKey();
        void getDebug();
        void setTime(wxString* text);

        void invoke_cli_async(wxArrayString* options, std::function<void ( CLIResponse response)> callback);
    };
}

#endif
