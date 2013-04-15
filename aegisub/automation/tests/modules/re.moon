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

require 'aegisub'
require 'lunatest'
re = require 'aegisub.re'

export test_compile = ->
  r = re.compile '.'

  res = r\find 'abc'
  assert_not_nil res
  assert_equal 3, #res

  res = r\find 'bc'
  assert_not_nil res
  assert_equal 2, #res

export test_compile_bad_call_is_error = ->
  r = re.compile '.'
  assert_error -> r.find 'bc'

export test_find_bol_only_matches_once = ->
  res = re.find 'aaa', '^a'
  assert_not_nil res
  assert_equal 1, #res

export test_find_finds_characters_not_bytes = ->
  res = re.find '☃☃', '.'
  assert_not_nil res
  assert_equal 2, #res
  assert_equal '☃', res[1].str
  assert_equal 1,   res[1].first
  assert_equal 3,   res[1].last
  assert_equal '☃', res[2].str
  assert_equal 4,   res[2].first
  assert_equal 6,   res[2].last

export test_find_no_matches_returns_nil = ->
  assert_nil(re.find 'a', 'b')

export test_find_on_empty_string_returns_nil = ->
  assert_nil(re.find '', '.')

export test_find_plain = ->
  res = re.find 'aa', '.'
  assert_not_nil res
  assert_equal 2, #res
  assert_equal 'a', res[1].str
  assert_equal 1,   res[1].first
  assert_equal 1,   res[1].last
  assert_equal 'a', res[2].str
  assert_equal 2,   res[2].first
  assert_equal 2,   res[2].last

export test_find_zero_length_match = ->
  res = re.find 'abc', '^'
  assert_not_nil res
  assert_equal 1, #res
  assert_equal '', res[1].str
  assert_equal 1, res[1].first
  assert_equal 0, res[1].last

export test_flags_in_wrong_position_is_error = ->
  assert_error -> re.sub 'a', re.ICASE, 'b', 'c'
  assert_error -> re.sub 'a', 'b', re.ICASE, 'c'

export test_invalid_regex_syntax_is_error = ->
  assert_error -> re.compile '('

export test_match_plain = ->
  res = re.match '{250 1173 380}Help!', '(\\d+) (\\d+) (\\d+)'
  assert_not_nil res
  assert_equal 4, #res
  assert_equal '250 1173 380', res[1].str
  assert_equal 2,              res[1].first
  assert_equal 13,             res[1].last

  assert_equal '250', res[2].str
  assert_equal 2,     res[2].first
  assert_equal 4,     res[2].last

  assert_equal '1173', res[3].str
  assert_equal 6,      res[3].first
  assert_equal 9,      res[3].last

  assert_equal '380', res[4].str
  assert_equal 11,    res[4].first
  assert_equal 13,    res[4].last

export test_match_zero_length_match = ->
  res = re.match 'abc', '^'
  assert_not_nil res
  assert_equal 1, #res
  assert_equal '', res[1].str
  assert_equal 1, res[1].first
  assert_equal 0, res[1].last

export test_split_delim_not_found_gives_input_string_in_table = ->
  res = re.split 'abc', ','
  assert_not_nil res
  assert_equal 1, #res
  assert_equal 'abc', res[1]

export test_split_empty_string_returns_empty_table = ->
  res = re.split '', ','
  assert_not_nil res
  assert_equal 0, #res

export test_split_max_splits_noskip = ->
  res = re.split 'a,,b,c', ',', false, 1
  assert_not_nil res
  assert_equal 2, #res
  assert_equal 'a', res[1]
  assert_equal ',b,c', res[2]

export test_split_max_splits_skip = ->
  res = re.split 'a,,b,c', ',', true, 1
  assert_not_nil res
  assert_equal 2, #res
  assert_equal 'a', res[1]
  assert_equal ',b,c', res[2]

  res = re.split 'a,,b,c,d', ',', true, 2
  assert_not_nil res
  assert_equal 3, #res
  assert_equal 'a', res[1]
  assert_equal 'b', res[2]
  assert_equal 'c,d', res[3]

export test_split_multi_character_delimeter = ->
  res = re.split 'a::b::c:d', '::'
  assert_not_nil res
  assert_equal 3, #res
  assert_equal 'a', res[1]
  assert_equal 'b', res[2]
  assert_equal 'c:d', res[3]

export test_split_plain = ->
  res = re.split 'a,,b,c', ','
  assert_not_nil res
  assert_equal 4, #res
  assert_equal 'a', res[1]
  assert_equal '',  res[2]
  assert_equal 'b', res[3]
  assert_equal 'c', res[4]

export test_split_skip_empty = ->
  res = re.split 'a,,b,c', ',', true
  assert_not_nil res
  assert_equal 3, #res
  assert_equal 'a', res[1]
  assert_equal 'b', res[2]
  assert_equal 'c', res[3]

export test_sub_bad_find_is_error = ->
  assert_error -> re.sub 'aa', 5, 'b'

export test_sub_bad_haystack_is_error = ->
  assert_error -> re.sub 5, 'a', 'b'

export test_sub_bad_replacement_is_error = ->
  assert_error -> re.sub 'aa', 'a', 5

