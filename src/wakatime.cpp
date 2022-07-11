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

#include "options.h"
#include "wakatime.h"

#include "libaegisub/log.h"
#include "libaegisub/path.h"

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


        void cli::change_project(wxString* new_file, wxString* project_name){
            project_info.file_name = new_file;
            project_info.project_name = project_name;
			project_info.changed = true;

			send_heartbeat(false);
		}

        void cli::change_api_key(wxString* key){
			this->key = key;
			// TODO check validity of key!
		}

        bool cli::send_heartbeat(bool isWrite){

/* 

    --TODO:    Check for api key in ~/.wakatime.cfg, prompt user to enter if does not exist
    -- TODO: Process event listeners to detect when current file changes, a file is modified, and a file is saved
    --[[   Current file changed (our file change event listener code is run)
        go to Send heartbeat function with isWrite false
    User types in a file (our file modified event listener code is run)
        go to Send heartbeat function with isWrite false
    A file is saved (our file save event listener code is run)
        go to Send heartbeat function with isWrite true ]]
            assert(false && "Not implemented yet"); */


		seconds now =  duration_cast<seconds>(steady_clock::now().time_since_epoch());

        // TODO get that data!
		if (!(last_heartbeat + (2s* 60) < now || isWrite || project_info.changed)){
            LOG_D("wakatime/send_heartbet") << "No heartbeat to send";
			return false;
		}

        project_info.changed = false;
		last_heartbeat = now;



        wxArrayString *buffer = new wxArrayString();

        buffer->Add(wxString::Format("--language '%s'", plugin_info.type));

        //TODO use translation for default file name!
        buffer->Add(wxString::Format("--entity '%s'", project_info.file_name == nullptr ? "Unbenannt.ass": *project_info.file_name));
        buffer->Add(wxString::Format("--project '%s'", project_info.project_name == nullptr ?  "Unbenannt" : *project_info.project_name));

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

        wxArrayString *output = new wxArrayString();

		long returnCode = wxExecute("bash  -c \"realpath ~",*output, wxEXEC_SYNC);

        if(returnCode != 0){
            return false;
        }
        wxString* homePath = StringArrayToString(output);
		cli_path = new wxString(wxString::Format("%s/.wakatime/wakatime-cli",*homePath));
        LOG_D("CLI/PATH") << cli_path->ToAscii();
		if(!is_cli_present()){
            LOG_D("CLI/PATH") << "path not present!";
			return download_cli();
		}

		return true;


        }


       	bool cli::is_cli_present(){
			
            long returnCode = wxExecute(wxString::Format("bash  -c \"which '%s' > /dev/null\"",*cli_path), wxEXEC_SYNC);

            return returnCode == 0;
        }

        bool cli::download_cli(){
            //TODO generalize and finish, WIP
            return false;
             long returnCode = wxExecute(wxString::Format("bash  -c \"mkdir -p ~/.wakatime && wget  'https://github.com/wakatime/wakatime-cli/releases/download/v1.52.1-alpha.1/wakatime-cli-linux-amd64.zip -o ~/.wakatime/ > /dev/null && unzip && ln -S .... etc\"",*cli_path), wxEXEC_SYNC);

            return returnCode == 0;
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
		wakatime_cli = new cli();
	}

	void clear() {
		delete wakatime_cli;
        LOG_D("wakatime/clear");
	}


	void update(bool isWrite, agi::fs::path const* filename){
        if(filename != nullptr){
            wxString* temp_file_name = new wxString(filename->string());
            wxString* temp_project_name = new wxString(filename->parent_path().filename().string());
            wakatime_cli->project_info.changed =  wakatime_cli->project_info.file_name == nullptr || !wakatime_cli->project_info.file_name->IsSameAs(*temp_file_name)
                || wakatime_cli->project_info.project_name == nullptr || wakatime_cli->project_info.project_name->IsSameAs(*temp_project_name);
                LOG_D("wakatime/update") << "has changed: " << wakatime_cli->project_info.changed;

            if(wakatime_cli->project_info.changed){
                wakatime_cli->project_info.file_name = temp_file_name;
                wakatime_cli->project_info.project_name = temp_project_name; 
            }
        }
	    bool send_successfully = wakatime_cli->send_heartbeat(isWrite);
		LOG_D("wakatime/update") << "isWrite: " << isWrite << " send_successfully: " << send_successfully;
	}

} // namespace wakatime



