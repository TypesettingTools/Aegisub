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

#include <chrono>
#include <functional>

#include <wx/string.h>
#include <wx/arrstr.h>



namespace wakatime {


void init();

void clear();

void update(bool isWrite, agi::fs::path const* filename = nullptr);


using namespace std::chrono;

    typedef struct  {
        const wxString program;
        const wxString plugin_name;
        const wxString type;
        const wxString version;
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
        cli (){
                if(!handle_cli()){
                    assert(false && "ERROR");
                }

                last_heartbeat = 0s;

                //TODO get from internal settings!
                wxString* aegisub_key =  nullptr; //"8c982ef8-3baa-44d6-865b-203949200c5e"; //agi::config::getKey(wakatime_key_setting);
                if(aegisub_key == nullptr){
                    wxArrayString *buffer = new wxArrayString();
                    buffer->Add(wxString("--config-read api_key"));
                    invoke_cli_async(buffer,[this](CLIResponse response)-> void{
                
                        if (response.ok){
                            this->key = response.output_string;
                            if(this->key->Last() == '\n'){
                                this->key = new wxString(this->key->ToAscii().data(),this->key->Length()-1 );
                            }

                            std::cout << response;
                            //agi::config::setKey(wakatime_key_setting,key);
                        }
                    });
                }else{
                    this->key = aegisub_key;
                    // write to wakatime config!!!
                }

            }


        /// Map to hold Combo instances
        bool initialize();
        void change_project(wxString* new_file, wxString* project_name);
        void change_api_key(wxString* key);
        void send_heartbeat(bool isWrite);
        void get_time_today();
        ProjectInfo project_info = {
            project_name : nullptr,
            file_name: nullptr,
            changed: false
        };

    private:
        wxString* key;
        seconds last_heartbeat;
        Plugin plugin_info = {
            program: "Aegisub",
            plugin_name: "aegisub-wakatime",
            type: "ASS",//Advanced SubStation Alpha",
            version: "0.0.1"
        };
        wxString* cli_path;
        bool handle_cli();
        bool is_cli_present();
        bool download_cli();

        //@deprecated
        CLIResponse invoke_cli_sync(wxArrayString* options);

        void invoke_cli_async(wxArrayString* options, std::function<void ( CLIResponse response)> callback);
    };
}

#endif