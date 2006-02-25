// Copyright (c) 2005, Rodrigo Braz Monteiro
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of the Aegisub Group nor the names of its contributors
//     may be used to endorse or promote products derived from this software
//     without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// -----------------------------------------------------------------------------
//
// AEGISUB
//
// Website: http://aegisub.cellosoft.com
// Contact: mailto:zeratul@cellosoft.com
//


///////////
// Headers
#include <wx/wxprec.h>
#include <wx/filename.h>
#include <Mmreg.h>
#include "avisynth_wrap.h"
#include "utils.h"
#include "audio_provider.h"
#include "video_display.h"
#include "options.h"
#include "audio_display.h"
#include "main.h"
#include "dialog_progress.h"

extern "C" {
#include <portaudio.h>
}

int AudioProvider::pa_refcount = 0;

#define CacheBits ((22))
#define CacheBlockSize ((1 << CacheBits))

//////////////
// Constructor
AudioProvider::AudioProvider(wxString _filename, AudioDisplay *_display) {
	type = AUDIO_PROVIDER_NONE;
	playing = false;
	stopping = false;
	blockcache = NULL;
	blockcount = 0;
	raw = NULL;
	display = _display;
	blockcount = 0;
	volume = 1.0f;

	filename = _filename;

	// Initialize portaudio
	if (!pa_refcount) {
		PaError err = Pa_Initialize();
		if (err != paNoError)
			throw wxString::Format(_T("Failed opening PortAudio with error: %s"), wxString(Pa_GetErrorText(err),wxConvLocal));
		pa_refcount++;
	}

	try {
		OpenAVSAudio();
		OpenStream();
	} catch (...) {
		Unload();
		throw;
	}
}


//////////////
// Destructor
AudioProvider::~AudioProvider() {
	CloseStream();
	Unload();
}


////////////////
// Unload audio
void AudioProvider::Unload() {
	// Clean up avisynth
	clip = NULL;

	// Close file
	if (type == AUDIO_PROVIDER_DISK_CACHE) {
		file_cache.close();
		wxRemoveFile(DiskCacheName());
	}

	// Free ram cache
	if (blockcache) {
		for (int i = 0; i < blockcount; i++)
			if (blockcache[i])
				delete blockcache[i];
		delete blockcache;
	}

	// Clear buffers
	delete raw;

	if (!--pa_refcount)
		Pa_Terminate();
}


////////////////////////////
// Load audio from avisynth
void AudioProvider::OpenAVSAudio() {
	// Set variables
	type = AUDIO_PROVIDER_AVS;
	AVSValue script;

	// Prepare avisynth
	wxMutexLocker lock(AviSynthMutex);

	try {
		const char * argnames[3] = { 0, "video", "audio" };
		AVSValue args[3] = { env->SaveString(filename.mb_str(wxConvLocal)), false, true };
		script = env->Invoke("DirectShowSource", AVSValue(args,3),argnames);

		LoadFromClip(script);

	} catch (AvisynthError &err) {
		throw wxString::Format(_T("AviSynth error: %s"), wxString(err.msg,wxConvLocal));
	}
}


