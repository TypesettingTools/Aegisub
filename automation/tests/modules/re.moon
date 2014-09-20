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

re = require 'aegisub.re'

describe 'compile', ->
  it 'should return a object with the regexp bound', ->
    r = re.compile '.'

    res = r\find 'abc'
    assert.is.not.nil res
    assert.is.equal 3, #res

    res = r\find 'bc'
    assert.is.not.nil res
    assert.is.equal 2, #res

  it 'should throw an error when it is not called with \\', ->
    r = re.compile '.'
    assert.is.error -> r.find 'bc'

  it 'should throw an error when given an invalid regex', ->
    assert.is.error -> re.compile '('

  it 'should throw an error when given an empty regex', ->
    assert.is.error -> re.compile ''

describe 'find', ->
  it 'should only match once when using ^', ->
    res = re.find 'aaa', '^a'
    assert.is.not.nil res
    assert.is.equal 1, #res

  it 'should match codepoints with ., not bytes', ->
    res = re.find '☃☃', '.'
    assert.is.not.nil res
    assert.is.equal 2, #res
    assert.is.equal '☃', res[1].str
    assert.is.equal 1,   res[1].first
    assert.is.equal 3,   res[1].last
    assert.is.equal '☃', res[2].str
    assert.is.equal 4,   res[2].first
    assert.is.equal 6,   res[2].last

  it 'should return nil when there are no matches', ->
    assert.is.nil(re.find 'a', 'b')

  it 'should return nil when called on an empty string', ->
    assert.is.nil(re.find '', '.')

  it 'should return full match information', ->
    res = re.find 'aa', '.'
    assert.is.not.nil res
    assert.is.equal 2, #res
    assert.is.equal 'a', res[1].str
    assert.is.equal 1,   res[1].first
    assert.is.equal 1,   res[1].last
    assert.is.equal 'a', res[2].str
    assert.is.equal 2,   res[2].first
    assert.is.equal 2,   res[2].last

  it 'should be able to handle a zero-length match', ->
    res = re.find 'abc', '^'
    assert.is.not.nil res
    assert.is.equal 1, #res
    assert.is.equal '', res[1].str
    assert.is.equal 1, res[1].first
    assert.is.equal 0, res[1].last

describe 'flag handling', ->
  it 'should be an error to pass flags somewhere other than the end', ->
    assert.is.error -> re.sub 'a', re.ICASE, 'b', 'c'
    assert.is.error -> re.sub 'a', 'b', re.ICASE, 'c'

describe 'match', ->
  it 'should be able to extract values from multiple match groups', ->
    res = re.match '{250 1173 380}Help!', '(\\d+) (\\d+) (\\d+)'
    assert.is.not.nil res
    assert.is.equal 4, #res
    assert.is.equal '250 1173 380', res[1].str
    assert.is.equal 2,              res[1].first
    assert.is.equal 13,             res[1].last

    assert.is.equal '250', res[2].str
    assert.is.equal 2,     res[2].first
    assert.is.equal 4,     res[2].last

    assert.is.equal '1173', res[3].str
    assert.is.equal 6,      res[3].first
    assert.is.equal 9,      res[3].last

    assert.is.equal '380', res[4].str
    assert.is.equal 11,    res[4].first
    assert.is.equal 13,    res[4].last

  it 'should handle zero length matches', ->
    res = re.match 'abc', '^'
    assert.is.not.nil res
    assert.is.equal 1, #res
    assert.is.equal '', res[1].str
    assert.is.equal 1, res[1].first
    assert.is.equal 0, res[1].last

