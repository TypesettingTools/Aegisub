# CineCanvas Documentation Index

## Overview

This directory contains complete documentation for the CineCanvas/Interop DCP subtitle format, developed to support professional cinema subtitle workflows in Aegisub.

**Documentation Created**: November 13, 2025  
**Based On**: Texas Instruments CineCanvas Specification Rev C (2005)  
**Purpose**: Enable ASS to CineCanvas XML conversion for DCP mastering

---

## Documentation Files

### 1. CineCanvas_Technical_Spec.md
**Size**: ~122 KB | **Lines**: 1,700+

**Complete technical specification covering**:

#### Core Format
- XML document structure and DTD
- All elements and attributes
- Validation rules
- Character encoding (UTF-8)

#### Color System (Section 8)
- RRGGBBAA format explained
- Color conversion algorithms
- Common cinema colors
- Contrast and readability standards
- Color space considerations

#### Timing System (Section 9)
- Frame-based timecodes
- Multiple frame rate support (24, 25, 30, 48 fps)
- Edit units and frame-accurate timing
- Fade effects (FadeUpTime/FadeDownTime)
- Best practices for subtitle duration

#### Positioning System (Section 10)
- Percentage-based coordinates (0-100%)
- VAlign/VPosition attributes
- HAlign/HPosition attributes
- Resolution-independent design
- Cinema safe areas

#### Advanced Features
- Ruby characters (furigana) for Japanese
- Image-based subtitles (Base64 encoded)
- Special typography (Space, HGroup, Rotate)
- Font management and LoadFont references
- Font optimization for CJK languages

#### Format Comparison (Section 13)
- Detailed ASS vs CineCanvas comparison
- Feature availability matrix
- Conversion challenges
- Mapping strategies

**Target Audience**: Developers, format researchers, QA testers

---

### 2. ASS_to_CineCanvas_Mapping.md
**Size**: ~80 KB | **Lines**: 1,400+

**Comprehensive conversion strategy with implementation details**:

#### Conversion Algorithms
- **Color Conversion** (Section 3)
  - BGR ↔ RGBA transformation
  - Alpha channel inversion
  - Complete C++ implementation
  - Test cases and examples

- **Timing Conversion** (Section 4)
  - Centiseconds to frames
  - Frame rate handling
  - Fade effect mapping
  - Code examples with edge cases

- **Position Conversion** (Section 5)
  - Alignment mapping (NumPad 1-9 → VAlign/HAlign)
  - Margin to percentage calculation
  - Pixel coordinates to percentage
  - Safe area enforcement

#### Content Handling
- Override tag processing
- Multi-line text splitting
- Special character escaping
- Font mapping strategies

#### Advanced Conversions
- Border and shadow effects
- Animation handling (limitations)
- Karaoke effects (workarounds)
- Drawing command to image conversion

#### Testing & Validation
- Unit test examples
- Integration testing strategy
- Round-trip testing
- Common issues and solutions

**Target Audience**: Implementation developers

---

### 3. Sample Files (samples/ directory)

#### sample_basic.xml
**Purpose**: Introduction to CineCanvas format  
**Features**:
- Single-line subtitles
- Multi-line subtitles
- Standard timing (24 fps)
- Basic fade effects
- Bottom-centered positioning

**Use Case**: Learning the format basics

---

#### sample_advanced.xml
**Purpose**: Comprehensive feature demonstration  
**Features**:
- Multiple positioning modes
- Color variations (white, yellow, red)
- Italic styling
- Border and shadow effects
- Different font sizes
- Multi-speaker positioning
- Title cards
- Semi-transparent overlays
- Special characters

**Use Case**: Reference for complex scenarios

---

#### sample_japanese.xml
**Purpose**: Japanese language with Ruby annotations  
**Features**:
- Hiragana, Katakana, Kanji text
- Ruby elements (furigana)
- Proper UTF-8 encoding
- Mixed script handling

**Use Case**: Asian language template

---

#### sample_multilanguage.xml
**Purpose**: International character support  
**Features**:
- 16+ languages and scripts
- Latin, Cyrillic, Greek alphabets
- Arabic and Hebrew (RTL)
- CJK (Chinese, Japanese, Korean)
- Thai, Hindi (Devanagari)
- Special Unicode symbols
- XML entity escaping

**Use Case**: Testing font support and encoding

---

#### samples/README.md
**Purpose**: Sample file documentation  
**Contents**:
- Description of each sample
- Validation instructions
- Common modifications
- Testing workflow
- Font requirements
- Troubleshooting guide

---

## Quick Start Guide

### For Developers