/////////////////////////
// Read from environment
void AudioProvider::LoadFromClip(AVSValue _clip) {	
	// Prepare avisynth
	AVSValue script;

	// Check if it has audio
	VideoInfo vi = _clip.AsClip()->GetVideoInfo();
	if (!vi.HasAudio()) throw wxString(_T("No audio found."));

	// Convert to one channel
	char buffer[1024];
	strcpy(buffer,Options.AsText(_T("Audio Downmixer")).mb_str(wxConvLocal));
	script = env->Invoke(buffer, _clip);

	// Convert to 16 bits per sample
	script = env->Invoke("ConvertAudioTo16bit", script);

	// Convert sample rate
	int setsample = Options.AsInt(_T("Audio Sample Rate"));
	if (setsample != 0) {
		AVSValue args[2] = { script, setsample };
		script = env->Invoke("ResampleAudio", AVSValue(args,2));
	}

	// Set clip
	PClip tempclip = script.AsClip();
	vi = tempclip->GetVideoInfo();

	// Read properties
	channels = vi.AudioChannels();
	num_samples = vi.num_audio_samples;
	sample_rate = vi.SamplesPerSecond();
	bytes_per_sample = vi.BytesPerAudioSample();

	// Read whole thing into ram cache
	if (Options.AsInt(_T("Audio Cache")) == 1) {
		ConvertToRAMCache(tempclip);
		clip = NULL;
	}

	// Disk cache
	else if (Options.AsInt(_T("Audio Cache")) == 2) {
		ConvertToDiskCache(tempclip);
		clip = NULL;
	}

	// Assign to avisynth
	else {
		clip = tempclip;
	}
}


/////////////
// RAM Cache
void AudioProvider::ConvertToRAMCache(PClip &tempclip) {

	// Allocate cache
	__int64 ssize = num_samples * bytes_per_sample;
	blockcount = (ssize + CacheBlockSize - 1) >> CacheBits;

	blockcache = new char*[blockcount];
	for (int i = 0; i < blockcount; i++)
		blockcache[i] = NULL;

	try {
		for (int i = 0; i < blockcount; i++)
			blockcache[i] = new char[MIN(CacheBlockSize,ssize-i*CacheBlockSize)];
	} catch (...) { 
		for (int i = 0; i < blockcount; i++)
			delete blockcache[i];
		delete blockcache;
			
		blockcache = NULL;
		blockcount = 0;

		if (wxMessageBox(_("Not enough ram available. Use disk cache instead?"),_("Audio Information"),wxICON_INFORMATION | wxYES_NO) == wxYES) {
			ConvertToDiskCache(tempclip);
			return;
		} else
			throw wxString(_T("Couldn't open audio, not enough ram available."));
	}

	// Start progress
	volatile bool canceled = false;
	DialogProgress *progress = new DialogProgress(NULL,_("Load audio"),&canceled,_("Reading into RAM"),0,num_samples);
	progress->Show();
	progress->SetProgress(0,1);

	// Read cache
	int readsize = CacheBlockSize / bytes_per_sample;

	for (int i=0;i<blockcount && !canceled; i++) {
		tempclip->GetAudio((char*)blockcache[i],i*readsize, i == blockcount-1 ? (num_samples - i*readsize) : readsize,env);
		progress->SetProgress(i,blockcount-1);
	}

	type = AUDIO_PROVIDER_CACHE;

	// Clean up progress
	if (!canceled) 
		progress->Destroy();
	else
		throw wxString(_T("Audio loading cancelled by user"));
}


//////////////
// Disk Cache
void AudioProvider::ConvertToDiskCache(PClip &tempclip) {
	// Check free space
	wxLongLong freespace;
	if (wxGetDiskSpace(DiskCachePath(), NULL, &freespace))
		if (num_samples * channels * bytes_per_sample > freespace)
			throw wxString(_T("Not enough free diskspace in "))+DiskCachePath()+wxString(_T(" to cache the audio"));

	// Open output file
	std::ofstream file;
	char filename[512];
	strcpy(filename,DiskCacheName().mb_str(wxConvLocal));
	file.open(filename,std::ios::binary | std::ios::out | std::ios::trunc);

	// Start progress
	volatile bool canceled = false;
	DialogProgress *progress = new DialogProgress(NULL,_T("Load audio"),&canceled,_T("Reading to Hard Disk cache"),0,num_samples);
	progress->Show();

	// Write to disk
	int block = 4096;
	char *temp = new char[block * channels * bytes_per_sample];
	for (__int64 i=0;i<num_samples && !canceled; i+=block) {
		if (block+i > num_samples) block = num_samples - i;
		tempclip->GetAudio(temp,i,block,env);
		file.write(temp,block * channels * bytes_per_sample);
		progress->SetProgress(i,num_samples);
	}
	file.close();
	type = AUDIO_PROVIDER_DISK_CACHE;

	// Finish
	if (!canceled) {
		progress->Destroy();
		file_cache.open(filename,std::ios::binary | std::ios::in);
	}
	else 
		throw wxString(_T("Audio loading cancelled by user"));
}

