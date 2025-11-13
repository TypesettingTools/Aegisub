# CineCanvas/Interop DCP Subtitle Format - Technical Specification

## Document Information

**Format Name**: CineCanvas Subtitle XML Format  
**Specification Version**: 1.1 (Rev C)  
**Standard**: DLP Cinema™ Subtitling (Interop DCP)  
**Source**: Texas Instruments Document 2504760 Rev C  
**Date**: March 31, 2005  
**Reference**: [CineCanvas Specification PDF](https://interop-docs.cinepedia.com/Reference_Documents/CineCanvas(tm)_RevC.pdf)

## Overview

CineCanvas is the XML-based subtitle format used in Interop Digital Cinema Packages (DCPs) for theatrical exhibition. It was developed by Texas Instruments for their DLP Cinema™ projection technology and has become a widely-adopted standard for cinema subtitles.

### Key Characteristics

- **File Format**: UTF-8 encoded XML
- **Color Space**: RGBA with 8-bit channels (RRGGBBAA hex format)
- **Positioning**: Percentage-based coordinate system (0-100%)
- **Timing**: Precise frame-based timecodes
- **Typography**: External TrueType font references
- **Effects**: Built-in support for borders and shadows

## 1. XML Document Structure

### 1.1 XML Declaration and Encoding

All CineCanvas XML files must begin with the standard XML prolog:

```xml
<?xml version="1.0" encoding="UTF-8"?>
```

**Requirements**:
- Encoding MUST be UTF-8
- XML version 1.0
- Well-formed XML document structure required

### 1.2 Document Type Definition (DTD)

The CineCanvas format has an official DTD (see Appendix A). Key validation rules:
- Element nesting must follow strict hierarchy
- Required attributes must be present
- Attribute values must conform to specified formats

### 1.3 Root Element: DCSubtitle

```xml
<DCSubtitle Version="1.0">
  <!-- Subtitle content -->
</DCSubtitle>
```

**Attributes**:
- `Version` (REQUIRED): Format version string
  - Current value: `"1.0"` or `"1.1"`
  - Type: CDATA string

**Child Elements** (in order):
1. `SubtitleID` (REQUIRED, exactly 1)
2. `MovieTitle` (REQUIRED, exactly 1)
3. `ReelNumber` (REQUIRED, exactly 1)
4. `Language` (REQUIRED, exactly 1)
5. `LoadFont` (OPTIONAL, 0 or more)
6. `Font` (OPTIONAL, 0 or more)
7. `Subtitle` (OPTIONAL, 0 or more)

## 2. Metadata Elements

### 2.1 SubtitleID

Unique identifier for the subtitle file using UUID format.

```xml
<SubtitleID>urn:uuid:3ae7d009-b6d8-4d5b-b622-af52ba9267df</SubtitleID>
```

**Format**:
- URN namespace: `urn:uuid:`
- UUID: RFC 4122 compliant UUID (128-bit identifier)
- Example: `urn:uuid:3ae7d009-b6d8-4d5b-b622-af52ba9267df`

**Generation**:
- Use UUID version 4 (random) or version 1 (timestamp-based)
- Must be unique for each subtitle file
- Should remain consistent across revisions of the same content

### 2.2 MovieTitle

Human-readable title of the motion picture.

```xml
<MovieTitle>JULIUS CAESAR</MovieTitle>
```

**Properties**:
- Content: Plain text string
- Encoding: UTF-8 (supports international characters)
- Recommended: Match DCP ContentTitle
- Maximum length: Not specified (keep reasonable, < 256 chars)

### 2.3 ReelNumber

Sequential reel number within the DCP.

```xml
<ReelNumber>1</ReelNumber>
```

**Format**:
- Integer value (typically 1-based)
- For multi-reel DCPs: increment for each reel
- Single reel: Always use `1`
- Range: Usually 1-20 (DCPs rarely exceed 20 reels)

### 2.4 Language

ISO 639-2/T three-letter language code.

```xml
<Language>en</Language>
```

**Format**:
- ISO 639-2/T code (terminological codes)
- Always lowercase
- Examples:
  - `en` - English
  - `fr` - French
  - `de` - German
  - `es` - Spanish
  - `it` - Italian
  - `ja` - Japanese
  - `zh` - Chinese
  - `ar` - Arabic

**Note**: Some implementations accept ISO 639-1 two-letter codes, but three-letter codes are more formally correct for DCP.

## 3. Font Management

### 3.1 LoadFont Element

References external TrueType font files for use in subtitles.

```xml
<LoadFont Id="theFontId" URI="arial.ttf"/>
```

**Attributes**:
- `Id` (REQUIRED): Unique identifier for this font
  - Type: String
  - Referenced by Font elements
  - Case-sensitive
- `URI` (REQUIRED): Path or filename of font file
  - Type: String
  - Usually relative path
  - Typically `.ttf` TrueType fonts
  - Maximum font file size: Recommended < 10 MB

**Font File Requirements**:
- Format: TrueType (.ttf)
- Embedding: Font files must be included in DCP package
- Licensing: Must have proper licensing for cinema exhibition
- Compression: Large CJK fonts may need glyph subset compression

**Best Practices**:
- Use standard, widely-supported fonts
- Test font rendering on target systems
- For Asian languages: Use compressed font subsets (see Section 11)
- Include only glyphs actually used in the subtitle file

### 3.2 Font Element

Container for typography settings and subtitle content.

```xml
<Font Id="theFontId" Size="42" Color="FFFFFFFF" Effect="border" EffectColor="FF000000" Italic="no" Script="normal">
  <Subtitle>...</Subtitle>
</Font>
```

**Attributes**:

#### Id (OPTIONAL)
- References a LoadFont Id
- If omitted: uses default system font
- Type: String (must match a LoadFont Id)

#### Size (OPTIONAL)
- Font size in points
- Type: Integer
- Range: Typically 28-64 for cinema
- Default: System default (usually 42-48)
- Cinema recommendations:
  - Small screens: 36-42 points
  - Large screens: 42-48 points
  - IMAX: 48-64 points

#### Color (OPTIONAL)
- Text color in RRGGBBAA hex format
- Type: 8-character hex string
- Format: `RRGGBBAA`
  - RR: Red channel (00-FF)
  - GG: Green channel (00-FF)
  - BB: Blue channel (00-FF)
  - AA: Alpha channel (00-FF)
    - FF = fully opaque
    - 00 = fully transparent
- Default: `FFFFFFFF` (white, opaque)
- Common values:
  - `FFFFFFFF` - White
  - `FFFF00FF` - Yellow
  - `FF0000FF` - Red
  - `00FF00FF` - Green
  - `0000FFFF` - Blue
  - `000000FF` - Black

#### Effect (OPTIONAL)
- Visual effect applied to text
- Type: String enum
- Values:
  - `none` - No effect (default)
  - `border` - Outline around characters
  - `shadow` - Drop shadow behind characters
- Default: `none`

#### EffectColor (OPTIONAL)
- Color of the border or shadow effect
- Type: 8-character hex string (RRGGBBAA)
- Only applies when Effect is `border` or `shadow`
- Default: `000000FF` (black, opaque)
- Common for borders: Black with full opacity

#### Italic (OPTIONAL)
- Italic text styling
- Type: String enum
- Values:
  - `yes` - Italic text
  - `no` - Normal text (default)
- Default: `no`

#### Script (OPTIONAL)
- Text direction and script type
- Type: String enum
- Values:
  - `normal` - Left-to-right, horizontal (default)
  - `vertical` - Top-to-bottom, vertical (for Asian languages)
- Default: `normal`

**Nesting**:
- Font elements can be nested to override parent settings
- Inner Font attributes override outer Font attributes
- Applies to all child Subtitle and Text elements

**Child Elements**:
- `Font` (recursive nesting allowed)
- `Subtitle`
- `Text` (when Font is inside Subtitle)
- `Image`

## 4. Subtitle Timing

### 4.1 Subtitle Element

Individual subtitle entry with timing information.

```xml
<Subtitle SpotNumber="1" TimeIn="00:00:05:000" TimeOut="00:00:08:000" FadeUpTime="20" FadeDownTime="20">
  <Text VAlign="bottom" VPosition="10.0">Subtitle text here</Text>
</Subtitle>
```

**Attributes**:

#### SpotNumber (REQUIRED)
- Sequential subtitle number
- Type: Integer
- Range: 1 to N (number of subtitles)
- Must be unique within the file
- Should be sequential (though not strictly required)
- Used for reference and troubleshooting

#### TimeIn (REQUIRED)
- Start time of subtitle display
- Format: `HH:MM:SS:mmm`
  - HH: Hours (00-23)
  - MM: Minutes (00-59)
  - SS: Seconds (00-59)
  - mmm: Milliseconds (000-999) OR edit units
- Example: `00:00:05:000` = 5 seconds

**Important Timing Note**:
The `:mmm` field can represent different units depending on the DCP:
- **Edit Units**: Frame number within the second (most common)
  - At 24 fps: 000-023 (24 frames)
  - At 25 fps: 000-024 (25 frames)
  - At 30 fps: 000-029 (30 frames)
- **Milliseconds**: Some implementations use true milliseconds (000-999)

**Frame-based timing examples** (24 fps):
- `00:00:05:000` = 5 seconds, frame 0
- `00:00:05:012` = 5 seconds, frame 12 (half second)
- `00:00:05:023` = 5 seconds, frame 23 (last frame)

#### TimeOut (REQUIRED)
- End time of subtitle display
- Same format as TimeIn
- Must be greater than TimeIn
- Subtitles should not overlap (validation warning if they do)

#### FadeUpTime (OPTIONAL)
- Duration of fade-in effect in milliseconds
- Type: Integer
- Range: 0-1000 ms (typically 10-100 ms)
- Default: 0 (instant appearance)
- Common value: 20 ms
- Cinema standard: 40-80 ms for smooth transitions

#### FadeDownTime (OPTIONAL)
- Duration of fade-out effect in milliseconds
- Type: Integer
- Range: 0-1000 ms (typically 10-100 ms)
- Default: 0 (instant disappearance)
- Common value: 20 ms
- Should match FadeUpTime for consistency

**Child Elements**:
- `Font` (typography override)
- `Text` (subtitle content)
- `Image` (graphical subtitle)

## 5. Text Content and Positioning

### 5.1 Text Element

Contains the actual subtitle text and positioning information.

```xml
<Text VAlign="bottom" VPosition="10.0" HAlign="center" HPosition="50.0" Direction="ltr">
  This is the subtitle text.
</Text>
```

**Attributes**:

#### VAlign (OPTIONAL)
- Vertical alignment anchor point
- Type: String enum
- Values:
  - `top` - Align to top edge
  - `center` - Align to vertical center
  - `bottom` - Align to bottom edge (most common)
- Default: `bottom`
- Cinema standard: `bottom` (subtitles at screen bottom)

#### VPosition (OPTIONAL)
- Vertical position as percentage from edge
- Type: Float (decimal number)
- Range: 0.0 to 100.0
- Units: Percentage of screen height
- Reference point: Determined by VAlign
  - `VAlign="bottom"`: Percentage from bottom edge
  - `VAlign="top"`: Percentage from top edge
  - `VAlign="center"`: Percentage from center (±50)
- Default: Depends on VAlign
- Common value: `10.0` (10% from bottom)

**VPosition Examples**:
```xml
<!-- 10% from bottom (standard cinema position) -->
<Text VAlign="bottom" VPosition="10.0">Standard subtitle</Text>

<!-- 5% from top (forced narrative) -->
<Text VAlign="top" VPosition="5.0">Title card</Text>

<!-- Centered vertically -->
<Text VAlign="center" VPosition="50.0">Centered text</Text>
```

#### HAlign (OPTIONAL)
- Horizontal alignment anchor point
- Type: String enum
- Values:
  - `left` - Align to left edge
  - `center` - Align to horizontal center (most common)
  - `right` - Align to right edge
- Default: `center`
- Cinema standard: `center` (centered subtitles)

#### HPosition (OPTIONAL)
- Horizontal position as percentage from edge
- Type: Float (decimal number)
- Range: 0.0 to 100.0
- Units: Percentage of screen width
- Reference point: Determined by HAlign
  - `HAlign="left"`: Percentage from left edge
  - `HAlign="right"`: Percentage from right edge
  - `HAlign="center"`: Percentage from center (typically 50.0)
- Default: 50.0 (center)

#### Direction (OPTIONAL)
- Text direction for bidirectional text
- Type: String enum
- Values:
  - `ltr` - Left-to-right (default)
  - `rtl` - Right-to-left (Arabic, Hebrew)
- Default: `ltr`

**Text Content**:
- Plain text string
- UTF-8 encoding supports all Unicode characters
- Special characters: Use XML entities (see Section 5.3)
- Line breaks: Multiple Text elements (not `\n` or `<br/>`)
- Maximum length: Recommended 40-50 characters per line

### 5.2 Multi-line Subtitles

Multiple lines are created using separate Text elements:

```xml
<Subtitle SpotNumber="1" TimeIn="00:00:05:000" TimeOut="00:00:08:000">
  <Text VAlign="bottom" VPosition="15.0">This is the first line</Text>
  <Text VAlign="bottom" VPosition="10.0">This is the second line</Text>
</Subtitle>
```

**Best Practices**:
- First line has higher VPosition (further from edge)
- Spacing: Typically 5% between lines
- Maximum 2-3 lines per subtitle
- Avoid overlapping with subsequent subtitles

### 5.3 Special Characters and XML Entities

XML requires escaping of certain characters:

| Character | Entity | Example |
|-----------|--------|---------|
| `<` | `&lt;` | `5 &lt; 10` |
| `>` | `&gt;` | `10 &gt; 5` |
| `&` | `&amp;` | `Smith &amp; Jones` |
| `"` | `&quot;` | `He said &quot;Hello&quot;` |
| `'` | `&apos;` | `It&apos;s working` |

**Unicode Characters**:
- Use UTF-8 encoding directly for most characters
- Example: `你好` (Chinese), `مرحبا` (Arabic), `Привет` (Russian)
- Alternative: Use numeric character references: `&#x4F60;&#x597D;`

## 6. Advanced Typography Features

### 6.1 Ruby Characters (Furigana)

Ruby annotations provide pronunciation guides, commonly used in Japanese subtitles.

```xml
<Text>
  <Ruby>
    <Rb>漢字</Rb>
    <Rt Size="50" Position="before" Offset="0" Spacing="0">かんじ</Rt>
  </Ruby>
</Text>
```

**Elements**:
- `Ruby`: Container for ruby annotation
- `Rb`: Base text (main characters)
- `Rt`: Ruby text (pronunciation guide)

**Rt Attributes**:
- `Size` (OPTIONAL): Ruby text size as percentage of base text
  - Type: Integer (percentage)
  - Range: 30-70
  - Default: 50 (half size)
- `Position` (OPTIONAL): Placement relative to base
  - Values: `before` (above/right) or `after` (below/left)
  - Default: `before`
- `Offset` (OPTIONAL): Vertical offset adjustment
  - Type: Integer (pixels or percentage)
  - Default: 0
- `Spacing` (OPTIONAL): Letter spacing adjustment
  - Type: Integer
  - Default: 0

### 6.2 Space Element

Inserts precise horizontal spacing between characters.

```xml
<Text>
  Word<Space Size="200"/>Word
</Text>
```

**Attributes**:
- `Size` (OPTIONAL): Space width as percentage of normal space
  - Type: Integer (percentage)
  - Range: 0-1000
  - Default: 100 (normal space)
  - Examples:
    - `50` - Half-width space
    - `100` - Normal space
    - `200` - Double-width space
    - `0` - No space (tight kerning)

**Use Cases**:
- Fine-tuning character spacing
- Creating visual separators
- Adjusting kerning for specific fonts

### 6.3 HGroup Element (Horizontal Grouping)

Groups multiple characters to be treated as a single unit, useful for vertical text.

```xml
<Text Direction="vertical">
  <HGroup>ABC</HGroup>
</Text>
```

**Purpose**:
- Keeps certain characters horizontal in vertical text
- Useful for numbers, acronyms, or mixed script
- Common in Japanese vertical subtitles

**Content**:
- Plain text (multiple characters)
- Treated as single glyph in layout

### 6.4 Rotate Element

Rotates individual characters by specified angle.

```xml
<Text>
  Normal <Rotate>90</Rotate> Rotated
</Text>
```

**Content**:
- Rotation angle in degrees
- Positive: Clockwise rotation
- Negative: Counter-clockwise rotation
- Common values: 90, 180, 270

**Use Cases**:
- Special typography effects
- Mixed orientation text
- Artistic subtitle designs

## 7. Image-based Subtitles

### 7.1 Image Element

Embeds bitmap graphics as subtitles (for graphics or unsupported scripts).

```xml
<Subtitle SpotNumber="1" TimeIn="00:00:05:000" TimeOut="00:00:08:000">
  <Image VAlign="bottom" VPosition="10.0" HAlign="center">
    iVBORw0KGgoAAAANSUhEUgAAAAUA... (base64 encoded PNG)
  </Image>
</Subtitle>
```

**Attributes**:
- Same positioning attributes as Text element:
  - `VAlign`, `VPosition`
  - `HAlign`, `HPosition`

**Content**:
- Base64-encoded image data
- Supported formats:
  - PNG (recommended)
  - TIFF
  - BMP (less common)
- Alpha channel supported for transparency

**Image Requirements**:
- Resolution: Match DCP resolution
  - 2K DCP: Max 2048x1080
  - 4K DCP: Max 4096x2160
- Color space: RGB or RGBA
- Compression: PNG with transparency recommended
- File size: Keep reasonable (< 1 MB per image)

**When to Use Image Subtitles**:
- Complex typography not supported by fonts
- Special graphical effects
- Languages with rendering issues
- Karaoke-style effects
- Sign translations with stylized text

**Base64 Encoding**:
```xml
<Image VAlign="bottom" VPosition="10.0">
  <!-- Base64 data, typically split across lines for readability -->
  iVBORw0KGgoAAAANSUhEUgAAAAUA
  AAAFCAYAAACNbyblAAAAHElEQVQI
  12P4//8/w38GIAXDIBKE0DHxgljN
  BAAOw9b1BAiSMAAAAABJRU5ErkJg
  ==
</Image>
```

## 8. Color System Deep Dive

### 8.1 Color Format: RRGGBBAA

CineCanvas uses 8-character hexadecimal color codes with alpha channel.

**Format**: `RRGGBBAA`
- Each component: 2 hex digits (00-FF)
- Range per channel: 0-255 (decimal)

**Channel Breakdown**:
1. **RR** - Red channel (00-FF)
2. **GG** - Green channel (00-FF)
3. **BB** - Blue channel (00-FF)
4. **AA** - Alpha channel (00-FF)
   - FF = 100% opaque
   - 80 = 50% transparent
   - 00 = 100% transparent

### 8.2 Common Cinema Colors

| Color | RGBA Hex | Description |
|-------|----------|-------------|
| White | `FFFFFFFF` | Standard subtitle color |
| Black | `000000FF` | Border/shadow color |
| Yellow | `FFFF00FF` | Alternate subtitle color |
| Red | `FF0000FF` | Emphasis/warnings |
| Green | `00FF00FF` | Special indicators |
| Blue | `0000FFFF` | Cool-toned subtitles |
| Gray | `808080FF` | Muted text |
| Semi-transparent White | `FFFFFF80` | Overlay text |

### 8.3 Color Space Considerations

**CineCanvas Color Space**:
- Format uses RGB color model
- DCPs are mastered in XYZ color space
- Conversion RGB → XYZ happens during DCP mastering

**Recommendations**:
- Use high-contrast colors (white on black border is standard)
- Avoid subtle color differences (may be lost in projection)
- Test on cinema projectors when possible
- Standard practice: White text (`FFFFFFFF`) with black border (`000000FF`)

### 8.4 Contrast and Readability

**Cinema Standards**:
- Minimum contrast ratio: 3:1 (WCAG AA)
- Recommended: 7:1 or higher (WCAG AAA)
- Border/shadow essential for readability

**Best Practices**:
```xml
<!-- Optimal readability: white text, black border -->
<Font Color="FFFFFFFF" Effect="border" EffectColor="000000FF">
  <Subtitle>...</Subtitle>
</Font>

<!-- Alternative: yellow text, black border -->
<Font Color="FFFF00FF" Effect="border" EffectColor="000000FF">
  <Subtitle>...</Subtitle>
</Font>
```

## 9. Timing System Detailed

### 9.1 Frame Rates and Edit Units

**Common DCP Frame Rates**:
- **24 fps** (23.976): Most common for theatrical cinema
- **25 fps**: European PAL standard
- **30 fps** (29.97): Less common, some NTSC content
- **48 fps**: High frame rate (HFR) cinema

**Edit Units**:
- 1 edit unit = 1 frame
- Edit units per second (EditRate) = fps
- Timecode format: `HH:MM:SS:ff`
  - ff = frame number within second (0 to fps-1)

### 9.2 Frame-accurate Timing

**24 fps Example**:
```xml
<!-- Start: 5 seconds, frame 0 -->
<!-- End: 8 seconds, frame 12 (8.5 seconds) -->
<Subtitle TimeIn="00:00:05:000" TimeOut="00:00:08:012">
```

**Calculating Frame Numbers**:
- Frame number = (fractional seconds) × fps
- Example at 24 fps:
  - 0.5 seconds = frame 12
  - 0.25 seconds = frame 6
  - 0.75 seconds = frame 18

### 9.3 Timing Best Practices

**Minimum Duration**:
- 1 second (24 frames at 24fps) absolute minimum
- 2 seconds recommended minimum for readability
- Reading time: ~0.3 seconds per word

**Maximum Duration**:
- No hard limit
- Typically 6-8 seconds maximum
- Avoid subtitles spanning scene changes

**Gaps Between Subtitles**:
- Minimum: 2 frames (for fade transitions)
- Recommended: 4-6 frames
- Allows clean transitions

**Overlapping Subtitles**:
- Generally not allowed
- Will trigger validation warnings
- Exception: Multi-speaker dialogue with separate positions

## 10. Positioning System

### 10.1 Percentage-based Coordinates

CineCanvas uses percentage-based positioning (0-100%) for resolution independence.

**Screen Coordinate System**:
```
(0,0)                      (100,0)
  ┌─────────────────────────┐
  │                         │
  │      (50,50)            │ Screen
  │                         │
  └─────────────────────────┘
(0,100)                  (100,100)
```

**Origin Points**:
- (0, 0) = Top-left corner
- (100, 100) = Bottom-right corner
- (50, 50) = Screen center

### 10.2 Vertical Positioning

**VAlign + VPosition Combinations**:

```xml
<!-- Bottom-aligned: 10% from bottom -->
<Text VAlign="bottom" VPosition="10.0">Standard position</Text>

<!-- Top-aligned: 5% from top -->
<Text VAlign="top" VPosition="5.0">Top position</Text>

<!-- Center-aligned: at vertical center -->
<Text VAlign="center" VPosition="50.0">Centered</Text>
```

**Common Cinema Positions**:
- Standard subtitle: `VAlign="bottom" VPosition="10.0"`
- Dual subtitles (bilingual):
  - Language 1: `VPosition="15.0"`
  - Language 2: `VPosition="10.0"`
- Forced narratives: `VAlign="top" VPosition="5.0"`

### 10.3 Horizontal Positioning

**HAlign + HPosition Combinations**:

```xml
<!-- Center-aligned (standard) -->
<Text HAlign="center" HPosition="50.0">Centered subtitle</Text>

<!-- Left-aligned: 10% from left -->
<Text HAlign="left" HPosition="10.0">Left position</Text>

<!-- Right-aligned: 10% from right -->
<Text HAlign="right" HPosition="10.0">Right position</Text>
```

**Multi-speaker Positioning**:
```xml
<!-- Speaker on left side of screen -->
<Text HAlign="left" HPosition="20.0" VAlign="bottom" VPosition="10.0">
  Left speaker dialogue
</Text>

<!-- Speaker on right side of screen -->
<Text HAlign="right" HPosition="20.0" VAlign="bottom" VPosition="10.0">
  Right speaker dialogue
</Text>
```

### 10.4 Safe Areas

Cinema screens have "safe areas" for subtitle placement:

**Recommended Safe Margins**:
- Horizontal: 5-10% from each edge
- Vertical (bottom): 5-12% from bottom edge
- Vertical (top): 5-10% from top edge

**Rationale**:
- Projector mask variations
- Screen curvature
- Aspect ratio differences
- Theater-specific cropping

## 11. Font Compression and Optimization

### 11.1 Font Size Limitations

**Maximum Font File Size**: ~10 MB recommended
- Some systems have hard limits (2-15 MB)
- Large CJK fonts often exceed limits

**Problem Fonts**:
- Chinese fonts: 20,000+ glyphs, often 8-15 MB
- Japanese fonts: 10,000+ glyphs
- Korean fonts: 11,000+ glyphs

### 11.2 Font Subset Compression

**Solution**: Glyph subsetting
- Analyze subtitle XML
- Extract only used characters
- Generate subset font with only needed glyphs

**Compression Results**:
- Original Chinese font: 8.5 MB
- Subset font (90-minute movie): 274 KB
- Compression ratio: ~97% size reduction

**Tools**:
- Texas Instruments Font Compressor
- FontForge with subset scripts
- pyftsubset (Python fontTools)

**Process**:
1. Parse complete XML subtitle file
2. Build character set of all used glyphs
3. Extract glyphs from original font
4. Remove unnecessary tables (bitmap, PCL, etc.)
5. Generate subset TrueType font
6. Update LoadFont URI to reference subset

**Caveats**:
- Subset font only works for that specific subtitle file
- If subtitles change, must regenerate subset
- Keep original font for future edits

### 11.3 Font Tables to Remove

**Unnecessary Tables** (for cinema):
- EBDT/EBLC: Embedded bitmap data
- PCLT: PCL printer data
- hdmx: Horizontal device metrics
- LTSH: Linear threshold data

**Required Tables** (keep):
- cmap: Character to glyph mapping
- glyf: Glyph outlines
- head: Font header
- hhea: Horizontal metrics header
- hmtx: Horizontal metrics
- loca: Glyph location index
- maxp: Maximum profile
- name: Naming table
- post: PostScript information

## 12. Validation and Quality Control

### 12.1 XML Validation

**Schema Validation**:
- Validate against official DTD
- Check well-formed XML structure
- Verify required elements present
- Confirm attribute value formats

**Common XML Errors**:
- Missing required attributes
- Invalid character encoding
- Improper element nesting
- Unclosed tags
- Invalid XML entities

### 12.2 Content Validation

**Timing Checks**:
- TimeOut > TimeIn for all subtitles
- No overlapping subtitles
- Reasonable fade durations
- Frame numbers valid for EditRate

**Position Checks**:
- VPosition/HPosition within 0-100 range
- Consistent positioning throughout
- No subtitles off-screen

**Typography Checks**:
- Referenced fonts exist in LoadFont
- Color values are 8-character hex
- Effect values are valid enums
- Font sizes are reasonable

**Content Checks**:
- Character count per line (< 50 chars recommended)
- Number of lines per subtitle (< 3 recommended)
- Reading speed (words per minute)
- Special character encoding

### 12.3 Cinema-specific Validation

**DCI Compliance**:
- Subtitle count per reel (typically < 500)
- Font licensing for exhibition
- Color contrast standards
- Safe area compliance

**Mastering Checks**:
- Test in DCP mastering software (DCP-o-matic, easyDCP)
- Validate with DCP validation tools
- Test playback on cinema servers
- Verify timing with picture

## 13. Differences from ASS Format

### 13.1 Format Philosophy

| Aspect | ASS Format | CineCanvas Format |
|--------|-----------|-------------------|
| Purpose | General subtitle editing | Cinema exhibition |
| Positioning | Pixel-based coordinates | Percentage-based |
| Colors | BGR hex (AABBGGRR) | RGBA hex (RRGGBBAA) |
| Timing | Centiseconds (0:00:00.00) | Frames (00:00:00:000) |
| Effects | Complex override tags | Limited built-in effects |
| Metadata | Minimal | DCP-specific (UUID, reel, etc.) |
| Font handling | Embedded or external | External references only |

### 13.2 Structural Differences

**ASS Structure**:
```ass
[Script Info]
Title: Movie
ScriptType: v4.00+

[V4+ Styles]
Style: Default,Arial,42,&H00FFFFFF,&H000000FF,&H00000000,&H00000000,...

[Events]
Dialogue: 0,0:00:05.00,0:00:08.00,Default,,0,0,0,,Subtitle text
```

**CineCanvas Structure**:
```xml
<DCSubtitle Version="1.0">
  <SubtitleID>urn:uuid:...</SubtitleID>
  <Font Color="FFFFFFFF" Size="42">
    <Subtitle TimeIn="00:00:05:000" TimeOut="00:00:08:000">
      <Text>Subtitle text</Text>
    </Subtitle>
  </Font>
</DCSubtitle>
```

### 13.3 Feature Comparison

| Feature | ASS Support | CineCanvas Support | Notes |
|---------|-------------|-------------------|-------|
| Basic text | ✓ Full | ✓ Full | - |
| Positioning | ✓ Pixel + % | ✓ Percentage only | CineCanvas resolution-independent |
| Colors | ✓ RGB + Alpha | ✓ RGBA | Different hex format |
| Font styles | ✓ Full CSS-like | ✓ Basic (size, italic) | ASS more flexible |
| Bold | ✓ Yes | ✗ No | Use different font weight |
| Underline | ✓ Yes | ✗ No | Not supported |
| Strikeout | ✓ Yes | ✗ No | Not supported |
| Border/Shadow | ✓ Adjustable size | ✓ Fixed effect | ASS more control |
| Animations | ✓ Extensive | ✗ No | Only fade in/out |
| Rotation | ✓ 3D rotation | ✓ 2D only (Rotate element) | Limited in CineCanvas |
| Karaoke | ✓ Built-in | ✗ No | Use image subtitles |
| Ruby/Furigana | ✗ No | ✓ Yes (Ruby element) | CineCanvas advantage |
| Multi-language | ✓ Via styles | ✓ Via multiple files | Separate XML per language |
| Embedded fonts | ✓ Yes | ✗ No | External references only |
| Drawing commands | ✓ Yes | ✗ No | Use Image element |
| Override tags | ✓ Extensive | ✗ No | Style via Font element |

### 13.4 Conversion Challenges

**ASS → CineCanvas Conversion Issues**:

1. **Override Tags**: ASS supports inline style overrides (`{\b1}bold{\b0}`)
   - CineCanvas: Use nested Font elements or split into multiple subtitles

2. **Animations**: ASS has complex animation tags (`\move`, `\fad`, `\t`)
   - CineCanvas: Only FadeUpTime/FadeDownTime supported

3. **Drawing Commands**: ASS vector drawing (`\p1`, `\p0`)
   - CineCanvas: Convert to PNG images

4. **Karaoke Effects**: ASS karaoke timing (`\k`, `\ko`, `\kf`)
   - CineCanvas: Create image-based subtitles or multiple timed subtitles

5. **Pixel Positioning**: ASS `\pos(x,y)` uses absolute pixels
   - CineCanvas: Convert to percentage-based VPosition/HPosition

6. **Color Format**: ASS uses `&HAABBGGRR&` (BGR with alpha prefix)
   - CineCanvas: Convert to `RRGGBBAA`

7. **Margins**: ASS has MarginL, MarginR, MarginV
   - CineCanvas: Calculate equivalent percentage positions

**CineCanvas → ASS Conversion Issues**:

1. **UUID and Metadata**: CineCanvas SubtitleID, ReelNumber not in ASS
   - Store in [Script Info] comments

2. **Ruby Characters**: CineCanvas Ruby element
   - ASS: Simulate with positioned text or lose annotations

3. **Frame-based Timing**: CineCanvas uses frame numbers
   - Convert to centiseconds (requires fps information)

4. **Image Subtitles**: CineCanvas base64-encoded images
   - ASS: Extract and reference as external files, or use drawing commands

### 13.5 Mapping Table

**Color Conversion**:
```
ASS: &H00FFFFFF& (white, BGR format)
  → CineCanvas: FFFFFFFF (white, RGBA format)

ASS: &HFF000000& (black, BGR with alpha)
  → CineCanvas: 000000FF (black, RGBA format)
```

**Timing Conversion** (24 fps example):
```
ASS: 0:00:05.50 (5.5 seconds)
  → CineCanvas: 00:00:05:012 (5 seconds + 12 frames)

Calculation: 0.50 seconds × 24 fps = 12 frames
```

**Position Conversion** (1920x1080 screen):
```
ASS: \pos(960, 972) (centered, 10% from bottom)
  → CineCanvas: HAlign="center" HPosition="50.0" VAlign="bottom" VPosition="10.0"

Calculation:
  X: 960 / 1920 = 50%
  Y: (1080 - 972) / 1080 = 10% from bottom
```

## 14. Complete Example

### 14.1 Simple Subtitle File

```xml
<?xml version="1.0" encoding="UTF-8"?>
<DCSubtitle Version="1.0">
  <SubtitleID>urn:uuid:3ae7d009-b6d8-4d5b-b622-af52ba9267df</SubtitleID>
  <MovieTitle>Example Movie</MovieTitle>
  <ReelNumber>1</ReelNumber>
  <Language>en</Language>
  <LoadFont Id="Arial" URI="arial.ttf"/>
  
  <Font Id="Arial" Size="42" Color="FFFFFFFF" Effect="border" EffectColor="000000FF">
    <!-- Basic single-line subtitle -->
    <Subtitle SpotNumber="1" TimeIn="00:00:05:000" TimeOut="00:00:08:000" 
              FadeUpTime="20" FadeDownTime="20">
      <Text VAlign="bottom" VPosition="10.0">Hello, world!</Text>
    </Subtitle>
    
    <!-- Two-line subtitle -->
    <Subtitle SpotNumber="2" TimeIn="00:00:10:000" TimeOut="00:00:14:000" 
              FadeUpTime="20" FadeDownTime="20">
      <Text VAlign="bottom" VPosition="15.0">This is the first line.</Text>
      <Text VAlign="bottom" VPosition="10.0">This is the second line.</Text>
    </Subtitle>
    
    <!-- Italic subtitle -->
    <Subtitle SpotNumber="3" TimeIn="00:00:16:000" TimeOut="00:00:19:000" 
              FadeUpTime="20" FadeDownTime="20">
      <Font Italic="yes">
        <Text VAlign="bottom" VPosition="10.0">Whispered dialogue...</Text>
      </Font>
    </Subtitle>
    
    <!-- Yellow subtitle (translation) -->
    <Subtitle SpotNumber="4" TimeIn="00:00:21:000" TimeOut="00:00:25:000" 
              FadeUpTime="20" FadeDownTime="20">
      <Font Color="FFFF00FF">
        <Text VAlign="bottom" VPosition="10.0">Translated text appears in yellow</Text>
      </Font>
    </Subtitle>
    
    <!-- Top-positioned subtitle (sign translation) -->
    <Subtitle SpotNumber="5" TimeIn="00:00:27:000" TimeOut="00:00:31:000" 
              FadeUpTime="20" FadeDownTime="20">
      <Text VAlign="top" VPosition="5.0">POLICE STATION</Text>
    </Subtitle>
  </Font>
</DCSubtitle>
```

### 14.2 Advanced Example with Multiple Features

```xml
<?xml version="1.0" encoding="UTF-8"?>
<DCSubtitle Version="1.0">
  <SubtitleID>urn:uuid:f81d4fae-7dec-11d0-a765-00a0c91e6bf6</SubtitleID>
  <MovieTitle>Advanced Subtitle Demo</MovieTitle>
  <ReelNumber>1</ReelNumber>
  <Language>ja</Language>
  <LoadFont Id="Japanese" URI="NotoSansJP-Regular.ttf"/>
  
  <Font Id="Japanese" Size="48" Color="FFFFFFFF" Effect="border" EffectColor="000000FF">
    <!-- Japanese with Ruby (furigana) -->
    <Subtitle SpotNumber="1" TimeIn="00:00:05:000" TimeOut="00:00:09:000">
      <Text VAlign="bottom" VPosition="10.0">
        <Ruby>
          <Rb>映画</Rb>
          <Rt Size="50" Position="before">えいが</Rt>
        </Ruby>
        が
        <Ruby>
          <Rb>始</Rb>
          <Rt Size="50" Position="before">はじ</Rt>
        </Ruby>
        まります
      </Text>
    </Subtitle>
    
    <!-- Image-based subtitle -->
    <Subtitle SpotNumber="2" TimeIn="00:00:12:000" TimeOut="00:00:16:000">
      <Image VAlign="bottom" VPosition="10.0" HAlign="center">
        iVBORw0KGgoAAAANSUhEUgAAAAUAAAAFCAYAAACNbyblAAAAHElEQVQI12P4
        //8/w38GIAXDIBKE0DHxgljNBAAO9TXL0Y4OHwAAAABJRU5ErkJggg==
      </Image>
    </Subtitle>
    
    <!-- Dual-speaker positioning -->
    <Subtitle SpotNumber="3" TimeIn="00:00:20:000" TimeOut="00:00:24:000">
      <Text VAlign="bottom" VPosition="10.0" HAlign="left" HPosition="20.0">
        Left speaker: Hello!
      </Text>
      <Text VAlign="bottom" VPosition="10.0" HAlign="right" HPosition="20.0">
        Right speaker: Hi there!
      </Text>
    </Subtitle>
    
    <!-- Custom spacing -->
    <Subtitle SpotNumber="4" TimeIn="00:00:28:000" TimeOut="00:00:32:000">
      <Text VAlign="bottom" VPosition="10.0">
        W I D E<Space Size="200"/>S P A C I N G
      </Text>
    </Subtitle>
  </Font>
</DCSubtitle>
```

## 15. Implementation Notes

### 15.1 XML Library Recommendations

**C++ (for Aegisub)**:
- **wxXML** (already used in Aegisub)
  - Part of wxWidgets
  - DOM-based parsing
  - Good for small-medium files
- **TinyXML-2**
  - Lightweight
  - DOM-based
  - Simple API
- **RapidXML**
  - Very fast
  - Header-only
  - Good for parsing

### 15.2 UUID Generation

**Libraries**:
- **Boost.UUID** (already in Aegisub dependencies)
  ```cpp
  #include <boost/uuid/uuid.hpp>
  #include <boost/uuid/uuid_generators.hpp>
  #include <boost/uuid/uuid_io.hpp>
  
  boost::uuids::uuid uuid = boost::uuids::random_generator()();
  std::string uuid_str = "urn:uuid:" + boost::uuids::to_string(uuid);
  ```

### 15.3 Color Conversion Algorithm

**ASS → CineCanvas**:
```cpp
// ASS format: &HAABBGGRR& (4 bytes, alpha + BGR)
// CineCanvas: RRGGBBAA (8 hex chars, RGBA)

std::string ConvertColorToRGBA(uint32_t ass_color, uint8_t alpha = 255) {
    uint8_t b = (ass_color >> 16) & 0xFF;
    uint8_t g = (ass_color >> 8) & 0xFF;
    uint8_t r = ass_color & 0xFF;
    
    char buffer[9];
    snprintf(buffer, sizeof(buffer), "%02X%02X%02X%02X", r, g, b, alpha);
    return std::string(buffer);
}

// Example:
// ASS: &H00FFFFFF& → 0x00FFFFFF → R=FF, G=FF, B=FF, A=FF → "FFFFFFFF"
// ASS: &H00FF0000& → 0x00FF0000 → R=00, G=00, B=FF, A=FF → "0000FFFF" (blue)
```

### 15.4 Timing Conversion Algorithm

**ASS → CineCanvas** (frame-based):
```cpp
std::string ConvertTimingToFrames(int milliseconds, double fps) {
    int hours = milliseconds / 3600000;
    milliseconds %= 3600000;
    int minutes = milliseconds / 60000;
    milliseconds %= 60000;
    int seconds = milliseconds / 1000;
    milliseconds %= 1000;
    
    // Convert remaining milliseconds to frame number
    int frames = (int)((milliseconds / 1000.0) * fps);
    
    char buffer[13];
    snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d:%03d", 
             hours, minutes, seconds, frames);
    return std::string(buffer);
}

// Example at 24 fps:
// 5500 ms → 00:00:05:012 (5 seconds + 12 frames)
```

### 15.5 Position Conversion Algorithm

**ASS → CineCanvas** (pixel to percentage):
```cpp
struct Position {
    double hposition;
    double vposition;
    std::string halign;
    std::string valign;
};

Position ConvertPosition(int x, int y, int screen_width, int screen_height, 
                        int alignment) {
    Position pos;
    
    // Horizontal alignment (1,4,7=left; 2,5,8=center; 3,6,9=right)
    if (alignment % 3 == 1) {
        pos.halign = "left";
        pos.hposition = (double)x / screen_width * 100.0;
    } else if (alignment % 3 == 2) {
        pos.halign = "center";
        pos.hposition = 50.0; // Typically centered
    } else {
        pos.halign = "right";
        pos.hposition = (double)(screen_width - x) / screen_width * 100.0;
    }
    
    // Vertical alignment (1,2,3=bottom; 4,5,6=middle; 7,8,9=top)
    if (alignment <= 3) {
        pos.valign = "bottom";
        pos.vposition = (double)(screen_height - y) / screen_height * 100.0;
    } else if (alignment <= 6) {
        pos.valign = "center";
        pos.vposition = 50.0;
    } else {
        pos.valign = "top";
        pos.vposition = (double)y / screen_height * 100.0;
    }
    
    return pos;
}
```

## 16. Testing and Validation Tools

### 16.1 DCP Mastering Software

**Recommended Tools**:
- **DCP-o-matic**: Open-source DCP creation
- **OpenDCP**: Open-source DCP tools
- **easyDCP**: Commercial DCP mastering
- **Clipster**: Professional mastering system

### 16.2 Validation Tools

**XML Validators**:
- `xmllint` (command-line)
  ```bash
  xmllint --noout --dtdvalid cinecanvas.dtd subtitle.xml
  ```
- Online XML validators
- Custom validation scripts

**DCP Validators**:
- Clipster validation module
- easyDCP validation
- Custom Python/Ruby scripts

### 16.3 Sample Files

**Test Suite Should Include**:
1. Basic single-line subtitles
2. Multi-line subtitles
3. Various timing patterns
4. Different colors and effects
5. Multiple fonts
6. Image-based subtitles
7. Ruby/furigana text
8. Edge cases (very short, very long)
9. Special characters
10. Multi-language examples

## 17. Best Practices Summary

### 17.1 Content Best Practices

1. **Timing**:
   - Minimum 2 seconds per subtitle
   - Maximum 6-8 seconds
   - 4-6 frame gap between subtitles
   - Use consistent fade times (20-40ms)

2. **Text**:
   - Maximum 40-50 characters per line
   - Maximum 2-3 lines per subtitle
   - Reading speed: 15-20 characters per second

3. **Positioning**:
   - Standard: Bottom-centered, 10% from edge
   - Stay within safe areas (5-10% margins)
   - Use consistent positioning

4. **Typography**:
   - Font size: 42-48 points for most screens
   - Always use border or shadow for readability
   - Standard: White text, black border

5. **Colors**:
   - High contrast required
   - Test on actual projectors
   - Standard: `FFFFFFFF` (white) on `000000FF` (black border)

### 17.2 Technical Best Practices

1. **File Structure**:
   - Validate against DTD
   - Use UTF-8 encoding
   - Generate unique UUIDs
   - Include proper metadata

2. **Fonts**:
   - Keep font files under 10 MB
   - Use subset fonts for CJK languages
   - Test font rendering
   - Verify font licensing

3. **Performance**:
   - Limit subtitle count per reel (< 500)
   - Optimize image sizes
   - Use efficient XML structure

4. **Quality Control**:
   - Validate XML structure
   - Check timing accuracy
   - Verify colors and readability
   - Test in DCP mastering software
   - Preview on cinema equipment

## 18. Troubleshooting

### 18.1 Common Issues

| Issue | Cause | Solution |
|-------|-------|----------|
| Invalid XML | Malformed structure | Validate with xmllint |
| Font not found | Missing LoadFont | Verify URI path correct |
| Wrong colors | Color format error | Use RRGGBBAA format |
| Timing off | Frame rate mismatch | Verify EditRate setting |
| Text off-screen | Position > 100% | Check VPosition/HPosition |
| Overlapping subtitles | TimeOut > next TimeIn | Adjust timing |
| Font too large | CJK font not subset | Use font compression |
| No fade effect | FadeTime = 0 | Set FadeUpTime/FadeDownTime |

### 18.2 Debugging Tips

1. **XML Issues**: Use xmllint with verbose output
2. **Timing Issues**: Convert to milliseconds and verify
3. **Position Issues**: Calculate percentages manually
4. **Color Issues**: Convert hex to RGB decimal to verify
5. **Font Issues**: Check file exists and is valid TrueType

## 19. References and Resources

### 19.1 Official Documentation

- **CineCanvas Specification**: TI Document 2504760 Rev C
- **Interop DCP Standard**: Digital Cinema Initiatives (DCI)
- **SMPTE Standards**: SMPTE 428-7 (subtitle format)

### 19.2 Tools

- **Font Compression**: Texas Instruments Font Compressor
- **DCP Mastering**: DCP-o-matic, easyDCP, OpenDCP
- **XML Tools**: xmllint, TinyXML, RapidXML
- **UUID Generators**: Boost.UUID, libuuid

### 19.3 Related Formats

- **SMPTE Timed Text**: Newer DCP subtitle standard
- **ASS/SSA**: Advanced SubStation Alpha format
- **SRT**: SubRip text format
- **TTML**: Timed Text Markup Language
- **WebVTT**: Web Video Text Tracks

## Appendix A: DTD Reference

```dtd
<!ELEMENT DCSubtitle (SubtitleID, MovieTitle, ReelNumber, Language, LoadFont*,
Font*, Subtitle*)>
    <!ATTLIST DCSubtitle Version CDATA #REQUIRED>
<!ELEMENT SubtitleID (#PCDATA)>
<!ELEMENT MovieTitle (#PCDATA)>
<!ELEMENT ReelNumber (#PCDATA)>
<!ELEMENT Language (#PCDATA)>
<!ELEMENT LoadFont EMPTY>
    <!ATTLIST LoadFont Id CDATA #REQUIRED>
    <!ATTLIST LoadFont URI CDATA #REQUIRED>
<!ELEMENT Font (#PCDATA | Font | Subtitle | Text | Image)*>
    <!ATTLIST Font Id CDATA #IMPLIED>
    <!ATTLIST Font Color CDATA #IMPLIED>
    <!ATTLIST Font Effect CDATA #IMPLIED>
    <!ATTLIST Font EffectColor CDATA #IMPLIED>
    <!ATTLIST Font Italic CDATA #IMPLIED>
    <!ATTLIST Font Script CDATA #IMPLIED>
    <!ATTLIST Font Size CDATA #IMPLIED>
<!ELEMENT Subtitle (Font | Text | Image)*>
    <!ATTLIST Subtitle SpotNumber CDATA #REQUIRED>
    <!ATTLIST Subtitle TimeIn CDATA #REQUIRED>
    <!ATTLIST Subtitle TimeOut CDATA #REQUIRED>
    <!ATTLIST Subtitle FadeUpTime CDATA #IMPLIED>
    <!ATTLIST Subtitle FadeDownTime CDATA #IMPLIED>
<!ELEMENT Text (#PCDATA | Font | Ruby* | Space* | HGroup* | Rotate*)*>
    <!ATTLIST Text Direction CDATA #IMPLIED>
    <!ATTLIST Text HAlign CDATA #IMPLIED>
    <!ATTLIST Text HPosition CDATA #IMPLIED>
    <!ATTLIST Text VAlign CDATA #IMPLIED>
    <!ATTLIST Text VPosition CDATA #IMPLIED>
<!ELEMENT Ruby (Rb, Rt)>
<!ELEMENT Rb (#PCDATA)>
<!ELEMENT Rt (#PCDATA)>
    <!ATTLIST Rt Size CDATA #IMPLIED>
    <!ATTLIST Rt Position CDATA #IMPLIED>
    <!ATTLIST Rt Offset CDATA #IMPLIED>
    <!ATTLIST Rt Spacing CDATA #IMPLIED>
<!ELEMENT Space EMPTY>
    <!ATTLIST Space Size CDATA #IMPLIED>
<!ELEMENT HGroup (#PCDATA)>
<!ELEMENT Rotate (#PCDATA)>
<!ELEMENT Image (#PCDATA)>
    <!ATTLIST Image HAlign CDATA #IMPLIED>
    <!ATTLIST Image HPosition CDATA #IMPLIED>
    <!ATTLIST Image VAlign CDATA #IMPLIED>
    <!ATTLIST Image VPosition CDATA #IMPLIED>
```

## Appendix B: Quick Reference Tables

### B.1 Alignment Values

| VAlign | Meaning |
|--------|---------|
| top | Align to top edge |
| center | Align to vertical center |
| bottom | Align to bottom edge (default) |

| HAlign | Meaning |
|--------|---------|
| left | Align to left edge |
| center | Align to horizontal center (default) |
| right | Align to right edge |

### B.2 Effect Values

| Effect | Description |
|--------|-------------|
| none | No effect (default) |
| border | Outline around characters |
| shadow | Drop shadow behind characters |

### B.3 Common Frame Rates

| FPS | Description | Edit Units |
|-----|-------------|------------|
| 24 | Standard cinema | 000-023 |
| 25 | PAL standard | 000-024 |
| 30 | NTSC standard | 000-029 |
| 48 | High frame rate | 000-047 |

### B.4 Language Codes (Common)

| Code | Language |
|------|----------|
| en | English |
| es | Spanish |
| fr | French |
| de | German |
| it | Italian |
| pt | Portuguese |
| ru | Russian |
| ja | Japanese |
| zh | Chinese |
| ko | Korean |
| ar | Arabic |
| he | Hebrew |

---

**Document Version**: 1.0  
**Last Updated**: November 13, 2025  
**Author**: Aegisub Development Team  
**License**: CC BY-SA 4.0