1. **Read First**: `CineCanvas_Technical_Spec.md` (Sections 1-5)
   - Understand XML structure
   - Learn color format
   - Learn timing format
   - Learn positioning system

2. **Implementation Guide**: `ASS_to_CineCanvas_Mapping.md`
   - Study conversion algorithms (Sections 3-5)
   - Review code examples
   - Check validation strategies

3. **Test With**: Sample files in `samples/`
   - Validate XML structure
   - Test in DCP mastering tools
   - Compare with reference implementation

### For QA/Testing

1. **Validation Checklist**:
   - XML well-formedness (use xmllint)
   - Color accuracy (check RRGGBBAA format)
   - Timing precision (frame-accurate)
   - Position safety (within safe areas)
   - Font availability

2. **Test Files**:
   - Use provided samples
   - Create custom test cases
   - Test edge cases from mapping doc

3. **Tools**:
   - xmllint (XML validation)
   - DCP-o-matic (DCP testing)
   - OpenDCP (validation)

### For Project Managers

**Capabilities Documented**:
- ✅ Complete format specification
- ✅ Conversion strategy defined
- ✅ Sample files provided
- ✅ Testing approach outlined

**Known Limitations**:
- ❌ Animations not supported
- ❌ Karaoke requires workarounds
- ❌ Drawing commands → images only
- ❌ Bold/underline/strikeout limited

**Implementation Ready**: All documentation complete for Phase 2.2

---

## Format Feature Summary

### Supported in CineCanvas

| Feature | Support Level | Notes |
|---------|--------------|-------|
| Basic text | ✅ Full | UTF-8, all languages |
| Colors | ✅ Full | RRGGBBAA format |
| Positioning | ✅ Full | Percentage-based |
| Timing | ✅ Full | Frame-accurate |
| Font styling | ✅ Partial | Italic only, no bold |
| Border | ✅ Fixed | Fixed-width border |
| Shadow | ✅ Fixed | Fixed-depth shadow |
| Fade in/out | ✅ Full | Millisecond precision |
| Multi-line | ✅ Full | Separate Text elements |
| Ruby/Furigana | ✅ Full | Unique to CineCanvas |
| Images | ✅ Full | Base64 PNG/TIFF |

### Not Supported / Limited

| Feature | Status | Workaround |
|---------|--------|------------|
| Animations | ❌ Not supported | Drop or create image sequence |
| Karaoke | ❌ Not supported | Split into timed subtitles |
| Drawing | ❌ Not supported | Convert to images |
| Bold | ❌ No attribute | Use bold font variant |
| Underline | ❌ Not supported | Drop with warning |
| Strikeout | ❌ Not supported | Drop with warning |
| 3D rotation | ❌ Not supported | Flatten to 2D |
| Variable border | ❌ Fixed only | Use fixed border |

---

## Technical Requirements

### Color Format
- **Format**: RRGGBBAA (8 hex characters)
- **Channels**: Red, Green, Blue, Alpha
- **Range**: 00-FF per channel
- **Alpha**: FF = opaque, 00 = transparent
- **Standard**: White text (FFFFFFFF) on black border (000000FF)

### Timing Format
- **Format**: HH:MM:SS:fff
- **Frame Rates**: 24, 25, 30, 48 fps
- **Precision**: Frame-accurate
- **Fade**: 10-100ms typical
- **Duration**: 2-8 seconds recommended

### Positioning Format
- **System**: Percentage-based (0-100%)
- **Vertical**: VAlign + VPosition
- **Horizontal**: HAlign + HPosition
- **Safe Areas**: 5-10% margins
- **Standard**: Bottom 10%, centered

### Font Requirements
- **Format**: TrueType (.ttf)
- **Size Limit**: ~10 MB recommended
- **CJK**: Use glyph subsetting
- **Licensing**: Must allow theatrical exhibition
- **Embedding**: External references only

---

## Validation Checklist

### XML Structure
- [ ] Well-formed XML (validate with xmllint)
- [ ] UTF-8 encoding declared
- [ ] DCSubtitle root element with Version
- [ ] Required metadata elements present
- [ ] Valid element nesting

### Metadata
- [ ] SubtitleID in urn:uuid: format
- [ ] MovieTitle non-empty
- [ ] ReelNumber valid integer
- [ ] Language valid ISO 639-2/T code

### Timing
- [ ] All times in HH:MM:SS:fff format
- [ ] TimeOut > TimeIn for all subtitles
- [ ] No overlapping subtitles
- [ ] Frame numbers valid for fps
- [ ] Fade times reasonable (10-100ms)

