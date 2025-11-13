# ASS to CineCanvas Conversion Mapping

## Document Overview

This document provides a detailed mapping strategy for converting ASS (Advanced SubStation Alpha) subtitles to CineCanvas XML format for DCP (Digital Cinema Package) workflows.

**Target Audience**: Developers implementing the conversion logic  
**Related Documents**:
- `CineCanvas_Technical_Spec.md` - Complete format specification
- `XML-PLAN.md` - Implementation plan

---

## 1. Conceptual Differences

### 1.1 Format Philosophy

| Aspect | ASS | CineCanvas |
|--------|-----|------------|
| **Purpose** | Flexible subtitle editing with effects | Cinema exhibition with standardization |
| **Positioning** | Pixel-based + percentage | Percentage-only |
| **Color Model** | BGR with prefix alpha (`&HAABBGGRR&`) | RGBA hex (`RRGGBBAA`) |
| **Timing** | Centiseconds (`0:00:00.00`) | Frame-based (`00:00:00:fff`) |
| **Styling** | Rich override tags | Limited built-in effects |
| **Resolution** | Fixed to video resolution | Resolution-independent |

### 1.2 Feature Availability Matrix

| Feature | ASS | CineCanvas | Conversion Strategy |
|---------|-----|------------|---------------------|
| Basic text | ✓ | ✓ | Direct mapping |
| Position (percentage) | ✓ | ✓ | Convert to VPosition/HPosition |
| Position (pixel) | ✓ | ✗ | Calculate percentage from resolution |
| Colors | ✓ | ✓ | Convert BGR→RGBA format |
| Font size | ✓ | ✓ | Direct mapping |
| Bold | ✓ | ✗ | Use bold font variant in LoadFont |
| Italic | ✓ | ✓ | Map to Italic attribute |
| Underline | ✓ | ✗ | Drop with warning |
| Strikeout | ✓ | ✗ | Drop with warning |
| Border | ✓ | ✓ | Map to Effect="border" |
| Shadow | ✓ | ✓ | Map to Effect="shadow" |
| Fade in/out | ✓ | ✓ | Map to FadeUpTime/FadeDownTime |
| Complex animations | ✓ | ✗ | Drop with warning |
| Rotation (simple) | ✓ | ✓ | Map to Rotate element |
| Rotation (3D) | ✓ | ✗ | Flatten to 2D or drop |
| Karaoke | ✓ | ✗ | Convert to multiple timed subtitles or images |
| Drawing commands | ✓ | ✗ | Render to PNG and embed as Image |
| Ruby/Furigana | ✗ | ✓ | CineCanvas advantage |

---

## 2. File Structure Mapping

### 2.1 Script Info Section

**ASS Format**:
```ass
[Script Info]
Title: Movie Title
ScriptType: v4.00+
WrapStyle: 0
PlayResX: 1920
PlayResY: 1080
```

**CineCanvas Mapping**:
```xml
<DCSubtitle Version="1.0">
  <SubtitleID>urn:uuid:[generated]</SubtitleID>
  <MovieTitle>Movie Title</MovieTitle>
  <ReelNumber>1</ReelNumber>
  <Language>en</Language>
  <!-- ... -->
</DCSubtitle>
```

**Conversion Rules**:
| ASS Field | CineCanvas Element | Notes |
|-----------|-------------------|-------|
| `Title` | `<MovieTitle>` | Direct copy |
| `PlayResX`, `PlayResY` | Used for percentage calculation | Not stored in output |
| `ScriptType` | N/A | Not needed |
| `WrapStyle` | N/A | CineCanvas uses multi-line Text elements |

**New Fields Required**:
- `SubtitleID`: Generate new UUID for each export
- `ReelNumber`: Default to 1, or use export option
- `Language`: Derive from filename or use export option

### 2.2 Styles Section

**ASS Format**:
```ass
[V4+ Styles]
Format: Name, Fontname, Fontsize, PrimaryColour, SecondaryColour, OutlineColour, BackColour, Bold, Italic, Underline, StrikeOut, ScaleX, ScaleY, Spacing, Angle, BorderStyle, Outline, Shadow, Alignment, MarginL, MarginR, MarginV, Encoding
Style: Default,Arial,42,&H00FFFFFF,&H000000FF,&H00000000,&H00000000,0,0,0,0,100,100,0,0,1,2,0,2,10,10,10,1
```

**CineCanvas Mapping**:
```xml
<LoadFont Id="Arial" URI="arial.ttf"/>
<Font Id="Arial" Size="42" Color="FFFFFFFF" Effect="border" EffectColor="000000FF">
  <!-- Subtitles using this style -->
</Font>
```

