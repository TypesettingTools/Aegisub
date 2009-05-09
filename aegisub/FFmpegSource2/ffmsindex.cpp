//  Copyright (c) 2008-2009 Karl Blomster
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//  THE SOFTWARE.


#include <iostream> 
#include <string>
#include <stdlib.h>
#include "ffms.h"

int TrackMask;
int DumpMask;
bool Overwrite;
bool IgnoreErrors;
bool Verbose;
std::string InputFile;
std::string CacheFile;
std::string AudioFile;

FrameIndex *Index;


void PrintUsage () {
	using namespace std;
	cout << "FFmpegSource2 indexing app" << endl
		<< "Usage: ffmsindex [options] inputfile [outputfile]" << endl
		<< "If no output filename is specified, inputfile.ffindex will be used." << endl << endl
		<< "Options:" << endl
		<< "-f        Force overwriting of existing index file, if any (default: no)" << endl
		<< "-s        Silently skip indexing of audio tracks that cannot be read (default: no)" << endl
		<< "-v        Be verbose; i.e. print FFmpeg warnings/diagnostics, if any (default: no)" << endl
		<< "-t N      Set the audio indexing mask to N (-1 means index all tracks, 0 means index none, default: 0)" << endl
		<< "-d N      Set the audio decoding mask to N (mask syntax same as -t, default: 0)" << endl
		<< "-a NAME   Set the audio output base filename to NAME (default: input filename)";
}


void ParseCMDLine (int argc, char *argv[]) {
	if (argc <= 1) {
		PrintUsage();
		throw "";
	}

	// defaults
	InputFile = "";
	CacheFile = "";
	AudioFile = "";
	TrackMask = 0;
	DumpMask  = 0;
	Overwrite = false;
	IgnoreErrors = false;
	Verbose = false;

	// argv[0] = name of program
	int i = 1;

	while (i < argc) {
		std::string Option = argv[i];
		std::string OptionArg = "";
		if (i+1 < argc)
			OptionArg = argv[i+1];

		if (!Option.compare("-f")) {
			Overwrite = true;
		} else if (!Option.compare("-s")) {
			IgnoreErrors = true;
		} else if (!Option.compare("-v")) {
			Verbose = true;
		} else if (!Option.compare("-t")) {
			TrackMask = atoi(OptionArg.c_str());
			i++;
		} else if (!Option.compare("-d")) {
			DumpMask = atoi(OptionArg.c_str());
			i++;
		} else if (!Option.compare("-a")) {
			AudioFile = OptionArg;
			i++;
		} else if (InputFile.empty()) {
			InputFile = argv[i];
		} else if (CacheFile.empty()) {
			CacheFile = argv[i];
		} else {
			std::cout << "Warning: ignoring unknown option " << argv[i] << std::endl;
		}

		i++;
	}

	if (InputFile.empty()) {
		throw "Error: no input file specified";
	}
	if (CacheFile.empty()) {
		CacheFile = InputFile;
		CacheFile.append(".ffindex");
	}
	if (AudioFile.empty()) {
		AudioFile = InputFile;
	}
}


static int FFMS_CC UpdateProgress(int State, int64_t Current, int64_t Total, void *Private) {
	using namespace std;
	int *LastPercentage = (int *)Private;
	int Percentage = int((double(Current)/double(Total)) * 100);

	if (Percentage <= *LastPercentage)
		return 0;

	*LastPercentage = Percentage;

	/*if (Percentage < 10)
		cout << "\b\b";
	else
		cout << "\b\b\b"; */

	cout << "Indexing, please wait... " << Percentage << "% \r" << flush;
	
	return 0;
}


void DoIndexing () {
	char FFMSErrMsg[1024];
	int MsgSize = sizeof(FFMSErrMsg);
	int Progress = 0;

	Index = FFMS_ReadIndex(CacheFile.c_str(), FFMSErrMsg, MsgSize);
	if (Overwrite || Index == NULL) {
		std::cout << "Indexing, please wait... 0% \r" << std::flush;
		Index = FFMS_MakeIndex(InputFile.c_str(), TrackMask, DumpMask, AudioFile.c_str(), IgnoreErrors, UpdateProgress, &Progress, FFMSErrMsg, MsgSize);
		if (Index == NULL) {
			std::string Err = "\nIndexing error: ";
			Err.append(FFMSErrMsg);
			throw Err;
		}

		if (Progress != 100)
			std::cout << "Indexing, please wait... 100% \r" << std::flush;
		
		std::cout << std::endl << "Writing index... ";

		if (FFMS_WriteIndex(CacheFile.c_str(), Index, FFMSErrMsg, MsgSize)) {
			std::string Err = "Error writing index: ";
			Err.append(FFMSErrMsg);
			throw Err;
		}

		std::cout << "done." << std::endl << std::flush;
	} else {
		throw "Error: index file already exists, use -f if you are sure you want to overwrite it.";
	}
}


int main(int argc, char *argv[]) {
	try {
		ParseCMDLine(argc, argv);
	} catch (const char *Error) {
		std::cout << std::endl << Error << std::endl;
		return 1;
	} catch (std::string Error) {
		std::cout << std::endl << Error << std::endl;
		return 1;
	} catch (...) {
		std::cout << std::endl << "Unknown error" << std::endl;
		return 1;
	}

	FFMS_Init();

	if (!Verbose)
		FFMS_NoLog();

	try {
		DoIndexing();
	} catch (const char *Error) {
		std::cout << Error << std::endl;
		FFMS_DestroyFrameIndex(Index);
		return 1;
	} catch (std::string Error) {
		std::cout << std::endl << Error << std::endl;
		FFMS_DestroyFrameIndex(Index);
		return 1;
	} catch (...) {
		std::cout << std::endl << "Unknown error" << std::endl;
		FFMS_DestroyFrameIndex(Index);
		return 1;
	}

	FFMS_DestroyFrameIndex(Index);
	return 0;
}
