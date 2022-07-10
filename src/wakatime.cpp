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

#include "libaegisub/fs.h"
#include "libaegisub/io.h"
#include "libaegisub/json.h"
#include "libaegisub/log.h"

#include <chrono>

namespace wakatime {

using namespace std::chrono;

/// @class cli
/// the wakatime cli class, it has the needed methods
    class cli {
    public:
        cli (char* project_name){
            if(!handle_cli()){
				assert(false && "ERROR");
			}
            this->project_name = project_name;

        	last_heartbeat = 0s;

			send_heartbeat("file",false);

			auto aegisub_key =  nullptr; //"8c982ef8-3baa-44d6-865b-203949200c5e";//agi::config::getKey(wakatime_key_setting);
			if(aegisub_key == nullptr){
				char* buffer[1] = {"--config-read api_key"};
				auto stored_api_key = invoke_cli(buffer,1);
				if (stored_api_key != nullptr && stored_api_key != ""){
					key = stored_api_key;
					//agi::config::setKey(wakatime_key_setting,key);
				}
			}else{
				key = aegisub_key;
			}

        }

        void change_project(char* project_name, char* new_file){
			this->project_name = project_name;
			send_heartbeat(new_file,true, true);
		}
        void change_api_key(char* key){
			this->key = key;
			// TODO check validity of key!
		}
        bool send_heartbeat(char* file, bool isWrite, bool force = false){

		seconds now =  duration_cast<seconds>(steady_clock::now().time_since_epoch());

		if (last_heartbeat + (2s* 60) < now && !force){
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
    char* buffer[3] = {};

// TODO don't use asprintf 
    asprintf( &buffer[0],"--language '%s'", type);
	asprintf( &buffer[1],"--entity  '%s'", type);
    if (isWrite ){
        buffer[2] = "--write";
    }

 // char* || bool ?
    auto send_successful = invoke_cli(buffer, isWrite ? 3: 2);


    char* buffer2[1] = { "--today"};

    auto time_today = invoke_cli(buffer2,1);

	LOG_D("time/today") << time_today;

	}

    private:
        char * project_name;
        char * key;
        seconds last_heartbeat;
        const char* program = "Aegisub";
        const char* plugin_name = "wakatime-aegisub";
        const char* type = "Advanced SubStation Alpha";
        const char* version = "0.0.1";
        char* cli_path;

        bool handle_cli(){
			cli_path = "/home/totto/.wakatime/wakatime-cli";
			if(!is_cli_present()){
				return download_cli();
			}

			return true;

/* 

    -- TODO:   Check for wakatime-cli, or download into ~/.wakatime/ if missing or needs an update
    --TODO:    Check for api key in ~/.wakatime.cfg, prompt user to enter if does not exist
    -- TODOD: but not really possible:    Setup event listeners to detect when current file changes, a file is modified, and a file is saved
    --[[   Current file changed (our file change event listener code is run)
        go to Send heartbeat function with isWrite false
    User types in a file (our file modified event listener code is run)
        go to Send heartbeat function with isWrite false
    A file is saved (our file save event listener code is run)
        go to Send heartbeat function with isWrite true ]]
            assert(false && "Not implemented yet"); */
        }

       	bool is_cli_present(){
			// TODO check if cli_path exists!
			return true;
            assert(false && "Not implemented yet");
        }

        bool download_cli(){
            assert(false && "Not implemented yet");
			return false;
        }

        char* invoke_cli(char** options, size_t options_size){
		//	assert(false && "Not implemented yet");

		#define ADD_PARAM_SZ 3
		size_t additional_params_size = ADD_PARAM_SZ;
		char* additional_params[ADD_PARAM_SZ] = {cli_path,"","--verbose"};
		asprintf(&additional_params[1],"--plugin 'aegisub/%s %s/8975-master-8d77da3'",version, plugin_name);

		char** buffer = (char**)malloc(sizeof(char)*(options_size+1+additional_params_size));

		for(int i=0; i < additional_params_size; ++i){
			buffer[i] = additional_params[i];
		}


		buffer[options_size+1] = nullptr;
		for(int i=0; i < options_size; ++i){
			buffer[i+additional_params_size] = options[i];
		}
		// TODO execve and read stdout
/* 
    local handle = io.popen(cli)
    local result = handle:read("*a")
    handle:close()

    return result
		} */

		return "";
    };


	};


	wakatime::cli *wakatime_cli = nullptr;
	void init() {
		//wakatime_cli = new cli("project name");
	}

	void clear() {
		delete wakatime_cli;
	}


	void update(bool isWrite){
	//TODO: 	assert(false && "Not implemented yet");
	//	wakatime_cli->send_heartbeat("file",isWrite);
		LOG_D("time/update") << "isWrite: " << isWrite;
	}

} // namespace wakatime