**Field Mapping**:
| ASS Style Field | CineCanvas Attribute | Conversion |
|----------------|---------------------|------------|
| `Name` | Used to group subtitles | Group Font elements by style |
| `Fontname` | `LoadFont/@Id` and `Font/@Id` | Create LoadFont reference |
| `Fontsize` | `Font/@Size` | Direct copy |
| `PrimaryColour` | `Font/@Color` | Convert &HAABBGGRR → RRGGBBAA |
| `OutlineColour` | `Font/@EffectColor` | Convert &HAABBGGRR → RRGGBBAA |
| `Bold` | N/A | Use bold font file variant |
| `Italic` | `Font/@Italic` | Map -1/1 to "yes", 0 to "no" |
| `Underline` | N/A | **Drop with warning** |
| `StrikeOut` | N/A | **Drop with warning** |
| `BorderStyle` | Determines Effect | 1 → "border", 3 → "shadow" |
| `Outline` | N/A | CineCanvas uses fixed border |
| `Shadow` | N/A | CineCanvas uses fixed shadow |
| `Alignment` | `Text/@VAlign` and `Text/@HAlign` | See alignment mapping |
| `MarginL`, `MarginR`, `MarginV` | `Text/@VPosition`, `Text/@HPosition` | Calculate percentage |

---

## 3. Color Conversion

### 3.1 Color Format Differences

**ASS Color Format**: `&HAABBGGRR&`
- 4 bytes: Alpha, Blue, Green, Red
- Alpha prefix (00 = opaque, FF = transparent)
- BGR color order
- Hexadecimal with `&H` prefix and `&` suffix

**CineCanvas Color Format**: `RRGGBBAA`
- 8 hex characters: Red, Green, Blue, Alpha
- RGBA color order
- Alpha suffix (FF = opaque, 00 = transparent)
- **Note**: Alpha is inverted compared to ASS!

### 3.2 Conversion Algorithm

```cpp
std::string ConvertASSColorToCineCanvas(uint32_t ass_color) {
    // ASS format: &HAABBGGRR& (stored as 0xAABBGGRR)
    uint8_t b = (ass_color >> 16) & 0xFF;  // Extract blue
    uint8_t g = (ass_color >> 8) & 0xFF;   // Extract green
    uint8_t r = ass_color & 0xFF;          // Extract red
    uint8_t a_ass = (ass_color >> 24) & 0xFF;  // Extract ASS alpha
    
    // Invert alpha: ASS 00=opaque, CineCanvas FF=opaque
    uint8_t a_cinema = 0xFF - a_ass;
    
    // Format as RRGGBBAA
    char buffer[9];
    snprintf(buffer, sizeof(buffer), "%02X%02X%02X%02X", r, g, b, a_cinema);
    return std::string(buffer);
}
```

### 3.3 Color Mapping Examples

| Color | ASS Format | ASS Hex | CineCanvas | Visual |
|-------|-----------|---------|------------|--------|
| White | `&H00FFFFFF&` | `0x00FFFFFF` | `FFFFFFFF` | ⬜ |
| Black | `&H00000000&` | `0x00000000` | `000000FF` | ⬛ |
| Red | `&H000000FF&` | `0x000000FF` | `FF0000FF` | 🟥 |
| Green | `&H0000FF00&` | `0x0000FF00` | `00FF00FF` | 🟩 |
| Blue | `&H00FF0000&` | `0x00FF0000` | `0000FFFF` | 🟦 |
| Yellow | `&H0000FFFF&` | `0x0000FFFF` | `FFFF00FF` | 🟨 |
| Cyan | `&H00FFFF00&` | `0x00FFFF00` | `00FFFFFF` | 🟦 |
| Magenta | `&H00FF00FF&` | `0x00FF00FF` | `FF00FFFF` | 🟪 |
| Semi-transparent White | `&H80FFFFFF&` | `0x80FFFFFF` | `FFFFFF7F` | ⬜ (50%) |

### 3.4 Special Cases

**ASS Transparency**:
- ASS `&H00` prefix = 100% opaque
- ASS `&HFF` prefix = 100% transparent
- Intermediate values: Linear interpolation

**CineCanvas Transparency**:
- CineCanvas `FF` suffix = 100% opaque
- CineCanvas `00` suffix = 100% transparent
- **Conversion**: `cinema_alpha = 255 - ass_alpha`

---

## 4. Timing Conversion

### 4.1 Timing Format Differences

**ASS Timing**: `H:MM:SS.CC`
- H: Hours (1 digit, can overflow)
- MM: Minutes (2 digits)
- SS: Seconds (2 digits)
- CC: Centiseconds (2 digits, 1/100 second)
- Example: `0:00:05.50` = 5.5 seconds

**CineCanvas Timing**: `HH:MM:SS:fff`
- HH: Hours (2 digits, 00-23)
- MM: Minutes (2 digits, 00-59)
- SS: Seconds (2 digits, 00-59)
- fff: Frame number or milliseconds (3 digits)
- Example: `00:00:05:012` = 5 seconds + 12 frames (at 24fps)

### 4.2 Conversion Algorithm

