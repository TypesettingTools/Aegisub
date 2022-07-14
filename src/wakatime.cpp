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
#include <functional>
#include <string>
#include <ostream>
#include <sstream>
#include <iostream>

#include <wx/string.h>
#include <wx/arrstr.h>
#include <wx/utils.h>
#include <wx/process.h>
#include <wx/stream.h>
#include <wx/event.h>

namespace wakatime {


    wxString* StringArrayToString(wxArrayString* input, wxString* seperator){
            wxString* output = new wxString();
            for(size_t i=0; i < input->GetCount(); ++i){
                output->Append(i == 0 ? input->Item(i) :(wxString::Format("%s%s",*seperator,input->Item(i))));
            }
        return output;
    }

    #define BUF_SIZE 1024
    wxString* ReadInputStream(wxInputStream* input){
        wxString* output = new wxString();
        void* buffer = malloc(BUF_SIZE);
        while(input->CanRead() && !input->Eof()){
            input->Read(buffer, BUF_SIZE);
            size_t read = input->LastRead();
            wxStreamError error = input->GetLastError();
            switch(error){
                case wxSTREAM_NO_ERROR:
                case wxSTREAM_EOF:
                    output->append(wxString::FromUTF8((char*)buffer, read));
                    return output;
                case wxSTREAM_WRITE_ERROR:
                    LOG_E("wxInputStream WRITE ERROR");
                    output->Empty();
                    return output;
                case wxSTREAM_READ_ERROR:
                    LOG_E("wxInputStream READ ERROR");
                    output->Empty();
                    return output;
                default:
                    assert(false && "UNREACHABLE");
            }
        }


        return output;
    }


    wxString* StringArrayToString(wxArrayString* input, const char * seperator = " "){
        return StringArrayToString(input, new wxString(seperator));
    }


        std::ostream& operator<<(std::ostream& os,  const wakatime::CLIResponse& arg){
                os << "CLIResponse: isOk: " << (arg.ok ? "yes" : "no" )<< "\n\tstreams:\n" <<
                "\t\terror: '" << (arg.error_string == nullptr ? "NULL" : arg.error_string->ToAscii()) << "'\n" <<
                "\t\toutput: '" << (arg.output_string == nullptr ? "NULL" : arg.output_string->ToAscii()) << "'\n\n"
                ;
                return os;
        }

        std::ostream& operator<<(std::ostream& os, const wakatime::CLIResponse * arg){
                os << (*arg);
                return os;
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

        void cli::send_heartbeat(bool isWrite){

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
			return;
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

        invoke_cli_async(buffer, [this](CLIResponse response_string)->void{

            get_time_today();

        });
	}

    void cli::get_time_today(){

        wxArrayString *buffer = new wxArrayString();
        buffer->Add("--today");
        invoke_cli_async(buffer,[this](CLIResponse time_today)->void{
                if(time_today.ok){
                    LOG_D("time/today") << time_today.output_string->ToAscii();
                }
        
            
        });
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
 
        void cli::invoke_cli_async(wxArrayString* options, std::function<void ( CLIResponse response)> callback){

        options->Add("--verbose");

        //TODO also version should be dynamic!
        options->Add(wxString::Format("--plugin 'aegisub/%s %s/8975-master-8d77da3'",plugin_info.version, plugin_info.plugin_name));

        wxString command = wxString::Format("%s %s", *cli_path, * StringArrayToString(options));

        LOG_D("wakatime/execute") << command.ToAscii();


    wxProcess* process = new wxProcess();
    process->Redirect();


        std::function< void (wxProcessEvent& event)> lambda = ([process, command, callback](wxProcessEvent& event)-> void{

            CLIResponse response = {
                ok:true,
                error_string : nullptr,
                output_string: nullptr
            };
            
        wxInputStream* outputStream = process->GetInputStream() ;
        if(outputStream == nullptr){
               LOG_E("wakatime/execute/async") << "Command couldn't be executed: " << command.ToAscii();
            response.error_string = new wxString("Stdout was NULL");
            response.ok = false;
             callback(response);
             return;
        }
        
        wxInputStream* errorStream = process->GetErrorStream() ;
        if(errorStream == nullptr){
               LOG_E("wakatime/execute/async") << "Command couldn't be executed: " << command.ToAscii();
            response.error_string = new wxString("Stderr was NULL");
            response.ok = false;
             callback(response);
             return;
        }


        wxString* error_string = ReadInputStream(errorStream);
        wxString* output_string = ReadInputStream(outputStream);


        if(event.GetExitCode() != 0 || output_string == nullptr || !error_string->IsEmpty()){
            LOG_E("wakatime/execute/async") << "Command couldn't be executed: " << command.ToAscii();
  
            LOG_E("wakatime/execute/async") << "The Errors were: " << error_string->ToAscii();
            
            response.error_string = error_string;
            response.ok = false;
             callback(response);
             return;
        } 


        LOG_D("wakatime/output") << " " << output_string->ToAscii();

        response.output_string = output_string;


		    callback(response);
            return;
        });
        // wxEventTag, wxProcessEvent

        process->Bind(wxEVT_END_PROCESS, lambda);



      long pid = wxExecute(command, wxEXEC_ASYNC,process);

        if(pid == 0){
            LOG_E("wakatime/execute") << "Command couldn't be executed: " << command.ToAscii();
            return;
        }
    }



    CLIResponse cli::invoke_cli_sync(wxArrayString* options){
		//	assert(false && "Not implemented yet");

        CLIResponse response = {
            ok:true,
            error_string : nullptr,
            output_string: nullptr
        };

        options->Add("--verbose");

        //TODO also version should be dynamic!
        options->Add(wxString::Format("--plugin 'aegisub/%s %s/8975-master-8d77da3'",plugin_info.version, plugin_info.plugin_name));

        wxString command = wxString::Format("%s %s", *cli_path, * StringArrayToString(options));
        LOG_D("wakatime/execute") << command.ToAscii();


        wxArrayString *output = new wxArrayString();
        wxArrayString *errors = new wxArrayString();

        long returnCode = wxExecute(command, *output, *errors, wxEXEC_SYNC & wxEXEC_NODISABLE);
        if(returnCode != 0 || !errors->IsEmpty()){
            LOG_E("wakatime/execute") << "Command couldn't be executed: " << command.ToAscii();
            wxString* error_string = StringArrayToString(errors,"\n");
            LOG_E("wakatime/execute") << "The Errors were: " << error_string->ToAscii();
            
            response.error_string = error_string;
            response.ok = false;
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
	    wakatime_cli->send_heartbeat(isWrite);
		LOG_D("wakatime/update") << "initiated async request";
	}

} // namespace wakatime



