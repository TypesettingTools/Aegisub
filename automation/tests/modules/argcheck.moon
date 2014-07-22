-- Copyright (c) 2014, Thomas Goyne <plorkyeran@aegisub.org>
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
--
-- Aegisub Project http://www.aegisub.org/

check = require 'aegisub.argcheck'

describe 'argcheck', ->
  it 'should permit simple valid calls', ->
    assert.has_no.errors -> (check'string' ->) ''
    assert.has_no.errors -> (check'number' ->) 10
    assert.has_no.errors -> (check'boolean' ->) true
    assert.has_no.errors -> (check'table' ->) {}

  it 'should support multiple arguments', ->
    assert.has_no.errors -> (check'string number' ->) '', 10
    assert.has_no.errors -> (check'string table number' ->) '', {}, 10

  it 'should support moonscript classes', ->
    class Foo
    assert.has_no.errors -> (check'Foo' (->) Foo)

  it 'should support optional arguments', ->
    assert.has_no.errors -> (check'?number' ->) nil
    assert.has_no.errors -> (check'?number ?number' ->) 5

  it 'should support ...', ->
    assert.has_no.errors -> (check'number ...' ->) 5
    assert.has_no.errors -> (check'number ...' ->) 5, 5
    assert.has_no.errors -> (check'number ...' ->) 5, 5, ''

  it 'should support alternates', ->
    assert.has_no.errors -> (check'number|string' ->) 5
    assert.has_no.errors -> (check'number|string' ->) ''

  it 'should support optional alternates', ->
    assert.has_no.errors -> (check'?number|string' ->) 5
    assert.has_no.errors -> (check'?number|string' ->) ''
    assert.has_no.errors -> (check'?number|string' ->) nil

  it 'should reject simple invalid calls', ->
    assert.has.errors -> (check'string' ->) 10
    assert.has.errors -> (check'number' ->) ''

  it 'should reject inccorect numbers of arguments', ->
    assert.has.errors -> (check'string number' ->) ''
    assert.has_no.errors -> (check'string ?number' ->) ''
    assert.has.errors -> (check'string number' ->) '', 5, 5

  it 'should reject non-optional nil arguments', ->
    assert.has.errors -> (check'string number' ->) nil, nil

  it 'should reject invalid matches with alternates', ->
    assert.has.errors -> (check'number|string' ->) {}

  it 'should report the correct error levels', ->
    valid_err_loc = (fn) ->
      _, err = pcall fn
      err\find('tests/modules/argcheck.moon') != nil

    assert.is.true valid_err_loc -> (check'number' ->) {}
    assert.is.true valid_err_loc -> (check'number' ->) nil
    assert.is.true valid_err_loc -> (check'number|string' ->) {}
    assert.is.true valid_err_loc -> (check'number|string' ->) nil
    assert.is.true valid_err_loc -> (check'number string' ->) {}
    assert.is.true valid_err_loc -> (check'?number ?string' ->) 1, 2, 3
    assert.is.true valid_err_loc -> (check'number string ...' ->) {}
    prevent_tail_call_so_that_this_shows_up_in_backtrace = 1