describe 'split', ->
  it 'should return the input string in a table when the delimiter does not appear in the string', ->
    res = re.split 'abc', ','
    assert.is.not.nil res
    assert.is.equal 1, #res
    assert.is.equal 'abc', res[1]

  it 'should return an empty table when given an empty string', ->
    res = re.split '', ','
    assert.is.not.nil res
    assert.is.equal 0, #res

  it 'should honor the max splits argument', ->
    res = re.split 'a,,b,c', ',', false, 1
    assert.is.not.nil res
    assert.is.equal 2, #res
    assert.is.equal 'a', res[1]
    assert.is.equal ',b,c', res[2]

  it 'should not count skipped segments in the maximum when skipping empty', ->
    res = re.split 'a,,b,c', ',', true, 1
    assert.is.not.nil res
    assert.is.equal 2, #res
    assert.is.equal 'a', res[1]
    assert.is.equal ',b,c', res[2]

    res = re.split 'a,,b,c,d', ',', true, 2
    assert.is.not.nil res
    assert.is.equal 3, #res
    assert.is.equal 'a', res[1]
    assert.is.equal 'b', res[2]
    assert.is.equal 'c,d', res[3]

  it 'should support multi-character delimiters', ->
    res = re.split 'a::b::c:d', '::'
    assert.is.not.nil res
    assert.is.equal 3, #res
    assert.is.equal 'a', res[1]
    assert.is.equal 'b', res[2]
    assert.is.equal 'c:d', res[3]

  it 'should not fail on basic splits', ->
    res = re.split 'a,,b,c', ','
    assert.is.not.nil res
    assert.is.equal 4, #res
    assert.is.equal 'a', res[1]
    assert.is.equal '',  res[2]
    assert.is.equal 'b', res[3]
    assert.is.equal 'c', res[4]

  it 'should be able to skip empty segments', ->
    res = re.split 'a,,b,c', ',', true
    assert.is.not.nil res
    assert.is.equal 3, #res
    assert.is.equal 'a', res[1]
    assert.is.equal 'b', res[2]
    assert.is.equal 'c', res[3]

  it 'should return an empty table when given only empty segments', ->
    res = re.split ',,,', ',', true
    assert.is.not.nil res
    assert.is.equal 0, #res

  it 'should be able to split on word boundaries', ->
    res = re.split 'aa bb cc', '\\b', true
    assert.is.not.nil res
    assert.is.equal 5, #res
    assert.is.equal res[1], 'aa'
    assert.is.equal res[2], ' '
    assert.is.equal res[3], 'bb'
    assert.is.equal res[4], ' '
    assert.is.equal res[5], 'cc'

  it 'should be able to split on the beginning of the buffer', ->
    res = re.split 'aa bb cc', '\\A'
    assert.is.not.nil res
    assert.is.equal 2, #res
    assert.is.equal '', res[1]
    assert.is.equal 'aa bb cc', res[2]