```cpp
std::string ConvertASSTimeToCineCanvas(const std::string& ass_time, double fps) {
    // Parse ASS time: H:MM:SS.CC
    int hours, minutes;
    double seconds;
    sscanf(ass_time.c_str(), "%d:%d:%lf", &hours, &minutes, &seconds);
    
    // Split seconds into whole and fractional parts
    int whole_seconds = (int)seconds;
    double fractional = seconds - whole_seconds;
    
    // Convert fractional seconds to frame number
    int frames = (int)(fractional * fps + 0.5);  // Round to nearest
    
    // Handle frame overflow (e.g., 24 frames at 24fps → next second)
    if (frames >= fps) {
        whole_seconds++;
        frames = 0;
    }
    
    // Handle seconds overflow
    if (whole_seconds >= 60) {
        minutes += whole_seconds / 60;
        whole_seconds %= 60;
    }
    
    // Handle minutes overflow
    if (minutes >= 60) {
        hours += minutes / 60;
        minutes %= 60;
    }
    
    // Format as HH:MM:SS:fff
    char buffer[13];
    snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d:%03d", 
             hours, minutes, whole_seconds, frames);
    return std::string(buffer);
}
```

### 4.3 Frame Rate Considerations

**Common Frame Rates**:
| FPS | Description | Frame Range |
|-----|-------------|-------------|
| 23.976 | NTSC film (often treated as 24) | 000-023 |
| 24 | Standard cinema | 000-023 |
| 25 | PAL standard | 000-024 |
| 29.97 | NTSC video (often treated as 30) | 000-029 |
| 30 | Video standard | 000-029 |
| 48 | High frame rate | 000-047 |

**Conversion Examples** (24 fps):

| ASS Time | Seconds | Frames | CineCanvas |
|----------|---------|--------|------------|
| `0:00:00.00` | 0.00 | 0 | `00:00:00:000` |
| `0:00:05.00` | 5.00 | 0 | `00:00:05:000` |
| `0:00:05.50` | 5.50 | 12 | `00:00:05:012` |
| `0:00:10.25` | 10.25 | 6 | `00:00:10:006` |
| `0:01:30.75` | 90.75 | 18 | `00:01:30:018` |

### 4.4 Fade Effects

**ASS Fade**:
```ass
{\fad(500,500)}Subtitle text
```
- Fade in: 500ms
- Fade out: 500ms
- Defined in override tags

**CineCanvas Fade**:
```xml
<Subtitle FadeUpTime="20" FadeDownTime="20">
```
- FadeUpTime: milliseconds
- FadeDownTime: milliseconds
- Defined in Subtitle attributes

**Conversion**:
- Parse `\fad(t1,t2)` tag
- Map `t1` → `FadeUpTime`
- Map `t2` → `FadeDownTime`
- Cinema standard: 20-40ms (ASS often uses 200-500ms)
- **Recommendation**: Clamp to 10-100ms range for cinema

---

## 5. Position and Alignment Conversion

### 5.1 ASS Alignment System

**ASS Alignment Numbers** (NumPad layout):
```
7 (Top-Left)     8 (Top-Center)     9 (Top-Right)
4 (Mid-Left)     5 (Mid-Center)     6 (Mid-Right)
1 (Bottom-Left)  2 (Bottom-Center)  3 (Bottom-Right)
```

**ASS Positioning Methods**:
1. **Alignment + Margins**: Most common
   - `Alignment`: 1-9 (NumPad layout)
   - `MarginL`: Left margin (pixels)
   - `MarginR`: Right margin (pixels)
   - `MarginV`: Vertical margin (pixels)

2. **Override tags**: `\an` (alignment) and `\pos(x,y)` (position)

### 5.2 CineCanvas Alignment System

**Attributes**:
- `VAlign`: "top", "center", "bottom"
- `VPosition`: 0.0-100.0 (percentage)
- `HAlign`: "left", "center", "right"
- `HPosition`: 0.0-100.0 (percentage)

**Coordinate System**:
```
(0,0) Top-Left          (50,0) Top-Center          (100,0) Top-Right
       ┌──────────────────────────────────────────────┐
       │                                              │
(0,50) │            (50,50) Center                    │ (100,50)
       │                                              │
       └──────────────────────────────────────────────┘
(0,100) Bottom-Left   (50,100) Bottom-Center   (100,100) Bottom-Right
```

### 5.3 Alignment Mapping

| ASS Alignment | VAlign | VPosition | HAlign | HPosition |
|---------------|--------|-----------|--------|-----------|
| 1 (Bottom-Left) | bottom | 10.0* | left | 10.0* |
| 2 (Bottom-Center) | bottom | 10.0* | center | 50.0 |
| 3 (Bottom-Right) | bottom | 10.0* | right | 10.0* |
| 4 (Mid-Left) | center | 50.0 | left | 10.0* |
| 5 (Mid-Center) | center | 50.0 | center | 50.0 |
| 6 (Mid-Right) | center | 50.0 | right | 10.0* |
| 7 (Top-Left) | top | 5.0* | left | 10.0* |
| 8 (Top-Center) | top | 5.0* | center | 50.0 |
| 9 (Top-Right) | top | 5.0* | right | 10.0* |

*Default values; adjust based on margins

### 5.4 Position Calculation from Margins

**ASS Margin System**:
- `MarginL`: Distance from left edge (pixels)
- `MarginR`: Distance from right edge (pixels)
- `MarginV`: Distance from top/bottom edge (pixels)
- Reference resolution: `PlayResX` × `PlayResY`

**Conversion to Percentage**:

