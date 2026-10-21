# LRC and TTML lyric imports

Both formats are import only. Their readers and regression tests call the same
`lyrics_import.cpp` parser; the readers handle encoding/file access and transfer
parsed rows into the ASS event list.

LRC accepts `[mm:ss]` and one to three fractional digits, repeated line timestamps,
`[offset:]` with either sign, and enhanced `<mm:ss.xxx>` word markers. An offset
applies to the entire file, wherever the metadata occurs. A trailing bare word
marker sets the line end. Empty timestamp-only lines clear the previous line
without creating empty events. Rows end at the next timestamp when necessary;
short and equal-time rows are not extended into their successors.

TTML accepts default or prefixed element namespaces, UTF-8 text, `<br/>`, bare
untimed paragraphs, paragraph `begin` plus `end` or `dur`, and word spans with
`begin` and optional `end`. Clock times (`hh:mm:ss.fraction`, `mm:ss.fraction`)
and metric offsets (`ms`, `s`, `m`, `h`) are supported. Apple Music `x-bg` spans
are separated when they overlap an explicitly timed lead voice. Sequential
background words remain inline. XML whitespace nodes between words are retained;
linefeeds/tabs and consecutive spaces in text nodes are folded into spaces.
This is an import subset: frame/tick time expressions, inherited or relative
container timing, TTML styling/layout and `xml:space` preservation are not
implemented. Invalid or unsupported specified paragraph/span timestamps produce
a parse error rather than becoming untimed lines. Times are bounded to ASS's
representable range (just under ten hours).

Karaoke uses centisecond durations, with empty `\k` segments to represent leading
or inter-word gaps. Durations are computed from rounded absolute boundaries to
avoid accumulated rounding drift, and words are clipped to the dialogue bounds.
Overlapping words in one voice are clipped to the karaoke cursor because one ASS
karaoke chain cannot express parallel timings.

Regression tests in `tests/tests/lyrics_import_test.cpp` include real wxWidgets XML
loading, prefixed namespaces, Chinese text, whitespace, nested spans, ordering,
background vocals, silence gaps, clearing markers, offsets, millisecond rounding,
clipping and malformed/overflowing timestamps. They run in the existing GTest
suite with `meson test -C build --suite Aegisub`.
