// Copyright (c) 2016, Thomas Goyne <plorkyeran@aegisub.org>
//
// Permission to use, copy, modify, and distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//
// Aegisub Project http://www.aegisub.org/

#include "font_file_lister.h"

#include <libaegisub/charset_conv_win.h>

#include <dwrite.h>
#include <wchar.h>
#include <windowsx.h>

#ifdef HAVE_DWRITE_3
#include <dwrite_3.h>
#endif

/// @brief Normalize the case of a file path.
/// @param path The path to be normalized. It can be a directory or a file.
/// @return A string representing the normalized path.
///         If the path normalization fails due to file handling errors or other issues,
///         an empty string is returned.
/// @example For "C:\WINDOWS\FONTS\ARIAL.TTF", it would return "C:\Windows\Fonts\arial.ttf"
std::wstring normalizeFilePathCase(std::wstring const& path) {
	/* FILE_FLAG_BACKUP_SEMANTICS is required to open a directory */
	HANDLE hfile = CreateFile(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, nullptr);
	if (hfile == INVALID_HANDLE_VALUE)
		return L"";
	agi::scoped_holder<HANDLE> hfile_sh(hfile, [](HANDLE hfile) { CloseHandle(hfile); });

	DWORD normalized_path_length = GetFinalPathNameByHandle(hfile_sh, nullptr, 0, FILE_NAME_NORMALIZED);
	if (!normalized_path_length)
		return L"";

	agi::scoped_holder<WCHAR*> normalized_path_sh(new WCHAR[normalized_path_length + 1], [](WCHAR* p) { delete[] p; });
	if (!GetFinalPathNameByHandle(hfile_sh, normalized_path_sh, normalized_path_length + 1, FILE_NAME_NORMALIZED))
		return L"";

	std::wstring normalized_path(normalized_path_sh);

	// GetFinalPathNameByHandle returns the path in "device path" form. Ex: "\\?\C:\Windows\Fonts\ariali.ttf"
	// We need to convert it to a "fully qualified DOS Path". Ex: "C:\Windows\Fonts\ariali.ttf"
	// There isn't any public API that removes the prefix (there is RtlNtPathNameToDosPathName, but it is really hacky to use it)
	// See: https://stackoverflow.com/questions/31439011/getfinalpathnamebyhandle-result-without-prepended
	// Even CPython removes the prefix manually: https://github.com/python/cpython/blob/963904335e579bfe39101adf3fd6a0cf705975ff/Lib/ntpath.py#L733-L793
	// Gecko: https://github.com/mozilla/gecko-dev/blob/6032a565e3be7dcdd01e4fe26791c84f9222a2e0/widget/windows/WinUtils.cpp#L1577-L1584
	if (normalized_path.compare(0, 7, L"\\\\?\\UNC") == 0)
		normalized_path.erase(2, 6);
	else if (normalized_path.compare(0, 4, L"\\\\?\\") == 0)
		normalized_path.erase(0, 4);

	return normalized_path;
}

GdiFontFileLister::GdiFontFileLister(FontCollectorStatusCallback &)
: dwrite_factory_sh(nullptr, [](IDWriteFactory* p) { p->Release(); })
, font_collection_sh(nullptr, [](IDWriteFontCollection* p) { p->Release(); })
, dc_sh(nullptr, [](HDC dc) { DeleteDC(dc); })
, gdi_interop_sh(nullptr, [](IDWriteGdiInterop* p) { p->Release(); })
{
	IDWriteFactory* dwrite_factory;
	if (FAILED(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&dwrite_factory))))
		throw agi::EnvironmentError("Failed to initialize the DirectWrite Factory");
	dwrite_factory_sh = dwrite_factory;

	IDWriteFontCollection* font_collection;
	if (FAILED(dwrite_factory_sh->GetSystemFontCollection(&font_collection, true)))
		throw agi::EnvironmentError("Failed to initialize the system font collection");
	font_collection_sh = font_collection;

	HDC dc = CreateCompatibleDC(nullptr);
	if (dc == nullptr)
		throw agi::EnvironmentError("Failed to initialize the HDC");
	dc_sh = dc;

	IDWriteGdiInterop* gdi_interop;
	if (FAILED(dwrite_factory_sh->GetGdiInterop(&gdi_interop)))
		throw agi::EnvironmentError("Failed to initialize the Gdi Interop");
	gdi_interop_sh = gdi_interop;
}

