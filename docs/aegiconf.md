# Project settings (.aegiconf)

By default, Aegisub saves current project settings in `.aegiconf-[full filename]`
next to each subtitle; for example `.aegiconf-episode01.ass`. ASS files retain
parsed project settings at their load-time values, even if active settings change
or a config file overrides them. Saving normalizes keys and formatting, and legacy
settings in `[Script Info]` may move into `[Aegisub Project Garbage]`. Comments and
unrecognized metadata follow the normal ASS parsing behavior. Empty project
settings sections are omitted.
Legacy project settings in subtitle files are used only when no matching
`.aegiconf` file is loaded, or when config loading is disabled.

For shared settings, create a `.aegiconf` file in the subtitle's directory or
an ancestor directory. Aegisub searches upward until it reaches a directory
containing `.git` (a directory or a worktree/submodule file), the filesystem root,
or a mount boundary. It then applies config files from the highest directory down.
Finally, it applies `.aegiconf-[full filename]` from the subtitle's own directory.
Each file overrides matching settings from earlier config files. If any matching
config file is loaded, all in-file project settings are discarded, including values
not specified in the config cascade. Empty or comment-only config files also
discard the in-file settings.

Files contain the same `Key: value` directives as project garbage, without a
required section header. Blank lines and comments starting with `;` or `#` are
ignored. All relative media paths and file patterns are resolved relative to the
subtitle file's directory, regardless of where the `.aegiconf` is located.
Absolute paths remain absolute.

For example, a shared `.aegiconf` above separate episode directories can contain:

```text
Video File Pattern: seriesname_*_premux.mkv
# Fallback on workraw if premux isn't available
Video File Pattern: seriesname_*_wr.mp4
Audio File: ?video
Keyframes File Pattern: *.key.txt
```

`Video File Pattern`, `Audio File Pattern`, `Keyframes File Pattern`, and
`Timecodes File Pattern` search within each subtitle's directory using filesystem
wildcards, including wildcards in directory components. Patterns are tried in
their written order. Within one
pattern, matches are sorted by filename/path and the first regular file is used.
Patterns are not recursive (`**` has no special meaning). The shared example
therefore selects media from each episode's own directory. Use narrower per-file
patterns when several files in that directory would match.

A closer config containing any patterns or an explicit file for a medium replaces
the complete candidate list for that medium. If none of its patterns match, the
value is left unset; parent candidates are not retried. An explicit `Video File`
(and equivalently audio, keyframes, or timecodes) replaces earlier candidates in
the same config. `Audio File: ?video` loads audio from the selected video.

Saving writes current settings to the per-subtitle config, which is also marked
hidden on Windows. Paths discovered from
patterns are omitted so they can be discovered again next time. Manually opening
or closing associated media replaces its per-file patterns with an explicit path
or empty value. Existing comments, unknown directives, and still-active patterns
in the per-subtitle config are preserved. Empty and zero values are saved too,
allowing local settings to clear inherited values. Recovery autosaves always
contain project settings in their `[Aegisub Project Garbage]` section, including
paths resolved from patterns.

General preferences include:

- **Save Project Settings in Subtitle File**: when enabled, write current project
  settings into the ASS file instead of updating the per-subtitle config. When
  disabled, preserve the in-file project settings captured at load time and write
  current settings to the per-subtitle config. Files loaded without in-file project
  settings remain without them. Config loading is controlled separately.
- **Load .aegiconf project settings**: disable this to read project settings only
  from the subtitle. This preference is independent of the save destination.
- **Save UI state**: controls saving the active line, scroll position, video
  position, and zoom, in either save mode.

To keep machine-local state out of Git, add `.aegiconf-*` to the project's
`.gitignore`. Shared `.aegiconf` files can still be checked in.