```cpp
struct CineCanvasPosition {
    std::string valign;
    double vposition;
    std::string halign;
    double hposition;
};

CineCanvasPosition ConvertAlignment(
    int alignment, 
    int marginL, 
    int marginR, 
    int marginV,
    int playResX,
    int playResY
) {
    CineCanvasPosition pos;
    
    // Vertical alignment
    if (alignment <= 3) {
        // Bottom aligned
        pos.valign = "bottom";
        pos.vposition = (double)marginV / playResY * 100.0;
        if (pos.vposition < 5.0) pos.vposition = 10.0;  // Safe area
    } else if (alignment <= 6) {
        // Middle aligned
        pos.valign = "center";
        pos.vposition = 50.0;
    } else {
        // Top aligned
        pos.valign = "top";
        pos.vposition = (double)marginV / playResY * 100.0;
        if (pos.vposition < 5.0) pos.vposition = 5.0;  // Safe area
    }
    
    // Horizontal alignment
    int h_align = ((alignment - 1) % 3) + 1;  // Convert to 1-3
    if (h_align == 1) {
        // Left aligned
        pos.halign = "left";
        pos.hposition = (double)marginL / playResX * 100.0;
        if (pos.hposition < 5.0) pos.hposition = 10.0;  // Safe area
    } else if (h_align == 2) {
        // Center aligned
        pos.halign = "center";
        pos.hposition = 50.0;
    } else {
        // Right aligned
        pos.halign = "right";
        pos.hposition = (double)marginR / playResX * 100.0;
        if (pos.hposition < 5.0) pos.hposition = 10.0;  // Safe area
    }
    
    return pos;
}
```

### 5.5 Position Override Tags

**ASS `\pos(x,y)` tag**:
- Absolute pixel position
- Reference: PlayResX × PlayResY

**Conversion**:
```cpp
CineCanvasPosition ConvertAbsolutePosition(
    int x, 
    int y, 
    int playResX, 
    int playResY
) {
    CineCanvasPosition pos;
    
    // Determine alignment based on position
    // Horizontal
    if (x < playResX * 0.33) {
        pos.halign = "left";
        pos.hposition = (double)x / playResX * 100.0;
    } else if (x > playResX * 0.67) {
        pos.halign = "right";
        pos.hposition = (double)(playResX - x) / playResX * 100.0;
    } else {
        pos.halign = "center";
        pos.hposition = 50.0;
    }
    
    // Vertical
    if (y < playResY * 0.33) {
        pos.valign = "top";
        pos.vposition = (double)y / playResY * 100.0;
    } else if (y > playResY * 0.67) {
        pos.valign = "bottom";
        pos.vposition = (double)(playResY - y) / playResY * 100.0;
    } else {
        pos.valign = "center";
        pos.vposition = 50.0;
    }
    
    return pos;
}
```

### 5.6 Safe Area Adjustments

**Cinema Safe Areas**:
- Horizontal: 5-10% from edges
- Vertical: 5-12% from edges

**Clamping Function**:
```cpp
double ClampToSafeArea(double position, double min_safe = 5.0, double max_safe = 95.0) {
    if (position < min_safe) return min_safe;
    if (position > max_safe) return max_safe;
    return position;
}
```

---

## 6. Text Content Conversion

### 6.1 Override Tags

**ASS Override Tag System**:
ASS uses `{...}` blocks with backslash commands to override styling.

**Common Override Tags**:

| Tag | Purpose | CineCanvas Equivalent |
|-----|---------|----------------------|
| `\b1` | Bold on | Use bold font variant |
| `\b0` | Bold off | Revert to normal font |
| `\i1` | Italic on | Nest `<Font Italic="yes">` |
| `\i0` | Italic off | Close italic Font element |
| `\u1` | Underline on | **Not supported** |
| `\s1` | Strikeout on | **Not supported** |
| `\c&H...&` | Color change | Nest `<Font Color="...">` |
| `\fs42` | Font size | Nest `<Font Size="42">` |
| `\fn Arial` | Font name | Nest `<Font Id="Arial">` |
| `\fad(t1,t2)` | Fade in/out | Map to FadeUpTime/FadeDownTime |
| `\pos(x,y)` | Position | Map to VPosition/HPosition |
| `\move(...)` | Animation | **Not supported** |
| `\t(...)` | Transform | **Not supported** |
| `\k...` | Karaoke | **Not supported** |

### 6.2 Text Splitting Strategy

**Problem**: ASS allows inline style changes via override tags  
**Solution**: Split into multiple Font elements or Text elements

**Example**:

**ASS**:
```ass
Dialogue: 0,0:00:05.00,0:00:08.00,Default,,0,0,0,,Normal {\i1}italic{\i0} normal
```

**CineCanvas**:
```xml
<Subtitle SpotNumber="1" TimeIn="00:00:05:000" TimeOut="00:00:08:000">
  <Text VAlign="bottom" VPosition="10.0">
    Normal <Font Italic="yes">italic</Font> normal
  </Text>
</Subtitle>
```

### 6.3 Multi-line Text

**ASS Line Breaks**:
- `\N` - Hard line break (always breaks)
- `\n` - Soft line break (break if needed)

