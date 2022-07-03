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
#include <chrono>

namespace wakatime {

void init();

void clear();

void update(bool isWrite);


using namespace std::chrono;

/// @class cli
/// the wakatime cli class, it has the needed methods
    class cli {
    public:
        /// Map to hold Combo instances
        bool initialize(char* project_name);
        void change_project(char* project_name, char* new_file);
        void change_api_key(char* key);
        bool send_heartbeat(char* file, bool isWrite, bool force = false);

    private:
        char * project_name;
        char * key;
        seconds last_heartbeat;
        const char* program = "Aegisub";
        const char* plugin_name = "wakatime-aegisub";
        const char* type = "Advanced SubStation Alpha";
        const char* version = "0.0.1";
        char* cli_path;
        bool handle_cli();
        bool is_cli_present();
        bool download_cli();
        char* invoke_cli(char** options, size_t options_size);

    };
}