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
#include "libaegisub/log.h"
#include "wakatime.h"

#include <chrono>
#include <wx/string.h>
#include <wx/arrstr.h>
#include <wx/utils.h>
#include <wx/process.h>
namespace wakatime {


    wxString* StringArrayToString(wxArrayString* input, wxString* seperator){
            wxString* output = new wxString();
            for(size_t i=0; i < input->GetCount(); ++i){
                output->Append(i == 0 ? input->Item(i) :(wxString::Format("%s%s",*seperator,input->Item(i))));
            }
        return output;
    }

    wxString* StringArrayToString(wxArrayString* input, const char * seperator = " "){
        return StringArrayToString(input, new wxString(seperator));
    }


using namespace std::chrono;


        void cli::change_project(wxString* project_name, wxString* new_file){
			this->project_name = project_name;
			send_heartbeat(new_file,true);
		}

        void cli::change_api_key(wxString* key){
			this->key = key;
			// TODO check validity of key!
		}

        bool cli::send_heartbeat(wxString* file, bool isWrite){

		seconds now =  duration_cast<seconds>(steady_clock::now().time_since_epoch());

        // TODO get that data!
        bool lastFileChanged = false;
		if (!(last_heartbeat + (2s* 60) < now || isWrite || lastFileChanged)){
            LOG_D("wakatime/send_heartbet") << "No heartbeat to send";
			return false;
		}

		last_heartbeat = now;

/* 
      "--entity string"                    Absolute path to file for the heartbeat. Can also be a url, domain or app when --entity-type is not file.

      "--key string                       Your wakatime api key; uses api_key from ~/.wakatime.cfg by default.
    language string                  Optional language name. If valid, takes priority over auto-detected language.


    [     Send heartbeat function
        check lastHeartbeat variable. if isWrite is false, and file has not changed since last heartbeat, and less than 2 minutes since last heartbeat, then return and do nothing
        run wakatime-cli in background process passing it the current file
        update lastHeartbeat variable with current file and current time
 ]]
        "--project string"                   Override auto-detected project. Use --alternate-project to supply a fallback project if one can't b
 */
        wxArrayString *buffer = new wxArrayString();

        buffer->Add(wxString::Format("--language '%s'", plugin_info.type));
        buffer->Add(wxString::Format("--entity '%s'", *file));
        buffer->Add(wxString::Format("--project '%s'", *project_name));
        if (isWrite ){
            buffer->Add("--write");
        }

        CLIResponse response_string = invoke_cli(buffer);


        wxArrayString *buffer2 = new wxArrayString();
        buffer2->Add("--today");
        CLIResponse time_today = invoke_cli(buffer2);
        if(time_today.send_successful){
            LOG_D("time/today") << time_today.output_string->ToAscii();
        }
        
        return response_string.send_successful;

	}

        bool cli::handle_cli(){
			cli_path = new wxString("/home/totto/.wakatime/wakatime-cli");
			if(!is_cli_present()){
				return download_cli();
			}

			return true;

/* 

    -- TODO:   Check for wakatime-cli, or download into ~/.wakatime/ if missing or needs an update
    --TODO:    Check for api key in ~/.wakatime.cfg, prompt user to enter if does not exist
    -- TODO: Process event listeners to detect when current file changes, a file is modified, and a file is saved
    --[[   Current file changed (our file change event listener code is run)
        go to Send heartbeat function with isWrite false
    User types in a file (our file modified event listener code is run)
        go to Send heartbeat function with isWrite false
    A file is saved (our file save event listener code is run)
        go to Send heartbeat function with isWrite true ]]
            assert(false && "Not implemented yet"); */
        }

       	bool cli::is_cli_present(){
			// TODO check if cli_path exists!
			return true;
            assert(false && "Not implemented yet");
        }

        bool cli::download_cli(){
            assert(false && "Not implemented yet");
			return false;
        }

        CLIResponse cli::invoke_cli(wxArrayString* options){
		//	assert(false && "Not implemented yet");

        CLIResponse response = {
            send_successful:true,
            error_string : nullptr,
            output_string: nullptr
        };

        options->Add("--verbose");

        //TODO also version should be dynamic!
        options->Add(wxString::Format("--plugin 'aegisub/%s %s/8975-master-8d77da3'",plugin_info.version, plugin_info.plugin_name));

        wxString command = wxString::Format("%s %s", *cli_path, * StringArrayToString(options));
        LOG_D("wakatime/execute") << command.ToAscii();
/*         wxProcess* process = new wxProcess();
        process.

        long pid = wxExecute(*command, wxEXEC_ASYNC,process);
        if(pid == 0){
            LOG_E("wakatime/execute") << "Command couldn't be executed: " << command->ToAscii();
            return nullptr;
        } */



        wxArrayString *output = new wxArrayString();
        wxArrayString *errors = new wxArrayString();

        long returnCode = wxExecute(command, *output, *errors, wxEXEC_SYNC);
        if(returnCode != 0 || !errors->IsEmpty()){
            LOG_E("wakatime/execute") << "Command couldn't be executed: " << command.ToAscii();
            wxString* error_string = StringArrayToString(errors,"\n");
            LOG_E("wakatime/execute") << "The Errors were: " << error_string->ToAscii();
            
            response.error_string = error_string;
            response.send_successful = false;
            return response;
        } 

        wxString* output_string = StringArrayToString(output,"\n");
        LOG_D("wakatime/output") << " " << output_string->ToAscii();

        response.output_string = output_string;


		return response;
    };




	wakatime::cli *wakatime_cli = nullptr;
	void init() {
        LOG_D("wakatime/init");
		wakatime_cli = new cli(new wxString("project name"));
	}

	void clear() {
		delete wakatime_cli;
        LOG_D("wakatime/clear");
	}


	void update(bool isWrite){
	//TODO: 	assert(false && "Not implemented yet");
	    bool send_successfully = wakatime_cli->send_heartbeat(new wxString("file"),isWrite);
		LOG_D("wakatime/update") << "isWrite: " << isWrite << " send_successfully: " << send_successfully;
	}

} // namespace wakatime