describe 'sub', ->
  it 'should throw an error when given a non-string to search for', ->
    assert.is.error -> re.sub 'aa', 5, 'b'

  it 'should throw an error when given a non-string to search in', ->
    assert.is.error -> re.sub 5, 'a', 'b'

  it 'should throw an error when given a non-string to replace with', ->
    assert.is.error -> re.sub 'aa', 'a', 5

  it 'should pass capture groups to the replacement function if given one', ->
    assert.is.equal 'aabbaabb', re.sub 'abab', '(a)(b)', (str) -> str .. str

  it 'should return an empty string when given an empty string', ->
    assert.is.equal '', re.sub '', 'b', 'c' 

  it 'should support functions for replacements in addition to strings', ->
    add_one = (str) -> tostring tonumber(str) + 1

    res = re.sub '{\\k10}a{\\k15}b{\\k30}c', '\\\\k([[:digit:]]+)', add_one
    assert.is.not.nil res
    assert.is.equal '{\\k11}a{\\k16}b{\\k31}c', res

  it 'should pass the full match to the replacement function if there are no capture groups', ->
    found = {}
    drop_match = (str) ->
      table.insert found, str
      ''

    res = re.sub '{\\k10}a{\\k15}b{\\k30}c', '\\\\k[[:digit:]]+', drop_match
    assert.is.not.nil res
    assert.is.equal '{}a{}b{}c', res

    assert.is.equal 3, #found
    assert.is.equal '\\k10', found[1]
    assert.is.equal '\\k15', found[2]
    assert.is.equal '\\k30', found[3]

  it 'should return the input unchanged if the replacement function does not return a string', ->
    found = {}
    mutate_external = (str) ->
      table.insert found, str
      nil

    res = re.sub '{\\k10}a{\\k15}b{\\k30}c', '\\\\k([[:digit:]]+)', mutate_external
    assert.is.not.nil res
    assert.is.equal '{\\k10}a{\\k15}b{\\k30}c', res

    assert.is.equal 3, #found
    assert.is.equal '10', found[1]
    assert.is.equal '15', found[2]
    assert.is.equal '30', found[3]

  it 'should be able to do case-insensitive replacements on English', ->
    res = re.sub '{\\K10}a{\\K15}b{\\k30}c', '\\\\k', '\\\\kf', re.ICASE
    assert.is.not.nil res
    assert.is.equal '{\\kf10}a{\\kf15}b{\\kf30}c', res

  it 'should be able to do case-insensitive replacements on Greek', ->
    res = re.sub '!συνεργ!', 'Συνεργ', 'foo', re.ICASE
    assert.is.not.nil res
    assert.is.equal '!foo!', res

  it 'should be able to limit the number of replacements', ->
    res = re.sub 'aaa', 'a', 'b', 2
    assert.is.not.nil res
    assert.is.equal 'bba', res

  it 'should return the input unchanged if there are no matches', ->
    assert.is.equal('a', re.sub 'a', 'b', 'c')

  it 'should be able to do simple string replacements', ->
    res = re.sub '{\\k10}a{\\k15}b{\\k30}c', '\\\\k', '\\\\kf'
    assert.is.not.nil res
    assert.is.equal '{\\kf10}a{\\kf15}b{\\kf30}c', res

  it 'should replace only once when given a zero-length-bol-match', ->
    res = re.sub 'abc', '^', 'd'
    assert.is.not.nil res
    assert.is.equal 'dabc', res

  it 'should replace only once when given a zero-length-bow-match', ->
    res = re.sub 'abc abc', '\\<', 'd'
    assert.is.not.nil res
    assert.is.equal 'dabc dabc', res

  it 'should replace only once when given a zero-length-bob-match', ->
    res = re.sub 'abc', '\\A', 'd'
    assert.is.not.nil res
    assert.is.equal 'dabc', res

  assert_empty_match_and_return_d = (str) ->
    assert.is.equal '', str
    'd'

  it 'should replace only once when given a zero-length-bol-match and a function replacements', ->
    res = re.sub 'abc', '^', assert_empty_match_and_return_d
    assert.is.not.nil res
    assert.is.equal 'dabc', res

  it 'should replace only once when given a zero-length-bow-match and a function replacements', ->
    res = re.sub 'abc abc', '\\<', assert_empty_match_and_return_d
    assert.is.not.nil res
    assert.is.equal 'dabc dabc', res

  it 'should replace only once when given a zero-length-bob-match and a function replacements', ->
    res = re.sub 'abc', '\\A', assert_empty_match_and_return_d
    assert.is.not.nil res
    assert.is.equal 'dabc', res

  it 'should replace only once when given a zero-length-eol-match', ->
    res = re.sub 'abc', '$', 'd'
    assert.is.not.nil res
    assert.is.equal 'abcd', res

  it 'should replace only once when given a zero-length-eol-match and a function replacements', ->
    res = re.sub 'abc', '$', assert_empty_match_and_return_d
    assert.is.not.nil res
    assert.is.equal 'abcd', res

  it 'should apply unanchored zero-length matches at each point in the string', ->
    res = re.sub 'abc', 'e?', 'd'
    assert.is.not.nil res
    assert.is.equal 'dadbdcd', res

    res = re.sub 'abc', 'b*', 'd'
    assert.is.not.nil res
    assert.is.equal 'daddcd', res

  it 'should apply unanchored zero-length matches with function insertions at each point in the string', ->
    res = re.sub 'abc', 'e?', assert_empty_match_and_return_d
    assert.is.not.nil res
    assert.is.equal 'dadbdcd', res

