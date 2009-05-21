/*
 * Copyright (c) 2004-2008 Mike Matsnev.  All Rights Reserved.
 * 
 * $Id: CoParser.h,v 1.21 2008/03/29 15:41:28 mike Exp $
 * 
 */

#ifndef COPARSER_H
#define	COPARSER_H

// Generic multimedia container parsers support

// random access stream, this will be provided by
// IMMContainer's client
interface __declspec(uuid("8E192E9F-E536-4027-8D46-664CC7A102C5")) IMMStream;
interface IMMStream : public IUnknown {
  // read count bytes starting at position
  STDMETHOD(Read)(	    unsigned long long position,
		  	    void *buffer,
			    unsigned int *count) = 0;

  // scan the file starting at position for the signature
  // signature can't have zero bytes
  STDMETHOD(Scan)(	    unsigned long long *position,
		  	    unsigned int signature) = 0;
};

interface __declspec(uuid("A237C873-C6AD-422E-90DB-7CB4627DCFD9")) IMMStreamOpen;
interface IMMStreamOpen : public IUnknown {
  STDMETHOD(Open)(LPCWSTR name) = 0;
};

interface __declspec(uuid("D8FF7213-6E09-4256-A2E5-5872C798B128")) IMMFrame;
interface IMMFrame : public IMediaSample2 {
  // track number must be the same as returned by
  // IMMContainer->EnumTracks() iterator
  STDMETHOD_(unsigned, GetTrack)() = 0;
  STDMETHOD(SetTrack)(unsigned) = 0;

  STDMETHOD_(unsigned, GetPre)() = 0;
  STDMETHOD(SetPre)(unsigned) = 0;
};

interface __declspec(uuid("B8324E2A-21A9-46A1-8922-70C55D06311A")) IMMErrorInfo;
interface IMMErrorInfo : public IUnknown {
  STDMETHOD(LogError)(BSTR message) = 0;    // message is owned by the caller
  STDMETHOD(LogWarning)(BSTR message) = 0;
};

interface __declspec(uuid("C7120EDB-528C-4ebe-BB53-DA8E70E618EE")) IMemAlloc;
interface IMemAlloc : public IUnknown {
  STDMETHOD(GetBuffer)(HANDLE hAbortEvt, DWORD size, IMMFrame **pS) = 0;
};

// container itself should support IPropertyBag and return
// at least Duration[UI8] (in ns) property
// if a containter supports multiple segments in the same
// physical file it should return SegmentTop[UI8] property
// that return the offset of the first byte after this
// segment's end
interface __declspec(uuid("A369001B-F292-45f7-A942-84F9C8C0718A")) IMMContainer;
interface IMMContainer : public IUnknown {
  STDMETHOD(Open)(	    IMMStream *stream,
			    unsigned long long position,
			    IMMErrorInfo *einfo,
			    IMemAlloc *alloc) = 0;
  STDMETHOD(GetProgress)(unsigned long long *cur,unsigned long long *max) = 0;
  STDMETHOD(AbortOpen)() = 0;

  // pu->Next() returns objects supporting IPropertyBag interface
  STDMETHOD(EnumTracks)(IEnumUnknown **pu) = 0;

  // pu->Next() returns objects supporting IProperyBag and IEnumUnknown interfaces
  STDMETHOD(EnumEditions)(IEnumUnknown **pu) = 0;

  // pu->Next() returns objects supporting IProperyBag and IMMStream interfaces
  STDMETHOD(EnumAttachments)(IEnumUnknown **pu) = 0;

  // S_FALSE is end of stream, S_OK next valid frame returned, E_ABORT wait aborted
  STDMETHOD(ReadFrame)(HANDLE hAbortEvt, IMMFrame **frame) = 0;

  // seeking
  STDMETHOD(Seek)(unsigned long long timecode,unsigned flags) = 0;
};

/* FIXME: duplicated in matroska parser
enum { // Track type
  TT_VIDEO = 1,
  TT_AUDIO = 2,
  TT_SUBS = 17,

  TT_INTERLEAVED = 0x10001,
};
*/

enum { // Seek flags
  MMSF_PREV_KF = 1,
  MMSF_NEXT_KF = 2
};

/* Track properties [IPropertyBag from EnumTracks->Next()]
    Name		    Type      Optional
  DefaultDuration	    UI8		Yes	    in ns
  Video.Interlaced	    BOOL	Yes
  Video.DisplayWidth	    UI4		Yes
  Video.DisplayHeight	    UI4		Yes
  Video.PixelWidth	    UI4		No
  Video.PixelHeight	    UI4		No
  CodecID		    BSTR	No
  Type			    UI4		No	    TT_* enumeration
  CodecPrivate		    ARRAY|UI1	Yes
  Audio.Channels	    UI4		No
  Audio.BitDepth	    UI4		Yes
  Audio.SamplingFreq	    UI4		No
  Audio.OutputSamplingFreq  UI4		Yes
  Language		    BSTR	Yes
  Name			    BSTR	Yes
  FOURCC		    UI4		Yes
*/

#endif