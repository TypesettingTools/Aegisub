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

unicode = require 'aegisub.unicode'

describe 'charwidth', ->
  it 'should return 1 for an ascii character', ->
    assert.is.equal 1, unicode.charwidth 'a'
  it 'should return 2 for a two byte character', ->
    assert.is.equal 2, unicode.charwidth 'ß'
  it 'should return 3 for a three byte character', ->
    assert.is.equal 3, unicode.charwidth 'ｃ'
  it 'should return 4 for a four byte character', ->
    assert.is.equal 4, unicode.charwidth '🄓'

describe 'char_iterator', ->
  it 'should iterator over multi-byte codepoints', ->
    chars = [c for c in unicode.chars 'aßｃ🄓']
    assert.is.equal 4, #chars
    assert.is.equal chars[1], 'a'
    assert.is.equal chars[2], 'ß'
    assert.is.equal chars[3], 'ｃ'
    assert.is.equal chars[4], '🄓'

describe 'len', ->
  it 'should give length in codepoints', ->
    assert.is.equal 4, unicode.len 'aßｃ🄓'

describe 'codepoint', ->
  it 'should give codepoint as an integer for a string', ->
    assert.is.equal 97, unicode.codepoint 'a'
    assert.is.equal 223, unicode.codepoint 'ß'
    assert.is.equal 0xFF43, unicode.codepoint 'ｃ'
    assert.is.equal 0x1F113, unicode.codepoint '🄓'
  it 'should give ignore codepoints after the first', ->
    assert.is.equal 97, unicode.codepoint 'abc'
