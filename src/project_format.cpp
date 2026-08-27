#include "project_format.h"

#include "ass_attachment.h"
#include "ass_dialogue.h"
#include "ass_info.h"
#include "ass_style.h"
#include "project_document.h"

#include <libaegisub/cajun/reader.h>
#include <libaegisub/cajun/writer.h>
#include <libaegisub/exception.h>
#include <libaegisub/io.h>

#include <limits>
#include <unordered_set>

namespace {
json::Object const& object(json::UnknownElement const& value) { return value; }
json::Array const& array(json::UnknownElement const& value) { return value; }
std::string string(json::UnknownElement const& value) { return static_cast<json::String const&>(value); }
bool read_bool(json::UnknownElement const& value) { return static_cast<json::Boolean const&>(value); }
int integer(json::UnknownElement const& value) {
	auto value64 = static_cast<json::Integer const&>(value);
	if (value64 < std::numeric_limits<int>::min() || value64 > std::numeric_limits<int>::max())
		throw json::Exception("integer outside supported range");
	return static_cast<int>(value64);
}
double number(json::UnknownElement const& value) {
	try { return static_cast<json::Double const&>(value); }
	catch (json::Exception const&) { return static_cast<double>(static_cast<json::Integer const&>(value)); }
}

json::Array margins(std::array<int, 3> const& values) {
	json::Array out; for (int value : values) out.emplace_back(value); return out;
}
std::array<int, 3> margins(json::UnknownElement const& value) {
	auto const& values = array(value);
	if (values.size() != 3) throw json::Exception("margin must contain exactly three integers");
	return {{integer(values[0]), integer(values[1]), integer(values[2])}};
}
json::Array color(agi::Color const& value) {
	json::Array out; out.emplace_back(value.r); out.emplace_back(value.g); out.emplace_back(value.b); out.emplace_back(value.a); return out;
}
agi::Color color(json::UnknownElement const& value) {
	auto const& values = array(value);
	if (values.size() != 4) throw json::Exception("color must contain exactly four integers");
	int rgba[4] = {integer(values[0]), integer(values[1]), integer(values[2]), integer(values[3])};
	for (int channel : rgba)
		if (channel < 0 || channel > 255) throw json::Exception("color channel outside 0..255");
	return agi::Color(rgba[0], rgba[1], rgba[2], rgba[3]);
}

std::string base64_encode(std::string_view input) {
	static constexpr char chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	std::string out;
	for (size_t i = 0; i < input.size(); i += 3) {
		uint32_t value = static_cast<unsigned char>(input[i]) << 16;
		if (i + 1 < input.size()) value |= static_cast<unsigned char>(input[i + 1]) << 8;
		if (i + 2 < input.size()) value |= static_cast<unsigned char>(input[i + 2]);
		out += chars[(value >> 18) & 63]; out += chars[(value >> 12) & 63];
		out += i + 1 < input.size() ? chars[(value >> 6) & 63] : '=';
		out += i + 2 < input.size() ? chars[value & 63] : '=';
	}
	return out;
}
std::string base64_decode(std::string_view input) {
	static std::string const chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	if (input.size() % 4) throw json::Exception("invalid base64 length");
	std::string out;
	for (size_t i = 0; i < input.size(); i += 4) {
		uint32_t value = 0; int padding = 0;
		for (size_t j = 0; j < 4; ++j) {
			if (input[i + j] == '=') { ++padding; value <<= 6; continue; }
			auto pos = chars.find(input[i + j]);
			if (pos == std::string::npos || padding) throw json::Exception("invalid base64 data");
			value = (value << 6) | static_cast<uint32_t>(pos);
		}
		if (padding > 2 || (padding && i + 4 != input.size())) throw json::Exception("invalid base64 padding");
		out += static_cast<char>((value >> 16) & 255);
		if (padding < 2) out += static_cast<char>((value >> 8) & 255);
		if (!padding) out += static_cast<char>(value & 255);
	}
	return out;
}

json::Object write_properties(ProjectProperties const& p) {
	json::Object out;
	out["automation_scripts"] = p.automation_scripts; out["export_filters"] = p.export_filters;
	out["export_encoding"] = p.export_encoding; out["style_storage"] = p.style_storage;
	out["audio_file"] = p.audio_file; out["video_file"] = p.video_file;
	out["timecodes_file"] = p.timecodes_file; out["keyframes_file"] = p.keyframes_file;
	json::Object settings; for (auto const& [key, value] : p.automation_settings) settings[key] = value;
	out["automation_settings"] = std::move(settings);
	out["video_zoom"] = p.video_zoom; out["aspect_ratio_value"] = p.ar_value;
	out["scroll_position"] = p.scroll_position; out["active_row"] = p.active_row;
	out["aspect_ratio_mode"] = p.ar_mode; out["video_position"] = p.video_position;
	return out;
}
ProjectProperties read_properties(json::Object const& in) {
	ProjectProperties p;
	p.automation_scripts = string(in.at("automation_scripts")); p.export_filters = string(in.at("export_filters"));
	p.export_encoding = string(in.at("export_encoding")); p.style_storage = string(in.at("style_storage"));
	p.audio_file = string(in.at("audio_file")); p.video_file = string(in.at("video_file"));
	p.timecodes_file = string(in.at("timecodes_file")); p.keyframes_file = string(in.at("keyframes_file"));
	for (auto const& [key, value] : object(in.at("automation_settings"))) p.automation_settings[key] = string(value);
	p.video_zoom = number(in.at("video_zoom")); p.ar_value = number(in.at("aspect_ratio_value"));
	p.scroll_position = integer(in.at("scroll_position")); p.active_row = integer(in.at("active_row"));
	p.ar_mode = integer(in.at("aspect_ratio_mode")); p.video_position = integer(in.at("video_position"));
	return p;
}
}

