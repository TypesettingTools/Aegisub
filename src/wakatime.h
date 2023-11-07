// Copyright (c) 2022-2023, Totto16
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

#include "frame_main.h"
#include "libaegisub/path.h"
#include "options.h"
#include "version.h"

#include <chrono>
#include <expected>
#include <functional>

#include <wx/arrstr.h>
#include <wx/string.h>

namespace wakatime {

void init();

void clear();

void update(bool isWrite, agi::fs::path const &filename);

void update(bool isWrite);

void setUpdateFunction(std::function<void()> updateFunction);

std::string getTime();

typedef struct {
  const std::string program;
  const std::string plugin_name;
  const std::string short_type;
  const std::string long_type;
  const std::string plugin_version;
  const std::string aegisub_version;
} Plugin;

typedef struct {

  std::string error_string;  // is empty if no error occured
  std::string output_string; // is empty if an error occured

  [[nodiscard]] bool ok() const { return this->error_string.empty(); };

} CLIResponse;

std::ostream &operator<<(std::ostream &os, const wakatime::CLIResponse &arg);

std::ostream &operator<<(std::ostream &os, const wakatime::CLIResponse *arg);

typedef struct {
  std::string project_name;
  std::string file_name;
  bool changed;
} ProjectInfo;

/// @class cli
/// the wakatime cli class, it has the needed methods
class cli {
public:
  cli(Plugin &plugin_info);

  bool initialize();
  void change_project(std::string &new_file, std::string &project_name);
  void change_api_key(std::string &key);
  void send_heartbeat(bool isWrite);
  ProjectInfo project_info = {
    project_name : "",
    file_name : "",
    changed : false
  };

  std::string currentTime = "";
  std::function<void()> updateFunction;

private:
  std::string key = " ";
  std::chrono::seconds last_heartbeat;
  bool debug;
  Plugin plugin_info;
  std::string cli_path = "";
  bool cliInstalled;

  void get_time_today();
  bool handle_cli();
  bool is_cli_present();
  bool download_cli();
  bool is_key_valid(std::string &key);
  void getKey();
  void getDebug();
  void setTime(std::string text);

  void invoke_cli_async(std::vector<std::string> &options,
                        std::function<void(CLIResponse response)> callback);
};
} // namespace wakatime
