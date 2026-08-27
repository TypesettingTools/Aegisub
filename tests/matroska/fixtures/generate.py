#!/usr/bin/env python3
"""Regenerate the small, deterministic project-owned parity fixture."""
import pathlib,subprocess
r=pathlib.Path(__file__).parent
subprocess.run(['ffmpeg','-hide_banner','-loglevel','error','-y','-fflags','+bitexact','-f','lavfi','-i','color=size=16x16:rate=1:duration=3','-i',r/'subtitle.ass','-i',r/'utf8.srt','-map','0:v','-map','1','-map','2','-c:v','ffv1','-c:s:0','ass','-c:s:1','srt','-metadata:s:s:0','language=eng','-metadata:s:s:0','title=ASS track','-metadata:s:s:1','language=jpn','-attach',r/'attachment.txt','-metadata:s:t','mimetype=text/plain','-metadata:s:t','filename=attachment.txt','-map_metadata','-1',r/'subtitle-attachment.mkv'],check=True)
subprocess.run(['mkvmerge','-o',r/'compressed-zlib.mkv','--compression','0:zlib',r/'utf8.srt'],check=True)
opus=r/'audio-only.opus'
subprocess.run(['ffmpeg','-hide_banner','-loglevel','error','-y','-fflags','+bitexact','-f','lavfi','-i','sine=frequency=440:duration=0.25','-c:a','libopus',opus],check=True)
subprocess.run(['mkvmerge','-o',r/'audio-only-opus.mka',opus],check=True)
opus.unlink()
subprocess.run(['ffmpeg','-hide_banner','-loglevel','error','-y','-fflags','+bitexact','-f','lavfi','-i','color=size=16x16:rate=1:duration=1','-map_metadata','-1','-c:v','ffv1',r/'video-only.mkv'],check=True)