void ProjectFormat::Write(ProjectDocument const& source, agi::fs::path const& filename) {
	json::Object root; root["format"] = "aegisub-project"; root["version"] = Version;
	json::Array info; for (auto const& value : source.Info) { json::Object item; item["key"] = value.Key(); item["value"] = value.Value(); info.emplace_back(std::move(item)); }
	root["script_info"] = std::move(info);
	json::Array styles;
	for (auto const& s : source.Styles) {
		json::Object o; o["name"] = s.name; o["font"] = s.font; o["font_size"] = s.fontsize;
		json::Array colors; colors.emplace_back(color(s.primary)); colors.emplace_back(color(s.secondary)); colors.emplace_back(color(s.outline)); colors.emplace_back(color(s.shadow)); o["colors"] = std::move(colors);
		o["bold"] = s.bold; o["italic"] = s.italic; o["underline"] = s.underline; o["strikeout"] = s.strikeout;
		o["scale_x"] = s.scalex; o["scale_y"] = s.scaley; o["spacing"] = s.spacing; o["angle"] = s.angle;
		o["border_style"] = s.borderstyle; o["outline"] = s.outline_w; o["shadow"] = s.shadow_w;
		o["alignment"] = s.alignment; o["margin"] = margins(s.Margin); o["encoding"] = s.encoding;
		styles.emplace_back(std::move(o));
	}
	root["styles"] = std::move(styles);
	json::Array events;
	for (auto const& d : source.Events) {
		json::Object o; o["id"] = d.Id; o["comment"] = d.Comment; o["layer"] = d.Layer;
		o["start_ms"] = static_cast<int>(d.Start); o["end_ms"] = static_cast<int>(d.End); o["style"] = d.Style.get(); o["actor"] = d.Actor.get();
		o["effect"] = d.Effect.get(); o["margin"] = margins(d.Margin); o["text"] = d.Text.get();
		json::Array ids; for (auto id : d.ExtradataIds.get()) ids.emplace_back(static_cast<int64_t>(id)); o["extradata"] = std::move(ids);
		events.emplace_back(std::move(o));
	}
	root["events"] = std::move(events);
	json::Array extra; for (auto const& e : source.Extradata) { json::Object o; o["id"] = static_cast<int64_t>(e.id); o["key"] = e.key; o["value"] = e.value; extra.emplace_back(std::move(o)); }
	root["extradata"] = std::move(extra);
	json::Array attachments; for (auto const& a : source.Attachments) { json::Object o; o["name"] = a.GetFileName(true); o["kind"] = a.Group() == AssEntryGroup::FONT ? "font" : "graphic"; o["data_base64"] = base64_encode(a.GetData()); attachments.emplace_back(std::move(o)); }
	root["attachments"] = std::move(attachments); root["properties"] = write_properties(source.Properties);
	agi::JsonWriter::Write(root, agi::io::Save(filename).Get());
}

