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

# Content encodings: a header-stripped track read via a BlockGroup with a
# duration, and an encrypted track which should be reported as unsupported
# without preventing the rest of the file from being read.
def encoding(*children):
 return element('6d80',element('6240',b''.join(children)))
stripped=element('ae',uint_element('d7',1)+uint_element('73c5',1)+uint_element('83',17)+element('86',b'S_TEXT/UTF8')
 +encoding(uint_element('5033',0)+element('5034',uint_element('4254',3)+element('4255',b'hel'))))
encrypted=element('ae',uint_element('d7',2)+uint_element('73c5',2)+uint_element('83',17)+element('86',b'S_TEXT/UTF8')
 +encoding(uint_element('5033',1)+element('5035',b'')))
group=element('a0',element('a1',b'\x81\x00\x00\x00lo')+uint_element('9b',1500))
encrypted_block=element('a3',b'\x82\x00\x00\x80xx')
cluster=element('1f43b675',uint_element('e7',0)+group+encrypted_block)
segment_payload=info+element('1654ae6b',stripped+encrypted)+cluster
(r/'encodings.mkv').write_bytes(header+element('18538067',segment_payload))

# Tracks repeated as live muxers do, a zlib-compressed CodecPrivate with
# uncompressed frames (scope 2), and attachments without UIDs or data
import zlib
entry=element('ae',uint_element('d7',1)+uint_element('73c5',7)+uint_element('83',17)+element('86',b'S_TEXT/UTF8')
 +element('63a2',zlib.compress(b'private data'))
 +encoding(uint_element('5032',2)+uint_element('5033',0)+element('5034',uint_element('4254',0))))
tracks=element('1654ae6b',entry)
def attached(name,data=None):
 return element('61a7',element('466e',name)+element('4660',b'text/plain')+(element('465c',data) if data is not None else b''))
attachments=element('1941a469',attached(b'a.txt',b'aaa')+attached(b'b.txt',b'bb')+attached(b'c.txt'))
cluster=element('1f43b675',uint_element('e7',0)+element('a3',b'\x81\x00\x00\x80hi'))
(r/'repeated-tracks.mkv').write_bytes(header+element('18538067',info+tracks+tracks+attachments+cluster))

# A block timestamp which overflows int64 once the block's relative timestamp
# is added to the cluster's
entry=element('ae',uint_element('d7',1)+uint_element('73c5',1)+uint_element('83',17)+element('86',b'S_TEXT/UTF8'))
cluster=element('1f43b675',uint_element('e7',(1<<63)-1)+element('a3',b'\x81\x00\x01\x80x'))
(r/'timestamp-overflow.mkv').write_bytes(header+element('18538067',info+element('1654ae6b',entry)+cluster))

# Timing: a Void before the Segment, a first block at 10s which all block
# timestamps are made relative to, and a subtitle track with a (deprecated)
# TrackTimecodeScale of 2
import struct
video=element('ae',uint_element('d7',1)+uint_element('73c5',1)+uint_element('83',1)+element('86',b'V_TEST'))
scaled=element('ae',uint_element('d7',2)+uint_element('73c5',2)+uint_element('83',17)+element('86',b'S_TEXT/UTF8')
 +element('23314f',struct.pack('>d',2.0)))
group=element('a0',element('a1',b'\x82\x01\xf4\x00sub')+uint_element('9b',1000))
cluster=element('1f43b675',uint_element('e7',10000)+element('a3',b'\x81\x00\x00\x80v')+group)
(r/'timing.mkv').write_bytes(header+element('ec',bytes(14))+element('18538067',info+element('1654ae6b',video+scaled)+cluster))

# A Void followed by CRC-prefixed Info and Tracks, as mkvmerge and ffmpeg
# write. Losing the Info would show as the second packet being at 4ms rather
# than 2ms.
crc=element('bf',bytes(4))
entry=element('ae',uint_element('d7',1)+uint_element('73c5',1)+uint_element('83',17)+element('86',b'S_TEXT/UTF8'))
void_info=element('1549a966',crc+uint_element('2ad7b1',500000))
clusters=element('1f43b675',uint_element('e7',0)+element('a3',b'\x81\x00\x00\x80a'))+element('1f43b675',uint_element('e7',4)+element('a3',b'\x81\x00\x00\x80b'))
(r/'void-crc.mkv').write_bytes(header+element('18538067',element('ec',bytes(6))+void_info+element('ec',bytes(6))+element('1654ae6b',crc+entry)+clusters))

# The start of the file is the earliest audio or video timestamp, ignoring a
# subtitle line before it
audio=element('ae',uint_element('d7',2)+uint_element('73c5',2)+uint_element('83',2)+element('86',b'A_TEST'))
subtitle=element('ae',uint_element('d7',3)+uint_element('73c5',3)+uint_element('83',17)+element('86',b'S_TEXT/UTF8'))
early=element('a0',element('a1',b'\x83\xfe\x0c\x00early')+uint_element('9b',1000))
cluster=element('1f43b675',uint_element('e7',1000)+early+element('a3',b'\x82\x00\x00\x80a')+element('a3',b'\x81\x00\x53\x80v'))
(r/'start-time.mkv').write_bytes(header+element('18538067',info+element('1654ae6b',video+audio+subtitle)+cluster))
