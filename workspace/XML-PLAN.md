Comprehensive Plan: Aegisub macOS Fork with CineCanvas XML Export

Overview
This plan outlines the development of a macOS-focused fork of Aegisub (v3.4.1) that maintains all existing features while adding support for exporting CineCanvas-style XML subtitles for Digital Cinema Package (DCP) workflows.

Phase 1: Project Setup & Fork Configuration
1.1 Repository Setup
Fork branding: Rename/rebrand if desired (e.g., "Aegisub Cinema" or keep "Aegisub")
Update metadata:
Modify meson.build project name and version
Update packages/osx_bundle/Contents/Info.plist with new bundle identifier
Update copyright notices and credits in relevant files
Modify URLs in codebase (already done per git status)
1.2 macOS-Specific Optimizations
Current macOS support is good, but can be enhanced:

Minimum macOS version: Set deployment target (currently flexible)

Location: meson.build - add MACOS_X_DEPLOYMENT_TARGET environment variable handling
Recommend: macOS 11.0+ (Big Sur) for modern Cinema workflows
Architecture support:

Update Info.plist to support Apple Silicon (arm64) natively
Current LSArchitecturePriority only lists x86_64 and i386
Add: <string>arm64</string> as first priority
Code signing & notarization:

Add scripts for code signing in tools/
Configure entitlements for hardened runtime
Setup notarization workflow for distribution
Phase 2: CineCanvas XML Format Implementation
2.1 Understanding CineCanvas/Interop DCP Subtitles
Format Specification (Interop DCP Subtitles):

The CineCanvas format is based on the Interop DCP subtitle specification. Key characteristics:

XML Structure:

<?xml version="1.0" encoding="UTF-8"?>
<DCSubtitle Version="1.0">
  <SubtitleID>urn:uuid:[UUID]</SubtitleID>
  <MovieTitle>Movie Title</MovieTitle>
  <ReelNumber>1</ReelNumber>
  <Language>en</Language>
  <LoadFont Id="Font1" URI="font.ttf"/>
  <Font Id="Font1" Size="42" Weight="normal" Color="FFFFFFFF" Effect="border" EffectColor="FF000000">
    <Subtitle SpotNumber="1" TimeIn="00:00:05:000" TimeOut="00:00:08:000" FadeUpTime="20" FadeDownTime="20">
      <Text VAlign="bottom" VPosition="10.0">Subtitle text here</Text>
    </Subtitle>
  </Font>
</DCSubtitle>
Key Elements:

DCSubtitle (root): Version="1.0"
SubtitleID: UUID identifier (urn:uuid:xxxx format)
MovieTitle: Film/project name
ReelNumber: DCP reel number (usually 1-based)
Language: ISO 639-2 language code (e.g., "en", "fr", "de")
LoadFont: External font references
Font: Typography container with attributes:
Id: Font identifier
Size: Point size (typically 42-48 for cinema)
Weight: normal/bold
Color: RRGGBBAA hex (Alpha is opacity)
Effect: none/border/shadow
EffectColor: Effect color in RRGGBBAA
Subtitle: Individual subtitle entry
SpotNumber: Sequential number
TimeIn/TimeOut: Timecode format HH:MM:SS:mmm (250fps basis for Interop)
FadeUpTime/FadeDownTime: Milliseconds for fade effects
Text: Subtitle content
VAlign: top/center/bottom
VPosition: Percentage from screen edge (0-100)
HAlign: left/center/right (optional)
HPosition: Horizontal percentage (optional)
Technical Requirements:

Encoding: UTF-8
Timecode base: 24, 25, or 30 fps (edit units per second)
Color format: 8-character RRGGBBAA hex
Positioning: Percentage-based (0-100 scale)
Font embedding: Fonts typically referenced, not embedded in XML
2.2 Create CineCanvas Export Module
File: src/subtitle_format_cinecanvas.cpp and .h

Implementation strategy (following existing patterns from TTXT format):

// src/subtitle_format_cinecanvas.h
#pragma once
#include "subtitle_format.h"

class CineCanvasSubtitleFormat : public SubtitleFormat {
public:
    CineCanvasSubtitleFormat();
    std::vector<std::string> GetReadWildcards() const override;
    std::vector<std::string> GetWriteWildcards() const override;
    
