// Copyright (c) 2014, Thomas Goyne <plorkyeran@aegisub.org>
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

#include "flyweight_hash.h"

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

class AssDialogue;
class wxDC;
class wxString;
namespace agi { struct Context; }

class WidthHelper {
	struct Entry {
		int width;
		int age;
	};
	int age = 0;
	wxDC *dc = nullptr;
	std::unordered_map<boost::flyweight<std::string>, Entry> widths;
#ifdef _WIN32
	wxString scratch;
#endif

public:
	void SetDC(wxDC *dc) { this->dc = dc; }
	void Age();

	int operator()(boost::flyweight<std::string> const& str);
	int operator()(std::string const& str);
	int operator()(wxString const& str);
	int operator()(const char *str);
	int operator()(const wchar_t *str);
};

class GridColumn {
protected:
	int width = 0;
	bool visible = true;

	virtual int Width(const agi::Context *c, WidthHelper &helper) const = 0;
	virtual wxString Value(const AssDialogue *d, const agi::Context *c) const = 0;

public:
	virtual ~GridColumn() = default;

	virtual bool Centered() const { return false; }
	virtual bool CanHide() const { return true; }
	virtual bool RefreshOnTextChange() const { return false; }

	virtual wxString const& Header() const = 0;
	virtual wxString const& Description() const = 0;
	virtual void Paint(wxDC &dc, int x, int y, const AssDialogue *d, const agi::Context *c) const;

	int Width() const { return width; }
	bool Visible() const { return visible; }

	virtual void UpdateWidth(const agi::Context *c, WidthHelper &helper);
	virtual void SetByFrame(bool /* by_frame */) { }
	void SetVisible(bool new_value) { visible = new_value; }
};

std::vector<std::unique_ptr<GridColumn>> GetGridColumns();