### Positioning
- [ ] VPosition/HPosition in 0-100 range
- [ ] Within cinema safe areas
- [ ] Consistent alignment usage
- [ ] Multi-line spacing appropriate

### Typography
- [ ] Colors in RRGGBBAA format (8 chars)
- [ ] Font IDs reference LoadFont elements
- [ ] Font sizes reasonable (28-64pt)
- [ ] Effect values valid (none/border/shadow)

### Content
- [ ] Special characters escaped (&, <, >, ", ')
- [ ] Line length reasonable (<50 chars)
- [ ] Line count reasonable (<3 lines)
- [ ] UTF-8 characters valid

---

## Reference Information

### Official Specifications
- **CineCanvas**: TI Document 2504760 Rev C
- **Interop DCP**: Digital Cinema Initiatives (DCI) Specification
- **SMPTE**: SMPTE 428-7 (newer standard)

### Tools
- **DCP Mastering**: DCP-o-matic, easyDCP, OpenDCP
- **XML Validation**: xmllint, online validators
- **Font Tools**: FontForge, pyftsubset (fontTools)
- **UUID Generation**: Boost.UUID, libuuid

### Related Formats
- **ASS**: Advanced SubStation Alpha (source format)
- **SRT**: SubRip (simple text subtitles)
- **TTML**: Timed Text Markup Language (web)
- **WebVTT**: Web Video Text Tracks
- **SMPTE-TT**: Newer DCP subtitle standard

---

## Implementation Status

### Phase 2.1: Research and Specification ✅ COMPLETE
- [x] Technical specification document
- [x] Sample XML files collection  
- [x] Color format documentation
- [x] Timing requirements documentation
- [x] Positioning system documentation
- [x] ASS format differences identified
- [x] Conversion mapping strategy

### Phase 2.2: Implementation (Next)
- [ ] Create `subtitle_format_cinecanvas.cpp/h`
- [ ] Implement color conversion
- [ ] Implement timing conversion
- [ ] Implement position conversion
- [ ] Implement XML generation
- [ ] Add export dialog integration
- [ ] Create unit tests

### Phase 2.3: Testing (Future)
- [ ] Unit test suite
- [ ] Integration tests
- [ ] Round-trip tests
- [ ] DCP mastering tool validation
- [ ] Real-world cinema testing

---

## Contributing

### Adding Documentation
When updating documentation:
1. Maintain consistent formatting
2. Update this index file
3. Add version history notes
4. Cross-reference related sections
5. Include code examples where helpful

### Reporting Issues
Documentation issues or format questions:
1. Reference specific section numbers
2. Provide example XML if applicable
3. Note which DCP tool exhibits the behavior
4. Include cinema/projector context if relevant

---

## License and Credits

**Documentation License**: CC BY-SA 4.0  
**Sample Files License**: CC0 1.0 (Public Domain)

**Based On**: 
- Texas Instruments CineCanvas Specification (2005)
- DCI Digital Cinema System Specification
- Real-world DCP mastering experience

**Authors**: Aegisub Development Team  
**Created**: November 13, 2025  
**Last Updated**: November 13, 2025

---

## Quick Reference

### Color Conversion
```cpp
// ASS (0xAABBGGRR) → CineCanvas (RRGGBBAA)
uint8_t r = ass_color & 0xFF;
uint8_t g = (ass_color >> 8) & 0xFF;
uint8_t b = (ass_color >> 16) & 0xFF;
uint8_t a = 0xFF - ((ass_color >> 24) & 0xFF);
sprintf("%02X%02X%02X%02X", r, g, b, a);
```

### Time Conversion
```cpp
// ASS (H:MM:SS.CC) → CineCanvas (HH:MM:SS:fff)
int frames = (int)(fractional_seconds * fps + 0.5);
sprintf("%02d:%02d:%02d:%03d", HH, MM, SS, frames);
```

### Position Conversion
```cpp
// ASS Alignment → CineCanvas VAlign/HAlign
VAlign = (alignment <= 3) ? "bottom" : 
         (alignment <= 6) ? "center" : "top";
HAlign = ((alignment-1) % 3 == 0) ? "left" :
         ((alignment-1) % 3 == 1) ? "center" : "right";
```

### Common Colors
- White: `FFFFFFFF`
- Black: `000000FF`
- Yellow: `FFFF00FF`
- Red: `FF0000FF`

### Standard Timing
- 24 fps: 000-023 frames
- Fade: 20-40ms
- Duration: 2-6 seconds

### Standard Position
- Bottom: VAlign="bottom" VPosition="10.0"
- Center: HAlign="center" HPosition="50.0"

---

**For questions or clarifications, refer to the detailed specification documents above.**