**CineCanvas Conversion**:
Split into multiple `<Text>` elements with different VPositions.

**Algorithm**:
```cpp
std::vector<std::string> SplitLines(const std::string& text) {
    std::vector<std::string> lines;
    std::string current_line;
    
    for (size_t i = 0; i < text.length(); i++) {
        if (text[i] == '\\' && i + 1 < text.length()) {
            if (text[i + 1] == 'N' || text[i + 1] == 'n') {
                lines.push_back(current_line);
                current_line.clear();
                i++;  // Skip the 'N' or 'n'
                continue;
            }
        }
        current_line += text[i];
    }
    
    if (!current_line.empty()) {
        lines.push_back(current_line);
    }
    
    return lines;
}

void CreateMultilineSubtitle(
    wxXmlNode* subtitle_node,
    const std::vector<std::string>& lines,
    double base_vposition
) {
    double line_spacing = 5.0;  // 5% spacing between lines
    
    for (int i = lines.size() - 1; i >= 0; i--) {
        wxXmlNode* text_node = new wxXmlNode(wxXML_ELEMENT_NODE, "Text");
        text_node->AddAttribute("VAlign", "bottom");
        
        double vpos = base_vposition + (lines.size() - 1 - i) * line_spacing;
        text_node->AddAttribute("VPosition", wxString::Format("%.1f", vpos));
        
        text_node->AddChild(new wxXmlNode(wxXML_TEXT_NODE, "", lines[i]));
        subtitle_node->AddChild(text_node);
    }
}
```

### 6.4 Special Character Escaping

**XML Entities Required**:

| Character | XML Entity | Example |
|-----------|-----------|---------|
| `&` | `&amp;` | `Smith &amp; Jones` |
| `<` | `&lt;` | `5 &lt; 10` |
| `>` | `&gt;` | `10 &gt; 5` |
| `"` | `&quot;` | `He said &quot;Hello&quot;` |
| `'` | `&apos;` | `It&apos;s working` |

**Escaping Function**:
```cpp
std::string EscapeXML(const std::string& text) {
    std::string result;
    result.reserve(text.length() * 1.1);  // Pre-allocate
    
    for (char c : text) {
        switch (c) {
            case '&': result += "&amp;"; break;
            case '<': result += "&lt;"; break;
            case '>': result += "&gt;"; break;
            case '"': result += "&quot;"; break;
            case '\'': result += "&apos;"; break;
            default: result += c; break;
        }
    }
    
    return result;
}
```

---

## 7. Font Handling

### 7.1 Font Name Mapping

**ASS**: Uses font name string (e.g., "Arial", "Times New Roman")  
**CineCanvas**: Uses LoadFont references with URI to font files

**Conversion Strategy**:

```cpp
struct FontMapping {
    std::string ass_name;      // "Arial"
    std::string font_file;     // "arial.ttf"
    std::string cinema_id;     // "Arial"
};

std::map<std::string, FontMapping> BuildFontMap(const AssFile& ass) {
    std::map<std::string, FontMapping> font_map;
    std::set<std::string> used_fonts;
    
    // Collect all used fonts
    for (const auto& style : ass.styles) {
        used_fonts.insert(style.fontname);
    }
    
    // Map to font files
    for (const auto& font_name : used_fonts) {
        FontMapping mapping;
        mapping.ass_name = font_name;
        mapping.cinema_id = font_name;
        mapping.font_file = FindFontFile(font_name);
        font_map[font_name] = mapping;
    }
    
    return font_map;
}
```

### 7.2 Font Weight Handling

**Problem**: CineCanvas doesn't have a "Bold" attribute

**Solutions**:

1. **Use Bold Font Variant**:
   ```xml
   <LoadFont Id="Arial" URI="arial.ttf"/>
   <LoadFont Id="ArialBold" URI="arialbd.ttf"/>
   ```

2. **Map ASS Bold Styles**:
   ```cpp
   std::string GetFontId(const std::string& font_name, bool is_bold) {
       if (is_bold) {
           return font_name + "Bold";
       }
       return font_name;
   }
   ```

### 7.3 Font Subsetting for CJK

**Problem**: CJK fonts are too large (8-15 MB)  
**Solution**: Create subset fonts with only used glyphs

**Process**:
1. Parse entire ASS file
2. Collect all unique characters
3. Extract glyphs from original font
4. Generate subset font file
5. Reference subset in LoadFont

**Implementation Note**: Use external tools (fontTools, FontForge) or defer to user

---

## 8. Effects and Animations

### 8.1 Border and Shadow

**ASS Border/Shadow**:
```ass
Style: Default,...,BorderStyle,Outline,Shadow,...
```
- `BorderStyle`: 1 (outline) or 3 (opaque box)
- `Outline`: Border width in pixels
- `Shadow`: Shadow depth in pixels

**CineCanvas Mapping**:
```xml
<Font Effect="border" EffectColor="000000FF">
```
or
```xml
<Font Effect="shadow" EffectColor="000000FF">
```

