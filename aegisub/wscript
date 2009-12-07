#! /usr/bin/env python
# encoding: utf-8
#
# Copyright (c) 2009, Kevin Ollivier <kollivier@aegisub.org>
#
# Permission to use, copy, modify, and distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

import os
import sys

import Options

VERSION= (2,2,0)
APPNAME='Aegisub'

appdir = os.path.abspath('.')

srcdir = '.'
blddir = 'build'

def get_path_to_wxconfig():
    if 'WX_CONFIG' in os.environ:
        return os.environ['WX_CONFIG']
    else:
        return 'wx-config'

def set_options(opt):
    opt.tool_options('compiler_cxx compiler_cc')

    audio_player_default = "PortAudio"
    if sys.platform.startswith('darwin'):
        audio_player_default = "OpenAL"
    elif sys.platform.startswith("linux"):
        audio_player_default = "ALSA"

    opt.add_option('--debug', action='store_true', help='build in debug mode')
    opt.add_option('--with-provider-video', action='store', default="ffmpegsource", help='Default Video Provider. (default: ffmpegsource)')
    opt.add_option('--with-provider-audio', action='store', default="ffmpegsource", help='Default Audio Provider. (default: ffmpegsource)')
    opt.add_option('--with-player-audio', action='store',  default=audio_player_default, help='Default Audio Player (default: Linux/ALSA, Darwin/OpenAL, */PortAudio.)')
    opt.add_option('--with-provider-subtitle', action='store',  default='libass', help='Default Subtitle Provider. (default: libass)')