////////////////
// Get filename
wxString AudioProvider::GetFilename() {
	return filename;
}

int aaa = 0;

/////////////
// Get audio
void AudioProvider::GetAudio(void *buf, __int64 start, __int64 count) {

	// Requested beyond the length of audio
	if (start+count > num_samples) {
		__int64 oldcount = count;
		count = num_samples-start;
		if (count < 0) count = 0;

		// Fill beyond with zero
		if (bytes_per_sample == 1) {
			char *temp = (char *) buf;
			for (int i=count;i<oldcount;i++) {
				temp[i] = 0;
			}
		}
		if (bytes_per_sample == 2) {
			short *temp = (short *) buf;
			for (int i=count;i<oldcount;i++) {
				temp[i] = 0;
			}
		}
	}

	if (count) {
		char *charbuf = (char *)buf;

		// RAM Cache
		if (type == AUDIO_PROVIDER_CACHE) {
			int i = (start*bytes_per_sample) >> CacheBits;
			int start_offset = (start*bytes_per_sample) & (CacheBlockSize-1);

			__int64 bytesremaining = count*bytes_per_sample;
			
			while (bytesremaining) {
				int readsize=MIN(bytesremaining,CacheBlockSize); 
				readsize = MIN(readsize,CacheBlockSize - start_offset);

				memcpy(charbuf,(char *)(blockcache[i++]+start_offset),readsize);

				charbuf+=readsize;

				start_offset=0;
				bytesremaining-=readsize;
			}
		}

		// Disk cache
		else if (type == AUDIO_PROVIDER_DISK_CACHE) {
			wxMutexLocker disklock(diskmutex);
			file_cache.seekg(start*bytes_per_sample);
			file_cache.read((char*)buf,count*bytes_per_sample*channels);
		}

		// Avisynth
		else {
			wxMutexLocker disklock(diskmutex);
			clip->GetAudio(buf,start,count,env);
		}
	}
}


//////////////////////////
// Get number of channels
int AudioProvider::GetChannels() {
	return channels;
}


//////////////////////////
// Get number of samples
__int64 AudioProvider::GetNumSamples() {
	return num_samples;
}


///////////////////
// Get sample rate
int AudioProvider::GetSampleRate() {
	return sample_rate;
}


////////////////
// Get waveform
void AudioProvider::GetWaveForm(int *min,int *peak,__int64 start,int w,int h,int samples,float scale) {
	// Setup
	int channels = GetChannels();
	int n = w * samples;
	for (int i=0;i<w;i++) {
		peak[i] = 0;
		min[i] = h;
	}

	// Prepare waveform
	int cur;
	int curvalue;

	// Prepare buffers
	int needLen = n*channels*bytes_per_sample;
	if (raw) {
		if (raw_len < needLen) {
			delete raw;
			raw = NULL;
		}
	}
	if (!raw) {
		raw_len = needLen;
		raw = (void*) new char[raw_len];
	}

	if (bytes_per_sample == 1) {
		// Read raw samples
		unsigned char *raw_char = (unsigned char*) raw;
		GetAudio(raw,start,n);
		int amplitude = h*scale;

		// Calculate waveform
		for (int i=0;i<n;i++) {
			cur = i/samples;
			curvalue = h - (int(raw_char[i*channels])*amplitude)/0xFF;
			if (curvalue > h) curvalue = h;
			if (curvalue < 0) curvalue = 0;
			if (curvalue < min[cur]) min[cur] = curvalue;
			if (curvalue > peak[cur]) peak[cur] = curvalue;
		}
	}

	if (bytes_per_sample == 2) {
		// Read raw samples
		short *raw_short = (short*) raw;
		GetAudio(raw,start,n);
		int half_h = h/2;
		int half_amplitude = half_h * scale;

		// Calculate waveform
		for (int i=0;i<n;i++) {
			cur = i/samples;
			curvalue = half_h - (int(raw_short[i*channels])*half_amplitude)/0x8000;
			if (curvalue > h) curvalue = h;
			if (curvalue < 0) curvalue = 0;
			if (curvalue < min[cur]) min[cur] = curvalue;
			if (curvalue > peak[cur]) peak[cur] = curvalue;
		}
	}
}