**Conversion Logic**:
```cpp
std::string ConvertBorderStyle(int border_style, int outline, int shadow) {
    if (border_style == 1 && outline > 0) {
        return "border";
    } else if (shadow > 0) {
        return "shadow";
    }
    return "none";
}
```

**Limitation**: CineCanvas doesn't support customizable border/shadow size

### 8.2 Animations

**ASS Animations** (Not Supported in CineCanvas):
- `\move(x1,y1,x2,y2,t1,t2)` - Move animation
- `\t(t1,t2,\transform)` - Transform animation
- `\fscx`, `\fscy` - Scaling
- `\frx`, `\fry`, `\frz` - 3D rotation

**Handling Strategy**:

1. **Simple Fade**: Convert to FadeUpTime/FadeDownTime
2. **Simple Move**: Create multiple subtitles at different positions
3. **Complex Animations**: 
   - **Option A**: Render to image sequence
   - **Option B**: Drop with warning
   - **Option C**: Use start/end position only

### 8.3 Karaoke Effects

**ASS Karaoke** (Not Directly Supported):
- `\k`, `\K`, `\ko`, `\kf` - Karaoke timing

**Conversion Options**:

1. **Multiple Timed Subtitles**: Split into word-by-word subtitles
2. **Image-based**: Render karaoke effect to PNG sequence
3. **Drop**: Remove karaoke timing, show full text

---

## 9. Drawing Commands

### 9.1 ASS Drawing Mode

**ASS Drawing Commands**:
```ass
{\p1}m 0 0 l 100 0 l 100 100 l 0 100{\p0}
```
- `\p1` - Enter drawing mode
- Vector drawing commands (m, l, b, s)
- `\p0` - Exit drawing mode

**CineCanvas Solution**: Convert to Image element

### 9.2 Conversion Strategy

**Process**:
1. Detect drawing mode in ASS
2. Render drawing commands to bitmap
3. Encode as PNG
4. Base64 encode
5. Embed in Image element

**Implementation**:
```cpp
std::string ConvertDrawingToImage(
    const std::string& drawing_commands,
    int width,
    int height,
    const agi::Color& color
) {
    // 1. Parse drawing commands
    std::vector<DrawCommand> commands = ParseDrawCommands(drawing_commands);
    
    // 2. Render to bitmap
    wxImage image = RenderDrawing(commands, width, height, color);
    
    // 3. Save as PNG
    wxMemoryOutputStream stream;
    image.SaveFile(stream, wxBITMAP_TYPE_PNG);
    
    // 4. Base64 encode
    wxMemoryBuffer buffer;
    stream.CopyTo(buffer.GetData(), stream.GetLength());
    std::string base64 = Base64Encode(buffer.GetData(), buffer.GetLength());
    
    return base64;
}
```

---

## 10. Complete Conversion Workflow

### 10.1 High-Level Algorithm

```cpp
void ExportToCineCanvas(
    const AssFile* ass,
    const std::string& output_path,
    const ExportOptions& options
) {
    // 1. Initialize XML document
    wxXmlDocument doc;
    wxXmlNode* root = new wxXmlNode(wxXML_ELEMENT_NODE, "DCSubtitle");
    root->AddAttribute("Version", "1.0");
    doc.SetRoot(root);
    
    // 2. Add metadata
    AddMetadata(root, ass, options);
    
    // 3. Build font map and add LoadFont elements
    auto font_map = BuildFontMap(ass);
    AddLoadFonts(root, font_map);
    
    // 4. Group dialogues by style
    auto grouped_dialogues = GroupByStyle(ass->events);
    
    // 5. Convert each style group
    for (const auto& [style_name, dialogues] : grouped_dialogues) {
        wxXmlNode* font_node = CreateFontNode(ass->GetStyle(style_name), font_map);
        
        // 6. Convert each dialogue
        int spot_number = 1;
        for (const auto& dialogue : dialogues) {
            if (dialogue->comment) continue;  // Skip comments
            
            wxXmlNode* subtitle_node = ConvertDialogue(
                dialogue, 
                spot_number++, 
                options.fps,
                ass->GetStyle(dialogue->style),
                options
            );
            
            font_node->AddChild(subtitle_node);
        }
        
        root->AddChild(font_node);
    }
    
    // 7. Save XML document
    doc.Save(output_path, 2);  // 2-space indentation
}
```

### 10.2 Metadata Generation

```cpp
void AddMetadata(
    wxXmlNode* root,
    const AssFile* ass,
    const ExportOptions& options
) {
    // SubtitleID
    std::string uuid = GenerateUUID();
    AddChildElement(root, "SubtitleID", "urn:uuid:" + uuid);
    
    // MovieTitle
    std::string title = ass->GetScriptInfo("Title");
    if (title.empty()) title = "Untitled";
    AddChildElement(root, "MovieTitle", title);
    
    // ReelNumber
    AddChildElement(root, "ReelNumber", std::to_string(options.reel_number));
    
    // Language
    AddChildElement(root, "Language", options.language_code);
}
```

### 10.3 Dialogue Conversion

