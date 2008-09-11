//  Copyright (c) 2008 Karl Blomster
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


#include "ffmsindex.h"

int main(int argc, char *argv[]) {
	IndexingOptions *Options = FFMSIndexApp::ParseCMDLine(argc, argv);
	if (!Options)
		return 1;

	FFMS_Init();

	if (FFMSIndexApp::DoIndexing(Options))
		return 1;
	else
		return 0;
}


IndexingOptions *FFMSIndexApp::ParseCMDLine (int argc, char *argv[]) {
	if (argc <= 1) {
		PrintUsage();
		return NULL;
	}

	// defaults
	IndexingOptions *Options = new IndexingOptions();
	Options->InputFile = "";
	Options->CacheFile = "";
	Options->AudioFile = "";
	Options->TrackMask = 0;
	Options->Overwrite = false;

	// argv[0] = name of program
	int i = 1;

	while (i < argc) {
		std::string Option = argv[i];
		std::string OptionArg = "";
		if (i+1 < argc)
			OptionArg = argv[i+1];

		if (!Option.compare("-f")) {
			Options->Overwrite = true;
		} else if (!Option.compare("-t")) {
			Options->TrackMask = atoi(OptionArg.c_str());
			i++;
		} else if (!Option.compare("-a")) {
			Options->AudioFile = OptionArg;
			i++;
		} else if (!Options->InputFile.compare("")) {
			Options->InputFile = argv[i];
		} else if (!Options->CacheFile.compare("")) {
			Options->CacheFile = argv[i];
		} else {
			std::cout << "Warning: ignoring unknown option " << argv[i] << std::endl;
		}

		i++;
	}

	if (!Options->InputFile.compare("")) {
		std::cout << "Error: no input file specified" << std::endl;
		return NULL;
	}
	if (!Options->CacheFile.compare("")) {
		Options->CacheFile = Options->InputFile;
		Options->CacheFile.append(".ffindex");
	}
	if (!Options->AudioFile.compare("")) {
		Options->AudioFile = Options->InputFile;
	}

	return Options;
}


void FFMSIndexApp::PrintUsage () {
	using namespace std;
	cout << "FFmpegSource2 indexing app" << endl
		<< "Usage: ffmsindex [options] inputfile [outputfile]" << endl
		<< "If no output filename is specified, inputfile.ffindex will be used." << endl << endl
		<< "Options:" << endl
		<< "-f        Overwrite existing index file if it exists (default: no)" << endl
		<< "-t N      Set the audio trackmask to N (-1 means decode all tracks, 0 means decode none; default: 0)" << endl
		<< "-a NAME   Set the audio output base filename to NAME (default: input filename)" << endl;
}


int FFMSIndexApp::DoIndexing (IndexingOptions *Options) {
	FrameIndex *Index;
	char FFMSErrMsg[1024];
	int MsgSize = sizeof(FFMSErrMsg);
	int TrackMask = Options->TrackMask;
	std::string InputFileName = Options->InputFile;
	std::string CacheFileName = Options->CacheFile;
	std::string AudioFileName = Options->AudioFile;
	int Progress = 0;

	Index = FFMS_ReadIndex(CacheFileName.c_str(), FFMSErrMsg, MsgSize);
	if (Options->Overwrite || Index == NULL) {
		std::cout << "Indexing, please wait...  0%";
		Index = FFMS_MakeIndex(InputFileName.c_str(), TrackMask, AudioFileName.c_str(), FFMSIndexApp::UpdateProgress, &Progress, FFMSErrMsg, MsgSize);
		if (Index == NULL) {
			std::cout << std::endl << "Indexing error: " << FFMSErrMsg << std::endl;
			return 1;
		}

		std::cout << "\b\b\b100%" << std::endl << "Writing index... ";

		if (FFMS_WriteIndex(CacheFileName.c_str(), Index, FFMSErrMsg, MsgSize)) {
			std::cout << std::endl << "Error writing index: " << FFMSErrMsg << std::endl;
			return 1;
		}

		std::cout << "done." << std::endl;
	} else {
		std::cout << "Error: index file already exists, use -f if you are sure you want to overwrite it." << std::endl;
		return 1;
	}

	return 0;
}

int FFMSIndexApp::UpdateProgress(int State, int64_t Current, int64_t Total, void *Private) {
	using namespace std;
	int *LastPercentage = (int *)Private;
	int Percentage = int((double(Current)/double(Total)) * 100);

	if (Percentage <= *LastPercentage)
		return 0;

	*LastPercentage = Percentage;

	if (Percentage < 10)
		cout << "\b\b";
	else
		cout << "\b\b\b";

	cout << Percentage << "%";
	
	return 0;
}