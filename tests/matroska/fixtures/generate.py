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

def element(element_id, payload):
 length=len(payload)
 size_length=next(n for n in range(1,9) if length < (1 << (7*n))-1)
 encoded=(length | (1 << (7*size_length))).to_bytes(size_length,'big')
 return bytes.fromhex(element_id)+encoded+payload
def uint_element(element_id, value):
 size=max(1,(value.bit_length()+7)//8)
 return element(element_id,value.to_bytes(size,'big'))

source=(r/'video-only.mkv').read_bytes()
header_size=4+(source[4]&0x7f)+1
header=source[:header_size]
info=element('1549a966',uint_element('2ad7b1',1000000))
block=element('a3',b'\x41\x01\x00\x00\x80hello')
cluster=element('1f43b675',uint_element('e7',0)+block)
entry=element('ae',uint_element('d7',257)+uint_element('73c5',257)+uint_element('83',17)+element('86',b'S_TEXT/UTF8'))
tracks=element('1654ae6b',entry)
seek_entry=element('4dbb',element('53ab',bytes.fromhex('1654ae6b'))+uint_element('53ac',0))
seek_head=element('114d9b74',seek_entry)
tracks_position=len(seek_head)+len(info)+len(cluster)
seek_entry=element('4dbb',element('53ab',bytes.fromhex('1654ae6b'))+uint_element('53ac',tracks_position))
seek_head=element('114d9b74',seek_entry)
segment_payload=seek_head+info+cluster+tracks
(r/'tracks-after-cluster.mkv').write_bytes(header+element('18538067',segment_payload))