///////////////////////////
// Get disk cache path
wxString AudioProvider::DiskCachePath() {
	return AegisubApp::folderName;
}


///////////////////////////
// Get disk cache filename
wxString AudioProvider::DiskCacheName() {
	return DiskCachePath() + _T("audio.tmp");
}


//////////////////////
// PortAudio callback
int paCallback(void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer, PaTimestamp outTime, void *userData) {
	// Get provider
	AudioProvider *provider = (AudioProvider *) userData;
	int end = 0;

	// Calculate how much left
	__int64 lenAvailable = provider->endPos - provider->playPos;
	unsigned __int64 avail = 0;
	if (lenAvailable > 0) {
		avail = lenAvailable;
		if (avail > framesPerBuffer) {
			lenAvailable = framesPerBuffer;
			avail = lenAvailable;
		}
	}
	else {
		lenAvailable = 0;
		avail = 0;
	}

	// Play something
	if (lenAvailable > 0) {
		provider->GetAudio(outputBuffer,provider->playPos,lenAvailable);
	}

	// Pad end with blank
	if (avail < (unsigned __int64) framesPerBuffer) {
		provider->softStop = true;
	}

	// Set volume
	short *output = (short*) outputBuffer;
	for (unsigned int i=0;i<avail;i++) output[i] = MID(-(1<<15),int(output[i] * provider->volume),(1<<15)-1);

	// Fill rest with blank
	for (unsigned int i=avail;i<framesPerBuffer;i++) output[i]=0;

	// Set play position (and real one)
	provider->playPos += framesPerBuffer;
	provider->realPlayPos = provider->playPos - (outTime - Pa_StreamTime(provider->stream));

	// Cap to start if lower
	return end;
}


////////
// Play
void AudioProvider::Play(__int64 start,__int64 count) {
	// Stop if it's already playing
	wxMutexLocker locker(PAMutex);

	// Set values
	endPos = start + count;
	realPlayPos = start;
	playPos = start;
	startPos = start;
	startMS = startPos * 1000 / GetSampleRate();

	// Start playing
	if (!playing) {
		PaError err = Pa_StartStream(stream);
		if (err != paNoError) {
			return;
		}
	}

	playing = true;

	if (!display->UpdateTimer.IsRunning())	display->UpdateTimer.Start(15);
}


////////
// Stop
void AudioProvider::Stop(bool timerToo) {
	//wxMutexLocker locker(PAMutex);
	softStop = false;

	// Stop stream
	playing = false;
	Pa_StopStream (stream);
	realPlayPos = 0;

	// Stop timer
	if (timerToo) {
		display->UpdateTimer.Stop();
	}
}


///////////////
// Open stream
void AudioProvider::OpenStream() {
	// Open stream
	PaError err = Pa_OpenDefaultStream(&stream,0,GetChannels(),paInt16,GetSampleRate(),256,16,paCallback,this);
	if (err != paNoError)
		throw wxString(_T("Failed initializing PortAudio stream with error: ") + wxString(Pa_GetErrorText(err),wxConvLocal));
}


///////////////
// Close stream
void AudioProvider::CloseStream() {
	try {
		Stop(false);
		Pa_CloseStream(stream);
	} catch (...) {}
}
