// Copyright (c) 2006, David Lamparter
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


#pragma once


#include <map>
#include <wx/wxprec.h>


class VideoProvider;
class AssFile;

/////////////////////////////////////////
// Subtitle provider (renderer) interface
class SubtitleProvider {
public:
	// Video overlay interface. Renderers MAY implement it,
	// but do not need to. VideoProvider::SetOverlay takes it.
	class Overlay {
	public:
		virtual void SetParams(int width, int height) = 0;
		virtual void Render(wxImage &frame, int ms) = 0;
		virtual void Unbind() = 0;	// Called when VideoProvider is destroyed
		virtual ~Overlay() { };
	};

	// Renderer Class. Manages the different types of renderers.
	// Derivate a class off it, override its Get method and its constructor,
	// and create one single instance of it for your renderer,
	// as a static element in your SubtitleProvider derivated class. Example:
	//	class MyFancyRenderer : public SubtitleProvider {
	//		class MyClass : public Class { public:
	//			MyClass() : Class("FancyRenderer") { };
	//			virtual SubtitleProvider *Get(AssFile *subs) { return new MyFancyRenderer(subs); };
	//		};
	//		static MyClass me;
	//	};
	class Class {
	private:
		static std::map<wxString, SubtitleProvider::Class *> *classes;

	public:
		Class(wxString name);
		virtual SubtitleProvider *Get(AssFile *subs) = 0;
		virtual ~Class() {};

		static SubtitleProvider *GetProvider(wxString provider_name, AssFile *subs);
	};


	virtual ~SubtitleProvider() { };
	virtual void Bind(VideoProvider *vpro) = 0;
};