void ProjectFormat::Read(ProjectDocument& target, agi::fs::path const& filename) {
	try {
		json::UnknownElement element; json::Reader::Read(element, *agi::io::Open(filename)); auto const& root = object(element);
		if (string(root.at("format")) != "aegisub-project") throw json::Exception("not an Aegisub project");
		if (integer(root.at("version")) != Version) throw json::Exception("unsupported Aegisub project version");
		ProjectDocument loaded; loaded.Info.clear(); loaded.Styles.clear(); loaded.Events.clear(); loaded.Attachments.clear(); loaded.Extradata.clear();
		for (auto const& value : array(root.at("script_info"))) { auto const& o = object(value); loaded.Info.emplace_back(string(o.at("key")), string(o.at("value"))); }
		for (auto const& value : array(root.at("styles"))) { auto const& o = object(value); AssStyle *s = new AssStyle; s->name=string(o.at("name")); s->font=string(o.at("font")); s->fontsize=number(o.at("font_size")); auto const& cs=array(o.at("colors")); if(cs.size()!=4) throw json::Exception("colors must contain four colors"); s->primary=color(cs[0]); s->secondary=color(cs[1]); s->outline=color(cs[2]); s->shadow=color(cs[3]); s->bold=read_bool(o.at("bold")); s->italic=read_bool(o.at("italic")); s->underline=read_bool(o.at("underline")); s->strikeout=read_bool(o.at("strikeout")); s->scalex=number(o.at("scale_x")); s->scaley=number(o.at("scale_y")); s->spacing=number(o.at("spacing")); s->angle=number(o.at("angle")); s->borderstyle=integer(o.at("border_style")); s->outline_w=number(o.at("outline")); s->shadow_w=number(o.at("shadow")); s->alignment=integer(o.at("alignment")); s->Margin=margins(o.at("margin")); s->encoding=integer(o.at("encoding")); s->UpdateData(); loaded.Styles.push_back(*s); }
		std::unordered_set<uint32_t> extra_ids; uint32_t next_id = 0;
		for (auto const& value : array(root.at("extradata"))) { auto const& o=object(value); int id=integer(o.at("id")); if(id<0) throw json::Exception("negative extradata id"); auto unsigned_id=static_cast<uint32_t>(id); if(!extra_ids.insert(unsigned_id).second) throw json::Exception("duplicate extradata id"); loaded.Extradata.push_back({unsigned_id, string(o.at("key")), string(o.at("value"))}); next_id=std::max(next_id, unsigned_id+1); }
		loaded.next_extradata_id=next_id;
		std::unordered_set<int> event_ids; int max_event_id = 0;
		for (auto const& value : array(root.at("events"))) { auto const& o=object(value); AssDialogue *d=new AssDialogue; d->Id=integer(o.at("id")); if(d->Id <= 0) throw json::Exception("event id must be positive"); if(!event_ids.insert(d->Id).second) throw json::Exception("duplicate event id"); max_event_id=std::max(max_event_id, d->Id); d->Comment=read_bool(o.at("comment")); d->Layer=integer(o.at("layer")); d->Start=integer(o.at("start_ms")); d->End=integer(o.at("end_ms")); d->Style=string(o.at("style")); d->Actor=string(o.at("actor")); d->Effect=string(o.at("effect")); d->Margin=margins(o.at("margin")); d->Text=string(o.at("text")); std::vector<uint32_t> ids; for(auto const& id_value:array(o.at("extradata"))){int id=integer(id_value); if(id<0 || !extra_ids.count(static_cast<uint32_t>(id))) throw json::Exception("event references missing extradata"); ids.push_back(static_cast<uint32_t>(id));} d->ExtradataIds=std::move(ids); loaded.Events.push_back(*d); }
		for (auto const& value : array(root.at("attachments"))) { auto const& o=object(value); auto kind=string(o.at("kind")); if(kind!="font"&&kind!="graphic") throw json::Exception("unknown attachment kind"); loaded.Attachments.emplace_back(string(o.at("name")), kind=="font"?AssEntryGroup::FONT:AssEntryGroup::GRAPHIC, base64_decode(string(o.at("data_base64")))); }
		loaded.Properties=read_properties(object(root.at("properties"))); AssDialogue::EnsureNextId(max_event_id); target.swap(loaded);
	}
	catch (std::exception const& e) { throw agi::InvalidInputException("Invalid Aegisub project: " + std::string(e.what())); }
}