def configure(conf):
    conf.check_tool('compiler_cxx compiler_cc')
    if sys.platform.startswith('darwin'):
        conf.check_tool('osx')
        
    conf.env.append_value('CPPPATH', '/usr/local/aegisub-deps/include')
    
    conf.define('HAVE_GETTEXT', 1)
    
    if not sys.platform.startswith('win'):
        conf.define('HAVE_PTHREAD', 1)
    
    conf.check_cc(fragment='''
            #include <libintl.h>
            int main() { dcgettext('test', 'test', 0); }
    ''', define_name='HAVE_DCGETTEXT', define_ret='1')
    
    conf.check_cc(fragment='''
            #include <libintl.h>
            int main() { gettext('test', 'test', 0); }
    ''', define_name='HAVE_GETTEXT', define_ret='1')

    conf.check_cc(define_name='HAVE_LOCALE_H', header_name='locale.h', define_ret='1')
    conf.check_cc(define_name='HAVE_MEMORY_H', header_name='memory.h', define_ret='1')
    conf.check_cc(define_name='HAVE_SOUNDCARD_H', header_name='soundcard.h', define_ret='1')
    conf.check_cc(define_name='HAVE_STDINT_H', header_name='stdint.h', define_ret='1')
    conf.check_cc(define_name='HAVE_STDLIB_H', header_name='stdlib.h', define_ret='1')
    conf.check_cc(define_name='HAVE_STDBOOL_H', header_name='stdbool.h', define_ret='1')
    conf.check_cc(define_name='HAVE_STRING_H', header_name='string.h', define_ret='1')
    conf.check_cc(define_name='HAVE_STRINGS_H', header_name='strings.h', define_ret='1')
    conf.check_cc(define_name='HAVE_SYS_STAT_H', header_name='sys/stat.h', define_ret='1')
    conf.check_cc(define_name='HAVE_SYS_TYPES_H', header_name='sys/types.h', define_ret='1')
    conf.check_cc(define_name='HAVE_UNISTD_H', header_name='unistd.h', define_ret='1')
    
    conf.define('AEGISUB_VERSION_DATA', "%d.%d" % (VERSION[0], VERSION[1]))
    
    conf.define('DEFAULT_PROVIDER_VIDEO', Options.options.with_provider_video)
    conf.define('DEFAULT_PROVIDER_AUDIO', Options.options.with_provider_audio)
    conf.define('DEFAULT_PLAYER_AUDIO', Options.options.with_player_audio)
    conf.define('DEFAULT_PROVIDER_SUBTITLE', 'libass') # FIXME!
    
    wants_ffmpeg = False
    if Options.options.with_provider_video == 'ffmpegsource':
        conf.define('WITH_FFMPEGSOURCE', 1)
        wants_ffmpeg = True
        
    if Options.options.with_provider_audio == 'ffmpegsource':
        conf.define('WITH_FFMPEGSOURCE', 1)
        wants_ffmpeg = True
        
    if Options.options.with_provider_subtitle == 'libass':
         #conf.env['LIBASS_LIB'] = 'ass_aegisub'
         conf.env.append_value('CPPPATH', '../libass')
         conf.define('WITH_LIBASS', 1)
        
    if wants_ffmpeg:
        conf.check_cfg(package='libavcodec', args='--cflags --libs', uselib_store='FFMPEG', mandatory=True)
        conf.check_cfg(package='libavformat', args='--cflags --libs', uselib_store='FFMPEG', mandatory=True)
        conf.check_cfg(package='libswscale', args='--cflags --libs', uselib_store='FFMPEG', mandatory=True)
        conf.check_cfg(package='libavutil', args='--cflags --libs', uselib_store='FFMPEG', mandatory=True)
        conf.check_cfg(package='libpostproc', args='--cflags --libs', uselib_store='FFMPEG', mandatory=True)
    
    audio_player = Options.options.with_player_audio
    if audio_player.lower() == 'openal':
        if sys.platform.startswith('darwin'):
            conf.env.append_value('FRAMEWORK', 'OpenAL')
        else:
            conf.check_cfg(package='openal', args='--cflags --libs', uselib_store='FFMPEG', mandatory=True)
        conf.define('WITH_OPENAL', 1)
    
    gettext_version = ''
    if not sys.platform.startswith('darwin'):
        gettext_version = "%d%d" % (VERSION[0], VERSION[1])
    conf.define('GETTEXT_PACKAGE', 'aegisub')
    
    conf.env.append_value('LIB', 'iconv')
    
    # deps
    conf.check_cfg(path=get_path_to_wxconfig(), args='--cxxflags --libs core,adv,gl,net,stc,xml', package='', uselib_store='WX', mandatory=True)
    conf.check_cfg(path='freetype-config', args='--cflags --libs', package='', uselib_store='FREETYPE2', mandatory=True)
    conf.check_cfg(path='curl-config', args='--cflags --libs', package='', uselib_store='CURL', mandatory=True)
    conf.check_cfg(package='fontconfig', args='--cflags --libs', uselib_store='FONTCONFIG', mandatory=True)
    
    if sys.platform.startswith('darwin'):
        conf.define('HAVE_APPLE_OPENGL_FRAMEWORK', 1)
    #    conf.env['CPPPATHS_GL'] = ['/System/Library/Frameworks/OpenGL.framework/Versions/A/Headers']

    conf.write_config_header('acconf.h')

    if Options.options.debug:
        conf.env.append_value('CCFLAGS', conf.env['CCFLAGS_DEBUG'])
        conf.env.append_value('CXXFLAGS', conf.env['CXXFLAGS_DEBUG'])
        conf.env.append_value('LINKFLAGS', conf.env['LINKFLAGS_DEBUG'])

    if conf.env.CXX_NAME == 'gcc':
        conf.env.append_value('CPPFLAGS', ['-Wall', '-Wextra', '-Wno-unused-parameter'])
        conf.env.append_value('CCFLAGS', ['-std=gnu99', '-pipe'])

def build(bld):
    subdirs = ['tools', 'src']

    if Options.options.with_provider_video == 'ffmpegsource' or Options.options.with_provider_video == 'ffmpegsource':
        subdirs.insert(0, 'libffms')

    if Options.options.with_provider_subtitle == 'libass':
        subdirs.insert(0, 'libass')

    bld.add_subdirs(subdirs)
