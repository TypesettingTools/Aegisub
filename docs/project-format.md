# Aegisub project format

Aegisub project files use the `.aegi` extension. They are UTF-8 JSON files
whose root object contains `"format": "aegisub-project"` and an integer
`version`. Version 1 is the only currently supported version.

The project file is the canonical editable document. Subtitle files such as
ASS, SRT, and SSA are imported into a new project and are produced with the
Export command; Save never writes a subtitle delivery format.

## Version 1 sections

- `script_info`: ordered key/value objects corresponding to ASS script info.
- `styles`: typed style objects. Colors are `[red, green, blue, alpha]` arrays.
- `events`: typed dialogue metadata and opaque ASS-compatible `text` strings.
  Times are integer milliseconds and margins are `[left, right, vertical]`.
- `extradata`: typed ID/key/value objects, referenced by event ID arrays. It is
  not encoded into comments or ASS records in the project file.
- `attachments`: named font or graphic objects with base64-encoded bytes.
- `properties`: linked media, export configuration, Automation settings, and
  recoverable editor state.

All required fields are type-checked while loading. Files with a different
format marker, an unsupported version, malformed colors or margins, invalid
base64, or out-of-range integer values are rejected without modifying the
currently loaded document. Dialogue text is deliberately not parsed or
normalized by the project reader.

Automation 4 continues to receive its existing flattened info/style/dialogue
view. Project-only data is not part of that view and survives Automation edits.
