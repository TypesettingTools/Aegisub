// Copyright (c) 2014, Thomas Goyne <plorkyeran@aegisub.org>
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
//
// Aegisub Project http://www.aegisub.org/

#include <libaegisub/color.h>

#include <functional>
#include <string>

class AssFile;
class AsyncVideoProvider;
class wxArrayInt;
class wxArrayString;
class wxWindow;
namespace agi {
struct Context;
}
struct ResampleSettings;

/// @brief Get a color from the user via a color picker dialog
/// @param parent Parent window
/// @param original Initial color to select
/// @param alpha Include controls for alpha
/// @param callback Function called whenever the selected color changes
/// @return Did the user accept the new color?
bool GetColorFromUser(wxWindow* parent, agi::Color original, bool alpha,
                      std::function<void(agi::Color)> callback);

/// Ask the user to pick an autosaved file to open
/// @return Path to file or empty string if canceled
std::string PickAutosaveFile(wxWindow* parent);

/// @brief Check whether a newer version is available and report to the user if there is
/// @param interactive If true, always check and report all results, both success and failure.
///                    If false, only check if auto-checking is enabled, and only report if a
///                    new version actually exists.
void PerformVersionCheck(bool interactive);

/// Ask the user to pick settings for a script resampling
/// @return Does the user want to proceed with the resampling?
bool PromptForResampleSettings(agi::Context* c, ResampleSettings& settings);

/// Update the video properties for a newly opened video, possibly prompting the user about what to
/// do
void UpdateVideoProperties(AssFile* file, const AsyncVideoProvider* new_provider, wxWindow* parent);

int GetSelectedChoices(wxWindow* parent, wxArrayInt& selections, wxString const& message,
                       wxString const& caption, wxArrayString const& choices);

std::string CreateDummyVideo(wxWindow* parent);

bool ShowPasteOverDialog(wxWindow* parent);
bool ShowPlainTextImportDialog();
void ShowAboutDialog(wxWindow* parent);
void ShowAttachmentsDialog(wxWindow* parent, AssFile* file);
void ShowAutomationDialog(agi::Context* c);
void ShowExportDialog(agi::Context* c);
void ShowFontsCollectorDialog(agi::Context* c);
void ShowJumpToDialog(agi::Context* c);
void ShowKanjiTimerDialog(agi::Context* c);
void ShowLogWindow(agi::Context* c);
void ShowPreferences(wxWindow* parent);
void ShowPropertiesDialog(agi::Context* c);
void ShowSelectLinesDialog(agi::Context* c);
void ShowShiftTimesDialog(agi::Context* c);
void ShowSpellcheckerDialog(agi::Context* c);
void ShowStyleManagerDialog(agi::Context* c);
void ShowTimingProcessorDialog(agi::Context* c);
void ShowVideoDetailsDialog(agi::Context* c);
