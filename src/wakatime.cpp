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

#include "frame_main.h"
#include "options.h"
#include "wakatime.h"

#include "libaegisub/log.h"
#include "libaegisub/path.h"

#include <chrono>
#include <functional>
#include <iostream>
#include <numeric>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>

#include <wx/arrstr.h>
#include <wx/event.h>
#include <wx/process.h>
#include <wx/stream.h>
#include <wx/string.h>
#include <wx/utils.h>

namespace wakatime {

std::string StringArrayToString(std::vector<std::string> &input,
                                const char *const delimiter = " ") {
  return std::accumulate(
      std::next(input.begin()), input.end(), input[0],
      [delimiter](std::string a, std::string b) { return a + delimiter + b; });
}

std::string StringArrayToString(wxArrayString *input,
                                const char *const delimiter = " ") {
  std::vector<std::string> result{};
  result.reserve(input->GetCount());
  for (size_t i = 0; i < input->GetCount(); ++i) {

    result.push_back(input->Item(i).ToStdString());
  }
  return StringArrayToString(result, delimiter);
}

#define BUF_SIZE 1024
wxString *ReadInputStream(wxInputStream *input, bool trimLastNewLine = false) {
  wxString *output = new wxString();
  // TODO and when did I think I free that xD LOL
  void *buffer = malloc(BUF_SIZE);
  while (input->CanRead() && !input->Eof()) {
    input->Read(buffer, BUF_SIZE);
    size_t read = input->LastRead();
    wxStreamError error = input->GetLastError();
    switch (error) {
    case wxSTREAM_NO_ERROR:
    case wxSTREAM_EOF:
      if (*((char *)buffer + strlen((char *)buffer) - 1) == '\n' &&
          trimLastNewLine) {
        read -= 1;
      }
      output->append(wxString::FromUTF8((char *)buffer, read));
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
      assert(false && "UNREACHABLE default in switch case");
    }
  }

  if (output->EndsWith("\n") && trimLastNewLine) {
    output->Trim(true);
  }
  return output;
}

std::ostream &operator<<(std::ostream &os, const wakatime::CLIResponse &arg) {
  os << "CLIResponse: isOk: " << (arg.ok() ? "yes" : "no") << "\n\tstreams:\n"
     << "\t\terror: '" << (arg.error_string.empty() ? "NULL" : arg.error_string)
     << "'\n"
     << "\t\toutput: '"
     << (arg.output_string.empty() ? "NULL" : arg.output_string) << "'\n\n";
  return os;
}

std::ostream &operator<<(std::ostream &os, const wakatime::CLIResponse *arg) {
  os << (*arg);
  return os;
}

cli::cli(Plugin &plugin_info) : plugin_info{plugin_info} {
  if (!handle_cli()) {
    this->cliInstalled = false;
    return;
  }
  this->cliInstalled = true;

  last_heartbeat = std::chrono::duration_cast<std::chrono::seconds>(
      std::chrono::steady_clock::now().time_since_epoch());
  ;

  OPT_SUB("Wakatime/API_Key", &cli::getKey, this);
  OPT_SUB("Wakatime/Debug", &cli::getDebug, this);

  this->setTime("Loading...");

  this->getDebug();
  this->getKey();

  this->setTime("No Project Selected");
}

void cli::change_project(std::string &new_file, std::string &project_name) {
  project_info.file_name = new_file;
  project_info.project_name = project_name;
  project_info.changed = true;

  send_heartbeat(false);
}

void cli::change_api_key(std::string &key) {
  this->key = key;
  // TODO check validity of key!
}

void cli::send_heartbeat(bool isWrite) {

  if (this->key.empty()) {
    return;
  }

  std::chrono::seconds now = std::chrono::duration_cast<std::chrono::seconds>(
      std::chrono::steady_clock::now().time_since_epoch());

  if (!((last_heartbeat + std::chrono::minutes(2) < now) || isWrite ||
        project_info.changed)) {
    return;
  }

  LOG_D("plugin/wakatime") << "send_heartbeat - isWrite: " << isWrite
                           << " project_info.changed: " << project_info.changed
                           << " last_heartbeat: "
                           << (now - last_heartbeat).count()
                           << " seconds ago\n";

  project_info.changed = false;
  last_heartbeat = now;

  std::vector<std::string> buffer{};

  buffer.push_back("--language '" + plugin_info.short_type + "'");
  buffer.push_back("--alternate-language '" + plugin_info.long_type + "'");

  // TODO use translation for default file name!
  //  subs_controller->Filename() return "Unbenannt" or "Untitled" when no file
  //  is loaded! (.ass has to be set additionally, and file path is ""!)

  buffer.push_back("--entity '" +
                   (project_info.file_name.empty() ? "Unbenannt.ass"
                                                   : project_info.file_name) +
                   "'");
  // "--project" gets detected by the folder name! (the manual project name is
  // also the folder name, atm at least!)
  buffer.push_back("--alternate-project '" +
                   (project_info.project_name.empty()
                        ? "Unbenannt"
                        : project_info.project_name) +
                   "'");

  // TODO: if project_info.changed, should --write be enabled?
  if (isWrite) {
    buffer.push_back("--write");
  }

  invoke_cli_async(buffer, [this](CLIResponse response_string) -> void {
    get_time_today();
  });
}

void cli::get_time_today() {

  std::vector<std::string> buffer{};
  buffer.push_back("--today");
  // TODO: maybe add an option for that?
  // buffer->Add("--today-hide-categories building"); // e.g.
  // Can be "coding", "building", "indexing", "debugging", "running tests",
  // "writing tests", "manual testing", "code reviewing", "browsing", or
  // "designing". Defaults to "coding".
  invoke_cli_async(buffer, [this](CLIResponse time_today) -> void {
    if (time_today.ok()) {
      this->setTime(time_today.output_string);
    }
  });
}

bool cli::handle_cli() {

  wxArrayString *output = new wxArrayString();
  // TODO: all this commands aren't os independent, fix that!
  long returnCode = wxExecute("bash  -c \"realpath ~", *output, wxEXEC_SYNC);

  if (returnCode != 0) {
    return false;
  }
  std::string homePath = StringArrayToString(output);
  cli_path = homePath + "/.wakatime/wakatime-cli";
  if (!is_cli_present()) {
    return download_cli();
  }

  return true;
}

bool cli::is_cli_present() {

  long returnCode = wxExecute(
      wxString::Format("bash  -c \"which '%s' > /dev/null\"", cli_path),
      wxEXEC_SYNC);

  return returnCode == 0;
}

bool cli::download_cli() {

  // TODO generalize (os independent) and finish, WIP
  return false;
  long returnCode = wxExecute(
      wxString::Format("bash  -c \"mkdir -p ~/.wakatime && wget  "
                       "'https://github.com/wakatime/wakatime-cli/releases/"
                       "download/v1.86.5/wakatime-cli-linux-amd64.zip' -o "
                       "~/.wakatime/ > /dev/null && unzip && ln -S .... etc\"",
                       cli_path),
      wxEXEC_SYNC);

  return returnCode == 0;
}

bool is_key_valid(std::string &key) {
  // TODO: invoke cli with --key and get return code!
  return true;
}

void cli::getDebug() { this->debug = OPT_GET("Wakatime/Debug")->GetBool(); }

void cli::setTime(std::string time) {
  this->currentTime = time;
  if (this->updateFunction != nullptr) {
    this->updateFunction();
  }
}

void cli::getKey() {
  std::string aegisub_key = OPT_GET("Wakatime/API_Key")->GetString();
  if (aegisub_key.empty()) {
    std::vector<std::string> buffer{};
    buffer.push_back("--config-read api_key");
    invoke_cli_async(buffer, [this](CLIResponse response) -> void {
      if (response.ok()) {
        this->key = response.output_string;
        OPT_SET("Wakatime/API_Key")->SetString(this->key);
      }
    });
  } else {
    this->key = aegisub_key;
    std::vector<std::string> buffer{};
    buffer.push_back("--config-write api_key=" + this->key);
    invoke_cli_async(buffer, [this](CLIResponse response) -> void {
      if (!response.ok()) {
        LOG_E("wakatime/execute/async")
            << "Couldn't save the wakatime key to the config, reason:\n"
            << response.error_string;
      }
    });
  }
}

void cli::invoke_cli_async(std::vector<std::string> &options,
                           std::function<void(CLIResponse response)> callback) {

  try {

    if (!this->cliInstalled) {
      CLIResponse response = {
        error_string : "Wakatime CLI not installed",
        output_string : ""
      };
      callback(response);
      return;
    }

    if (this->debug) {
      options.push_back("--verbose");
    }

    if (!this->key.empty()) {
      options.push_back("--key '" + this->key + "'");
    }
    options.push_back("--plugin 'aegisub/" + plugin_info.plugin_version + " " +
                      plugin_info.plugin_name + "/" +
                      plugin_info.aegisub_version + "'");

    wxString command =
        wxString::Format("%s %s", cli_path, StringArrayToString(options));

    // since we handle the OnTerminate indirectly via event handler, we have to
    // delete the process to close opened pipes and free other resources, when
    // it's okay< to do so, and its okay after the callback returns it!
    wxProcess *process = new wxProcess();
    process->Redirect();

    std::function<void(wxProcessEvent & event)> lambda =
        ([process, callback, command](wxProcessEvent &event) -> void {
          CLIResponse response = {error_string : "", output_string : ""};

          wxInputStream *outputStream = process->GetInputStream();
          if (outputStream == nullptr) {
            LOG_E("wakatime/execute/async")
                << "Command couldn't be executed: " << command.utf8_str();
            response.error_string = "Stdout was NULL";
            callback(response);
            delete process;
            return;
          }

          wxInputStream *errorStream = process->GetErrorStream();
          if (errorStream == nullptr) {
            LOG_E("wakatime/execute/async")
                << "Command couldn't be executed: " << command.utf8_str();
            response.error_string = "Stderr was NULL";
            callback(response);
            delete process;
            return;
          }

          wxString *error_string = ReadInputStream(errorStream, true);
          wxString *output_string = ReadInputStream(outputStream, true);

          if (event.GetExitCode() != 0 || output_string == nullptr ||
              !error_string->IsEmpty()) {
            LOG_E("wakatime/execute/async")
                << "Command couldn't be executed: " << command.utf8_str();

            LOG_E("wakatime/execute/async")
                << "The Errors were: " << error_string->ToStdString() << "\n"
                << "exitCode: " << event.GetExitCode();

            response.error_string = error_string->ToStdString();
            callback(response);
            delete process;
            return;
          }

          response.output_string = output_string->ToStdString();

          callback(response);
          delete process;
          return;
        });

    process->Bind(wxEVT_END_PROCESS, lambda);

    LOG_D("wakatime/execute") << "Executing: " << command.utf8_str();

    long pid = wxExecute(command, wxEXEC_ASYNC, process);

    if (pid == 0) {
      LOG_E("wakatime/execute")
          << "Command couldn't be executed: " << command.utf8_str();
      return;
    }

  } catch (const std::exception &exc) {
    LOG_E("wakatime/execute") << "Caught exception: " << exc.what();
    return;
  }
}

wakatime::cli *wakatime_cli = nullptr;
void init() {

  Plugin plugin_info = {
    program : "Aegisub",
    plugin_name : "aegisub-wakatime",
    short_type : "ASS",
    long_type : "Advanced SubStation Alpha",
    plugin_version : "1.2.0",
    aegisub_version : GetAegisubLongVersionString(),
  };

  wakatime_cli = new cli(plugin_info);
};

void clear() { delete wakatime_cli; }

void setUpdateFunction(std::function<void()> updateFunction) {
  updateFunction();
  if (wakatime_cli == nullptr) {
    LOG_E("plugin/wakatime") << "ERROR: couldn't set the update function!\n";
    return;
  }
  wakatime_cli->updateFunction = updateFunction;
}

std::string getTime() {
  if (wakatime_cli->currentTime.empty()) {
    return std::string("");
  }
  return std::string(" - ") + wakatime_cli->currentTime;
}

void update(bool isWrite) {
  LOG_D("plugin/wakatime") << "update -  isWrite: " << isWrite << "\n";

  wakatime_cli->send_heartbeat(isWrite);
}

void update(bool isWrite, agi::fs::path const &filename) {
  LOG_D("plugin/wakatime") << "update -  isWrite: " << isWrite
                           << " filename: " << filename.string() << "\n";

  std::string temp_file_name = filename.string();
  std::string temp_project_name = filename.parent_path().filename().string();
  wakatime_cli->project_info.changed =
      wakatime_cli->project_info.file_name.compare(temp_file_name) != 0 ||
      wakatime_cli->project_info.project_name.compare(temp_project_name) != 0;

  if (wakatime_cli->project_info.changed) {
    wakatime_cli->project_info.file_name = temp_file_name;
    wakatime_cli->project_info.project_name = temp_project_name;
  }
  wakatime_cli->send_heartbeat(isWrite);
}

} // namespace wakatime