    void ReadFile(AssFile *target, agi::fs::path const& filename, 
                  agi::vfr::Framerate const& fps, const char *encoding) const override;
    void WriteFile(const AssFile *src, agi::fs::path const& filename, 
                   agi::vfr::Framerate const& fps, const char *encoding="") const override;
    void ExportFile(const AssFile *src, agi::fs::path const& filename,
                    agi::vfr::Framerate const& fps, const char *encoding="") const override;
    
    bool CanSave(const AssFile *file) const override;
    
private:
    void ConvertToCineCanvas(AssFile &file) const;
    void WriteHeader(wxXmlNode *root, const AssFile *src) const;
    void WriteSubtitle(wxXmlNode *fontNode, const AssDialogue *line, 
                       int spotNumber, const agi::vfr::Framerate &fps) const;
    std::string ConvertColorToRGBA(const agi::Color &color, uint8_t alpha = 255) const;
    std::string GenerateUUID() const;
    void ParseFontAttributes(const AssStyle *style, wxXmlNode *fontNode) const;
    void ParseTextPosition(const AssDialogue *line, wxXmlNode *textNode) const;
};
Key implementation details:

Color Conversion: ASS uses BGR format, DCP uses RGBA

Convert from ASS color format to 8-char hex RRGGBBAA
Handle transparency/alpha channel mapping
Timing Conversion:

ASS uses centiseconds (0:00:00.00)
CineCanvas uses milliseconds (00:00:00:000)
Respect fps parameter for frame-accurate timing
Position Mapping:

ASS uses alignment codes (1-9) and margins
CineCanvas uses percentage-based VPosition/HPosition
Calculate screen percentages from ASS positioning
Style Conversion:

Map ASS font properties to CineCanvas Font attributes
Convert border/shadow to Effect/EffectColor
Handle font size scaling (ASS points to DCP points)
Text Processing:

Strip ASS override tags (use SubtitleFormat::StripTags())
Convert newlines (\N) to multi-line Text elements or <br/> tags
Handle special characters (XML escaping)
UUID Generation:

Generate RFC 4122 compliant UUIDs for SubtitleID
Use Boost UUID library (already a dependency)
2.3 Configuration & Preferences
Create configuration options for CineCanvas export:

File: libaegisub/common/option.cpp additions

Add configuration schema entries:

Subtitle Format/CineCanvas/Default Frame Rate: 24 (dropdown: 24, 25, 30)
Subtitle Format/CineCanvas/Movie Title: String (default: "Untitled")
Subtitle Format/CineCanvas/Reel Number: Integer (default: 1)
Subtitle Format/CineCanvas/Language Code: String (default: "en")
Subtitle Format/CineCanvas/Default Font Size: Integer (default: 42)
Subtitle Format/CineCanvas/Fade Duration: Integer (default: 20ms)
Subtitle Format/CineCanvas/Include Font Reference: Boolean (default: false)
File: src/preferences.cpp additions

Create preference dialog section: "CineCanvas Export Settings"

GUI controls for all above options
Frame rate selector with common DCP rates
Language code dropdown (ISO 639-2 codes)
Validation for proper format
2.4 Export Dialog Integration
File: src/dialog_export.cpp modifications

Add CineCanvas format to export format list
Show CineCanvas-specific options when format selected
Validation warnings for:
Unsupported ASS features (animations, complex effects)
Font embedding limitations
Color space compatibility
Phase 3: Build System Integration
3.1 Meson Build Configuration
File: src/meson.build

Add new source files to compilation:

aegisub_sources = files([
    # ... existing files ...
    'subtitle_format_cinecanvas.cpp',
])
3.2 Dependencies
Check existing dependencies - No new dependencies required:

✅ wxXML (already used for TTXT format)
✅ Boost (UUID library available)
✅ ICU (for character encoding)
3.3 macOS Bundle Integration
File: packages/osx_bundle/Contents/Info.plist

Add CineCanvas XML as supported document type:

<!-- .xml (CineCanvas) -->
<dict>
    <key>CFBundleTypeExtensions</key>
    <array>
        <string>xml</string>
    </array>
    <key>CFBundleTypeIconFile</key>
    <string>xmlIcon</string>
    <key>CFBundleTypeName</key>
    <string>CineCanvas DCP Subtitle</string>
    <key>CFBundleTypeRole</key>
    <string>Editor</string>
    <key>LSTypeIsPackage</key>
    <false/>
    <key>LSHandlerRank</key>
    <string>Alternate</string>
</dict>
Create icon: packages/osx_bundle/Contents/Resources/xmlIcon.icns

