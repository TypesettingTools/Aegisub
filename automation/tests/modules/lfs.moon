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

lfs = require 'aegisub.lfs'

-- os.tmpname creates the file on POSIX; we only want a unique name
temp_name = ->
  name = os.tmpname!
  os.remove name
  name

get_pwd = ->
  pwd = io.popen 'pwd'
  dir = pwd\read!
  pwd.close!
  dir

original_dir = get_pwd!

describe 'lfs', ->
  after_each ->
    lfs.chdir original_dir

  describe 'currentdir', ->
    it 'should give the same result as pwd', ->
      dir = lfs.currentdir()
      assert.is.equal dir, get_pwd!

  describe 'chdir', ->
    it 'should change the current directory', ->
      dir = get_pwd!
      lfs.chdir '/'
      new_dir = get_pwd!
      assert.is.equal '/', new_dir
      lfs.chdir dir
      assert.is.equal get_pwd!, dir

    it 'should fail on an invalid path', ->
      name = temp_name! .. '/child'
      res, msg = lfs.chdir name

      assert.is.nil res
      assert.is.not.nil msg

  describe 'mkdir', ->
    it 'should be able to create new directories', ->
      name = temp_name!
      lfs.mkdir name
      assert.is.equal lfs.attributes(name, 'mode'), 'directory'

      res, msg = lfs.rmdir name
      assert.is.nil lfs.attributes name, 'mode'

  describe 'touch', ->
    it 'should create files if given a nonexistent filename', ->
      name = temp_name!
      lfs.touch name
      assert.is.equal lfs.attributes(name).mode, 'file'

      os.remove(name)
      assert.is.nil lfs.attributes name, 'mode'

  describe 'dir', ->
    it 'should iterate over the files in a directory', ->
      name = temp_name!
      lfs.mkdir name
      lfs.touch name .. '/a'
      lfs.touch name .. '/b'

      files = [f for f in lfs.dir name]
      table.sort files
      assert.is.same {'a', 'b'}, files

      os.remove name .. '/a'
      os.remove name .. '/b'
      lfs.rmdir name

    it 'should error for a nonexistent path like vanilla lfs', ->
      ok, err = pcall -> lfs.dir temp_name!
      assert.is.false ok
      assert.is.truthy err\find 'cannot open', 1, true

    it 'should error when given a file rather than a directory', ->
      name = temp_name!
      lfs.touch name
      ok, err = pcall -> lfs.dir name
      assert.is.false ok
      assert.is.truthy err\find 'cannot open', 1, true
      os.remove name