```cpp
wxXmlNode* ConvertDialogue(
    const AssDialogue* dialogue,
    int spot_number,
    double fps,
    const AssStyle* style,
    const ExportOptions& options
) {
    wxXmlNode* subtitle = new wxXmlNode(wxXML_ELEMENT_NODE, "Subtitle");
    
    // Attributes
    subtitle->AddAttribute("SpotNumber", std::to_string(spot_number));
    subtitle->AddAttribute("TimeIn", ConvertTime(dialogue->start, fps));
    subtitle->AddAttribute("TimeOut", ConvertTime(dialogue->end, fps));
    
    // Fade effects
    auto [fade_in, fade_out] = ExtractFadeEffects(dialogue->text);
    if (fade_in > 0) {
        subtitle->AddAttribute("FadeUpTime", std::to_string(ClampFade(fade_in)));
    }
    if (fade_out > 0) {
        subtitle->AddAttribute("FadeDownTime", std::to_string(ClampFade(fade_out)));
    }
    
    // Text content
    std::string clean_text = StripOverrideTags(dialogue->text);
    std::vector<std::string> lines = SplitLines(clean_text);
    
    double base_vpos = options.default_vposition;
    CreateMultilineText(subtitle, lines, base_vpos, style, options);
    
    return subtitle;
}
```

---

## 11. Validation and Quality Checks

### 11.1 Pre-Export Validation

**Checks to Perform**:

```cpp
struct ValidationResult {
    bool has_errors;
    bool has_warnings;
    std::vector<std::string> errors;
    std::vector<std::string> warnings;
};

ValidationResult ValidateForCineCanvasExport(const AssFile* ass) {
    ValidationResult result;
    
    // Check for unsupported features
    for (const auto& dialogue : ass->events) {
        // Check for animations
        if (HasAnimationTags(dialogue->text)) {
            result.warnings.push_back(
                "Line " + std::to_string(dialogue->row) + 
                ": Animation tags will be ignored"
            );
        }
        
        // Check for karaoke
        if (HasKaraokeTags(dialogue->text)) {
            result.warnings.push_back(
                "Line " + std::to_string(dialogue->row) + 
                ": Karaoke effects not supported"
            );
        }
        
        // Check for drawing commands
        if (HasDrawingMode(dialogue->text)) {
            result.warnings.push_back(
                "Line " + std::to_string(dialogue->row) + 
                ": Drawing commands will be converted to images"
            );
        }
        
        // Check timing
        if (dialogue->end <= dialogue->start) {
            result.errors.push_back(
                "Line " + std::to_string(dialogue->row) + 
                ": Invalid timing (end <= start)"
            );
        }
        
        // Check character count
        std::string text = StripOverrideTags(dialogue->text);
        if (text.length() > 50) {
            result.warnings.push_back(
                "Line " + std::to_string(dialogue->row) + 
                ": Text exceeds 50 characters (readability concern)"
            );
        }
    }
    
    result.has_errors = !result.errors.empty();
    result.has_warnings = !result.warnings.empty();
    
    return result;
}
```

### 11.2 Post-Export Validation

**XML Validation**:
```cpp
bool ValidateCineCanvasXML(const std::string& xml_path) {
    // 1. Check well-formed XML
    wxXmlDocument doc;
    if (!doc.Load(xml_path)) {
        return false;
    }
    
    // 2. Validate against DTD (if available)
    // ...
    
    // 3. Check required elements
    wxXmlNode* root = doc.GetRoot();
    if (root->GetName() != "DCSubtitle") return false;
    
    if (!root->GetAttribute("Version").empty()) return false;
    if (!FindChildElement(root, "SubtitleID")) return false;
    if (!FindChildElement(root, "MovieTitle")) return false;
    if (!FindChildElement(root, "ReelNumber")) return false;
    if (!FindChildElement(root, "Language")) return false;
    
    return true;
}
```

---

## 12. Export Options and Configuration

### 12.1 Export Options Structure

```cpp
struct CineCanvasExportOptions {
    // Metadata
    int reel_number = 1;
    std::string language_code = "en";
    std::string movie_title = "";  // Empty = use ASS title
    
    // Timing
    double fps = 24.0;
    int default_fade_up = 20;    // milliseconds
    int default_fade_down = 20;  // milliseconds
    
    // Positioning
    double default_vposition = 10.0;  // percent from bottom
    bool enforce_safe_areas = true;
    
    // Fonts
    bool include_font_refs = true;
    std::map<std::string, std::string> font_file_map;
    
    // Conversion behavior
    bool convert_drawings_to_images = true;
    bool warn_on_unsupported_features = true;
    bool clamp_fade_times = true;
    int max_fade_time = 100;  // milliseconds
    
    // Color adjustments
    bool maintain_ass_colors = true;
    std::string default_text_color = "FFFFFFFF";
    std::string default_border_color = "000000FF";
};
```

### 12.2 Configuration UI

**Preference Dialog Fields**:
- Frame Rate: Dropdown (23.976, 24, 25, 29.97, 30, 48)
- Language Code: Text input with validation
- Reel Number: Integer input
- Movie Title: Text input
- Default VPosition: Slider (5-20%)
- Font References: Checkbox "Include font references"
- Safe Areas: Checkbox "Enforce cinema safe areas"