Design icon for XML/DCP files
Include in bundle resources
Phase 4: Feature Enhancements
4.1 DCP-Specific Features
Add DCP preview mode:

File: src/video_display.cpp enhancements
Render subtitles with DCP-compliant positioning
Show safe areas for cinema screens
Display color space warnings (DCP uses XYZ color space)
Add DCP validation:

New file: src/validators/dcp_validator.cpp
Check subtitle count limits (typically <500 per reel)
Validate timecodes don't overlap
Check character count per line (recommended <40-50 chars)
Verify color contrast meets DCI specifications
Flag unsupported features
Add batch export:

File: src/command/subtitle.cpp additions
Command: "Export multiple files as CineCanvas"
Support for multi-reel DCPs (split by time ranges)
Auto-increment reel numbers
UUID management across reels
4.2 Font Management for DCP
Font handling strategy:

New file: src/dcp_font_manager.cpp
Scan used fonts in ASS file
Generate LoadFont references
Option to embed fonts vs. reference external
Convert font names to DCP-compatible identifiers
4.3 Import CineCanvas (Optional)
Bi-directional support:

Implement ReadFile() in CineCanvasSubtitleFormat
Parse XML and convert to ASS format
Map DCP positioning to ASS alignment
Preserve timing and styling information
Handle font references
Phase 5: Testing Strategy
5.1 Unit Tests
File: tests/test_cinecanvas.cpp (new)

Test cases:

✅ Color conversion (ASS BGR to DCP RGBA)
✅ Timing conversion (various frame rates)
✅ Position calculation (alignment to percentage)
✅ UUID generation (RFC 4122 compliance)
✅ XML structure validation
✅ Special character escaping
✅ Multi-line text handling
✅ Style attribute mapping
Integration with existing test framework:

Uses GTest/GMock (already in dependencies)
Add to tests/meson.build
5.2 Format Validation Tests
XML Schema Validation:

Create XSD schema for CineCanvas format
Validate generated XML against schema
Test with real DCP mastering tools:
DCP-o-matic
OpenDCP
easyDCP
5.3 Round-trip Tests
Conversion accuracy:

Export ASS → CineCanvas XML
Import CineCanvas XML → ASS
Compare original vs. round-tripped data
Measure fidelity loss
5.4 Real-world Testing
Cinema validation:

Test on actual DCP players if available
Verify timing accuracy at 24/25/30fps
Check positioning on various screen ratios
Color accuracy verification
Font rendering consistency
Phase 6: Documentation
6.1 User Documentation
Create: docs/CineCanvas_Export_Guide.md

Contents:

Introduction to CineCanvas/DCP subtitles
Step-by-step export workflow
Configuration options explained
Best practices for cinema subtitles
Troubleshooting common issues
Limitations and known issues
Update: README.md

Add CineCanvas export to features list
macOS-specific build instructions
Link to detailed documentation
6.2 Technical Documentation
Create: docs/CineCanvas_Technical_Spec.md

Contents:

XML format specification
Mapping from ASS to CineCanvas
Color space conversion details
Timing calculations
Position algorithms
Font handling approach
Code documentation:

Doxygen comments in header files
Inline comments for complex algorithms
Examples in function documentation
6.3 Build Documentation
macOS build guide enhancements:

Apple Silicon specific instructions
Universal binary creation
Code signing and notarization steps
DMG creation with cinema-focused branding
Phase 7: Distribution & Packaging
7.1 macOS Application Bundle
Bundle structure:

AegisubCinema.app/
├── Contents/
│   ├── Info.plist (modified)
│   ├── MacOS/
│   │   └── aegisub (universal binary)
│   ├── Resources/
│   │   ├── Aegisub.icns
│   │   ├── xmlIcon.icns (new)
│   │   ├── *.lproj/ (localizations)
│   │   └── fonts/ (optional DCP fonts)
│   └── SharedSupport/
│       ├── config/
│       └── automation/
Code signing:

codesign --force --deep --sign "Developer ID Application: [Your ID]" \
         --options runtime \
         --entitlements entitlements.plist \
         AegisubCinema.app
Notarization:

xcrun notarytool submit AegisubCinema.dmg \
                        --apple-id [email] \
                        --team-id [team] \
                        --password [password]
7.2 DMG Creation
File: tools/create_dmg.sh (enhance existing)

Features:

Custom background for cinema context
Drag-to-Applications setup
License agreement display
README with CineCanvas quick start
Volume icon
Build command:

meson compile osx-build-dmg -C build_static
7.3 Homebrew Formula (Optional)
Create: homebrew/aegisub-cinema.rb

Enable easy installation:

brew install --cask aegisub-cinema
Phase 8: Maintenance & Features Roadmap
8.1 Priority 1 (Must Have)
✅ CineCanvas XML export functionality
✅ macOS build system fully functional
✅ Universal binary (Intel + Apple Silicon)
✅ Code signing and notarization
✅ Basic documentation
8.2 Priority 2 (Should Have)
✅ CineCanvas XML import functionality
✅ DCP validation warnings
✅ Cinema preview mode
✅ Batch export for multi-reel DCPs
✅ Comprehensive test suite
8.3 Priority 3 (Nice to Have)
SMPTE Timed Text support (newer DCP standard)
Direct integration with DCP mastering tools
Cloud subtitle validation service
Templates for common cinema subtitle styles
Automated QC reports
8.4 Ongoing Maintenance
Keep up with upstream Aegisub changes
macOS version compatibility updates
Bug fixes and user feedback
Performance optimizations
Security updates
Implementation Timeline Estimate
Week 1-2: Setup & Infrastructure
Fork configuration and branding
macOS build system verification
Development environment setup
Dependencies audit
Week 3-4: Core CineCanvas Implementation
Create subtitle_format_cinecanvas.cpp/h
Implement WriteFile() and ExportFile()
Color and timing conversion functions
XML generation logic
Week 5-6: Integration & Configuration
Configuration options integration
Export dialog enhancements
Info.plist updates
Icon creation
Week 7-8: Testing & Validation
Unit test development
XML schema validation
Round-trip testing
Real-world DCP testing
Week 9-10: Documentation & Polish
User documentation
Technical specifications
Code documentation
Build instructions
Week 11-12: Distribution & Release
Code signing setup
Notarization workflow
DMG creation and testing
Release preparation
Total: ~12 weeks for complete implementation

Risk Assessment & Mitigation
Technical Risks
| Risk | Impact | Probability | Mitigation | |------|--------|-------------|------------| | CineCanvas spec misinterpretation | High | Medium | Validate with DCP tools early | | Color space conversion errors | High | Medium | Extensive testing, reference implementations | | Timing drift at different fps | Medium | Low | Use proven timing libraries | | macOS API deprecation | Medium | Low | Use latest SDK, monitor Apple docs | | Code signing issues | Low | Medium | Test early, use Apple documentation |

Project Risks
| Risk | Impact | Probability | Mitigation | |------|--------|-------------|------------| | Upstream Aegisub divergence | Medium | High | Regular merge from upstream | | Limited DCP testing access | Medium | Medium | Partner with cinema or use simulators | | User adoption challenges | Low | Medium | Clear documentation, examples | | Font licensing issues | Medium | Low | Document font requirements clearly |

Success Criteria
Functional Requirements
✅ Successfully export ASS files to valid CineCanvas XML
✅ Output passes DCP validation tools
✅ Plays correctly in DCP players
✅ Maintains all core Aegisub functionality
✅ Runs natively on macOS (Intel + Apple Silicon)
Quality Requirements
✅ 90%+ test coverage for CineCanvas module
✅ Zero critical bugs at release
✅ Build time < 10 minutes on modern Mac
✅ App size < 100MB
✅ User documentation complete
Performance Requirements
✅ Export 1000 subtitles in < 5 seconds
✅ Launch time < 3 seconds
✅ Memory usage < 200MB for typical project
✅ Responsive UI (no blocking operations > 100ms)
Conclusion
This comprehensive plan provides a roadmap for creating a macOS-focused fork of Aegisub with professional CineCanvas XML export capabilities. The implementation follows Aegisub's existing architectural patterns, leverages proven dependencies, and maintains code quality standards.

Key advantages of this approach:

Minimal disruption to existing Aegisub functionality
Follows established code patterns (TTXT format as template)
Uses existing dependencies (no new external requirements)
Clear separation of concerns (new format = new module)
Comprehensive testing strategy
Professional documentation
Production-ready distribution
Next steps:

Confirm CineCanvas specification details (if PDF becomes available)
Set up development environment
Begin Phase 1 (Project Setup)
Implement core CineCanvas export (Phase 2)
Iterate based on testing and feedback
The plan is designed to be executed incrementally, with each phase building on the previous one, allowing for course corrections based on testing and validation results.