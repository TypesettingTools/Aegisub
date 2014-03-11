-- Copyright (c) 2013, Thomas Goyne <plorkyeran@aegisub.org>
--
-- Permission to use, copy, modify, and distribute this software for any
-- purpose with or without fee is hereby granted, provided that the above
-- copyright notice and this permission notice appear in all copies.
--
-- THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
-- WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
-- MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
-- ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
-- WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
-- ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
-- OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

require 'lunatest'
unicode = require 'aegisub.unicode'

export test_char_widths = ->
  assert_equal 1, unicode.charwidth 'a'
  assert_equal 2, unicode.charwidth 'ß'
  assert_equal 3, unicode.charwidth 'ｃ'
  assert_equal 4, unicode.charwidth '🄓'

export test_char_iterator = ->
  chars = [c for c in unicode.chars 'aßｃ🄓']
  assert_equal 4, #chars
  assert_equal chars[1], 'a'
  assert_equal chars[2], 'ß'
  assert_equal chars[3], 'ｃ'
  assert_equal chars[4], '🄓'

export test_len = ->
  assert_equal 4, unicode.len 'aßｃ🄓'

export test_codepoint = ->
  assert_equal 97, unicode.codepoint 'a'
  assert_equal 223, unicode.codepoint 'ß'
  assert_equal 0xFF43, unicode.codepoint 'ｃ'
  assert_equal 0x1F113, unicode.codepoint '🄓'