CollectionResult GdiFontFileLister::GetFontPaths(std::string const& facename, int bold, bool italic, std::vector<int> const& characters) {
	CollectionResult ret;

	int weight = bold == 0 ? 400 :
				 bold == 1 ? 700 :
				 bold;

	// From VSFilter
	//   - https://sourceforge.net/p/guliverkli2/code/HEAD/tree/src/subtitles/RTS.cpp#l45
	//   - https://sourceforge.net/p/guliverkli2/code/HEAD/tree/src/subtitles/STS.cpp#l2992
	LOGFONTW lf{};
	lf.lfCharSet = DEFAULT_CHARSET; // FIXME: Note that this currently ignores the font encoding specified in the ass file.
	wcsncpy_s(lf.lfFaceName, LF_FACESIZE, agi::charset::ConvertW(facename).c_str(), _TRUNCATE);
	lf.lfItalic = italic ? -1 : 0;
	lf.lfWeight = weight;
	lf.lfOutPrecision = OUT_TT_PRECIS;
	lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	lf.lfQuality = ANTIALIASED_QUALITY;
	lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;

	agi::scoped_holder<HFONT> hfont_sh(CreateFontIndirect(&lf), [](HFONT p) { DeleteObject(p); });
	if (hfont_sh == nullptr)
		return ret;

	SelectFont(dc_sh, hfont_sh);

	std::wstring selected_name(LF_FACESIZE - 1, L'\0');
	if (!GetTextFaceW(dc_sh, LF_FACESIZE, selected_name.data()))
		return ret;

	// If the selected_name is different then the lf.lfFaceName,
	// it means that the requested font doesn't exist.
	if (_wcsnicmp(&selected_name[0], lf.lfFaceName, LF_FACESIZE))
		return ret;

	IDWriteFontFace* font_face;
	if (FAILED(gdi_interop_sh->CreateFontFaceFromHdc(dc_sh, &font_face)))
		return ret;
	agi::scoped_holder<IDWriteFontFace*> font_face_sh(font_face, [](IDWriteFontFace* p) { p->Release(); });

	ret.fake_italic = font_face_sh->GetSimulations() & DWRITE_FONT_SIMULATIONS_OBLIQUE;
	ret.fake_bold = font_face_sh->GetSimulations() & DWRITE_FONT_SIMULATIONS_BOLD;

	bool is_query_font_face_3_succeeded = false;
#ifdef HAVE_DWRITE_3
	// Fonts added via the AddFontResource API are not included in the IDWriteFontCollection.
	// This omission causes GetFontFromFontFace to fail.
	// This issue is unavoidable on Windows 8 or lower.
	// However, on Windows 10 or higher, we address this by querying IDWriteFontFace to IDWriteFontFace3.
	// From this new instance, we can verify font character(s) availability.

	IDWriteFontFace3* font_face_3;
	if (SUCCEEDED(font_face_sh->QueryInterface(__uuidof(IDWriteFontFace3), (void**)&font_face_3))) {
		agi::scoped_holder<IDWriteFontFace3*> font_face_3_sh(font_face_3, [](IDWriteFontFace3* p) { p->Release(); });
		is_query_font_face_3_succeeded = true;

		for (int character : characters) {
			if (!font_face_3_sh->HasCharacter((UINT32)character)) {
				ret.missing += character;
			}
		}
	}
#endif

	if (!is_query_font_face_3_succeeded) {
		IDWriteFont* font;
		if (FAILED(font_collection_sh->GetFontFromFontFace(font_face_sh, &font)))
			return ret;
		agi::scoped_holder<IDWriteFont*> font_sh(font, [](IDWriteFont* p) { p->Release(); });

		BOOL exists;
		HRESULT hr;
		for (int character : characters) {
			hr = font_sh->HasCharacter((UINT32)character, &exists);
			if (FAILED(hr) || !exists)
				ret.missing += character;
		}
	}

	UINT32 file_count = 1;
	IDWriteFontFile* font_file;
	// DirectWrite only supports one file per face
	if (FAILED(font_face_sh->GetFiles(&file_count, &font_file)))
		return ret;
	agi::scoped_holder<IDWriteFontFile*> font_file_sh(font_file, [](IDWriteFontFile* p) { p->Release(); });

	IDWriteFontFileLoader* loader;
	if (FAILED(font_file_sh->GetLoader(&loader)))
		return ret;
	agi::scoped_holder<IDWriteFontFileLoader*> loader_sh(loader, [](IDWriteFontFileLoader* p) { p->Release(); });

	IDWriteLocalFontFileLoader* local_loader;
	if (FAILED(loader_sh->QueryInterface(__uuidof(IDWriteLocalFontFileLoader), (void**)&local_loader)))
		return ret;
	agi::scoped_holder<IDWriteLocalFontFileLoader*> local_loader_sh(local_loader, [](IDWriteLocalFontFileLoader* p) { p->Release(); });

	LPCVOID font_file_reference_key;
	UINT32 font_file_reference_key_size;
	if (FAILED(font_file_sh->GetReferenceKey(&font_file_reference_key, &font_file_reference_key_size)))
		return ret;

	UINT32 path_length;
	if (FAILED(local_loader_sh->GetFilePathLengthFromKey(font_file_reference_key, font_file_reference_key_size, &path_length)))
		return ret;

	std::wstring path(path_length, L'\0');
	if (FAILED(local_loader_sh->GetFilePathFromKey(font_file_reference_key, font_file_reference_key_size, path.data(), path_length + 1)))
		return ret;

	// DirectWrite always returns the file path in upper case. Ex: "C:\WINDOWS\FONTS\ARIAL.TTF"
	std::wstring normalized_path = normalizeFilePathCase(path);
	if (normalized_path.empty())
		return ret;

	ret.paths.push_back(agi::fs::path(normalized_path));

	return ret;
}