export test_sub_capture_groups = ->
  assert_equal 'aabbaabb', re.sub 'abab', '(a)(b)', (str) -> str .. str

export test_sub_empty_string_returns_empty = ->
  assert_equal '', re.sub '', 'b', 'c' 

export test_sub_function = ->
  add_one = (str) -> tostring tonumber(str) + 1

  res = re.sub '{\\k10}a{\\k15}b{\\k30}c', '\\\\k([[:digit:]]+)', add_one
  assert_not_nil res
  assert_equal '{\\k11}a{\\k16}b{\\k31}c', res

export test_sub_function_no_capture_group_passes_full_match = ->
  found = {}
  drop_match = (str) ->
    table.insert found, str
    ''

  res = re.sub '{\\k10}a{\\k15}b{\\k30}c', '\\\\k[[:digit:]]+', drop_match
  assert_not_nil res
  assert_equal '{}a{}b{}c', res

  assert_equal 3, #found
  assert_equal '\\k10', found[1]
  assert_equal '\\k15', found[2]
  assert_equal '\\k30', found[3]

export test_sub_function_returning_non_string_returns_unchanged_input = ->
  found = {}
  mutate_external = (str) ->
    table.insert found, str
    nil

  res = re.sub '{\\k10}a{\\k15}b{\\k30}c', '\\\\k([[:digit:]]+)', mutate_external
  assert_not_nil res
  assert_equal '{\\k10}a{\\k15}b{\\k30}c', res

  assert_equal 3, #found
  assert_equal '10', found[1]
  assert_equal '15', found[2]
  assert_equal '30', found[3]

export test_sub_icase = ->
  res = re.sub '{\\K10}a{\\K15}b{\\k30}c', '\\\\k', '\\\\kf', re.ICASE
  assert_not_nil res
  assert_equal '{\\kf10}a{\\kf15}b{\\kf30}c', res

export test_sub_icase_greek = ->
  res = re.sub '!συνεργ!', 'Συνεργ', 'foo', re.ICASE
  assert_not_nil res
  assert_equal '!foo!', res

export test_sub_max_replace_count = ->
  res = re.sub 'aaa', 'a', 'b', 2
  assert_not_nil res
  assert_equal 'bba', res

export test_sub_no_matches_leaves_unchanged = ->
  assert_equal('a', re.sub 'a', 'b', 'c')

export test_sub_plain = ->
  res = re.sub '{\\k10}a{\\k15}b{\\k30}c', '\\\\k', '\\\\kf'
  assert_not_nil res
  assert_equal '{\\kf10}a{\\kf15}b{\\kf30}c', res

export test_sub_zero_length_match_bol = ->
  res = re.sub 'abc', '^', 'd'
  assert_not_nil res
  assert_equal 'dabc', res

export test_sub_zero_length_match_bow = ->
  res = re.sub 'abc abc', '\\<', 'd'
  assert_not_nil res
  assert_equal 'dabc dabc', res

export test_sub_zero_length_match_bob = ->
  res = re.sub 'abc', '\\A', 'd'
  assert_not_nil res
  assert_equal 'dabc', res

assert_empty_match_and_return_d = (str) ->
  assert_equal '', str
  'd'

export test_sub_zero_length_match_bol_with_function_replacement = ->
  res = re.sub 'abc', '^', assert_empty_match_and_return_d
  assert_not_nil res
  assert_equal 'dabc', res

export test_sub_zero_length_match_bow_with_function_replacement = ->
  res = re.sub 'abc abc', '\\<', assert_empty_match_and_return_d
  assert_not_nil res
  assert_equal 'dabc dabc', res

export test_sub_zero_length_match_bob_with_function_replacement = ->
  res = re.sub 'abc', '\\A', assert_empty_match_and_return_d
  assert_not_nil res
  assert_equal 'dabc', res

export test_sub_zero_length_match_eol = ->
  res = re.sub 'abc', '$', 'd'
  assert_not_nil res
  assert_equal 'abcd', res

export test_sub_zero_length_match_eol_with_function_replacement = ->
  res = re.sub 'abc', '$', assert_empty_match_and_return_d
  assert_not_nil res
  assert_equal 'abcd', res

export test_sub_zero_length_match_mid = ->
  res = re.sub 'abc', 'e?', 'd'
  assert_not_nil res
  assert_equal 'dadbdcd', res

  res = re.sub 'abc', 'b*', 'd'
  assert_not_nil res
  assert_equal 'daddcd', res

export test_sub_zero_length_match_mid_with_function_replacement = ->
  res = re.sub 'abc', 'e?', assert_empty_match_and_return_d
  assert_not_nil res
  assert_equal 'dadbdcd', res

export test_zero_length_regex_is_error = ->
  assert_error -> re.compile ''

export test_split_word_boundary = ->
  res = re.split 'aa bb cc', '\\b', true
  assert_not_nil res
  assert_equal 5, #res
  assert_equal res[1], 'aa'
  assert_equal res[2], ' '
  assert_equal res[3], 'bb'
  assert_equal res[4], ' '
  assert_equal res[5], 'cc'

export test_split_bob = ->
  res = re.split 'aa bb cc', '\\A'
  assert_not_nil res
  assert_equal 2, #res
  assert_equal '', res[1]
  assert_equal 'aa bb cc', res[2]

