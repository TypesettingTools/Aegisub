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
#include <chrono>

#include <wx/string.h>
#include <wx/arrstr.h>

namespace wakatime {


void init();

void clear();

void update(bool isWrite);


using namespace std::chrono;

    typedef struct  {
        const wxString program;
        const wxString plugin_name;
        const wxString type;
        const wxString version;
    } Plugin;

    typedef struct  {
        bool send_successful;
        wxString* error_string; // is nullptr if no error occured
        wxString* output_string; // can be empty and is nullptr if an error occured
    } CLIResponse;


/// @class cli
/// the wakatime cli class, it has the needed methods
    class cli {
    public:
    cli (wxString* project_name){
                if(!handle_cli()){
                    assert(false && "ERROR");
                }
                this->project_name = project_name;

                last_heartbeat = 0s;

                //TODO get from internal settings!
                wxString* aegisub_key =  nullptr; //"8c982ef8-3baa-44d6-865b-203949200c5e";//agi::config::getKey(wakatime_key_setting);
                if(aegisub_key == nullptr){
                    wxArrayString *buffer = new wxArrayString();
                    buffer->Add(wxString("--config-read api_key"));
                    CLIResponse stored_api_key = invoke_cli(buffer);
                    if (stored_api_key.send_successful){
                        key = stored_api_key.output_string;
                        //agi::config::setKey(wakatime_key_setting,key);
                    }
                }else{
                    key = aegisub_key;
                }

                send_heartbeat(new wxString("file"),false);

            }


        /// Map to hold Combo instances
        bool initialize(wxString* project_name);
        void change_project(wxString* project_name, wxString* new_file);
        void change_api_key(wxString* key);
        bool send_heartbeat(wxString* file, bool isWrite);

    private:
        wxString* project_name;
        wxString* key;
        seconds last_heartbeat;
        Plugin plugin_info = {
            program: "Aegisub",
            plugin_name: "aegisub-wakatime",
            type: "Advanced SubStation Alpha",
            version: "0.0.1"
        };
        wxString* cli_path;
        bool handle_cli();
        bool is_cli_present();
        bool download_cli();
        CLIResponse invoke_cli(wxArrayString* options);

    };
}