---

## 13. Testing Strategy

### 13.1 Unit Tests

**Test Cases**:

```cpp
TEST(ColorConversion, BasicColors) {
    EXPECT_EQ(ConvertASSColorToCineCanvas(0x00FFFFFF), "FFFFFFFF");  // White
    EXPECT_EQ(ConvertASSColorToCineCanvas(0x00000000), "000000FF");  // Black
    EXPECT_EQ(ConvertASSColorToCineCanvas(0x000000FF), "FF0000FF");  // Red
}

TEST(TimingConversion, FrameAccuracy) {
    EXPECT_EQ(ConvertTime("0:00:05.50", 24.0), "00:00:05:012");
    EXPECT_EQ(ConvertTime("0:01:30.25", 24.0), "00:01:30:006");
}

TEST(PositionConversion, AlignmentMapping) {
    auto pos = ConvertAlignment(2, 0, 0, 100, 1920, 1080);
    EXPECT_EQ(pos.valign, "bottom");
    EXPECT_EQ(pos.halign, "center");
}
```

### 13.2 Integration Tests

**Test Files**:
1. Simple single-line subtitles
2. Multi-line subtitles
3. Multiple styles
4. Various colors
5. Different alignments
6. Special characters
7. Complex override tags
8. Fade effects
9. Drawing commands
10. Karaoke effects

### 13.3 Round-trip Testing

**Process**:
1. Export ASS → CineCanvas
2. Import CineCanvas → ASS
3. Compare original vs. round-tripped
4. Measure fidelity

**Expected Losses**:
- Animation information
- Precise border/shadow sizes
- Karaoke timing
- Drawing commands (unless converted to images)

---

## 14. Common Conversion Issues and Solutions

### Issue 1: Colors Look Wrong

**Cause**: BGR↔RGBA confusion or alpha inversion  
**Solution**: Double-check color conversion algorithm, especially alpha handling

### Issue 2: Timing Drift

**Cause**: Frame rate mismatch or rounding errors  
**Solution**: Use consistent fps, round frame numbers properly

### Issue 3: Text Off-Screen

**Cause**: Pixel positions don't map correctly to percentages  
**Solution**: Enforce safe areas, clamp positions to 5-95% range

### Issue 4: Font Not Found

**Cause**: Font file path incorrect or font not included  
**Solution**: Validate font files exist, use font fallback mechanism

### Issue 5: Special Characters Broken

**Cause**: XML entity escaping missed  
**Solution**: Always escape &, <, >, ", ' characters

### Issue 6: Multi-line Spacing Wrong

**Cause**: Line spacing calculation incorrect  
**Solution**: Use consistent 5% spacing, adjust for number of lines

---

## 15. Future Enhancements

### 15.1 Advanced Features

1. **Smart Animation Conversion**: Detect simple animations and convert to multiple subtitles
2. **Karaoke Mode**: Option to split karaoke into word-by-word subtitles
3. **Drawing Renderer**: Built-in ASS drawing to PNG converter
4. **Font Subsetter**: Automatic CJK font subsetting
5. **Bi-directional Import**: CineCanvas → ASS conversion

### 15.2 Quality Improvements

1. **Color Space Conversion**: RGB → XYZ color space for accurate cinema colors
2. **Subtitle Preview**: Cinema-accurate preview mode
3. **DCP Validation**: Integrate with DCP validation tools
4. **Batch Processing**: Convert multiple ASS files to CineCanvas
5. **Template System**: Pre-defined style templates for cinema

---

## Appendix A: Quick Reference

### A.1 Color Conversion Formula

```
ASS (0xAABBGGRR) → CineCanvas (RRGGBBAA)
R = ASS & 0xFF
G = (ASS >> 8) & 0xFF
B = (ASS >> 16) & 0xFF
A = 255 - ((ASS >> 24) & 0xFF)
Result = sprintf("%02X%02X%02X%02X", R, G, B, A)
```

### A.2 Time Conversion Formula

```
ASS (H:MM:SS.CC) → CineCanvas (HH:MM:SS:fff)
total_seconds = H * 3600 + MM * 60 + SS + CC / 100.0
HH = total_seconds / 3600
MM = (total_seconds % 3600) / 60
SS = total_seconds % 60
fff = (SS - floor(SS)) * fps
Result = sprintf("%02d:%02d:%02d:%03d", HH, MM, floor(SS), fff)
```

### A.3 Position Conversion Formula

```
ASS Alignment (1-9) → CineCanvas (VAlign/HAlign)
VAlign = (alignment <= 3) ? "bottom" : (alignment <= 6) ? "center" : "top"
HAlign = ((alignment-1) % 3 == 0) ? "left" : ((alignment-1) % 3 == 1) ? "center" : "right"

ASS Margin → CineCanvas Position
VPosition = (MarginV / PlayResY) * 100.0
HPosition = (MarginL / PlayResX) * 100.0  // for left align
```

---

**Document Version**: 1.0  
**Last Updated**: November 13, 2025  
**Author**: Aegisub Development Team  
**Related**: CineCanvas_Technical_Spec.md, XML-PLAN.md

