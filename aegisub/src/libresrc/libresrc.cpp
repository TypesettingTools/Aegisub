#include "../config.h"

#include "libresrc.h"

wxBitmap libresrc_getimage(const unsigned char *buff, size_t size) {
	wxMemoryInputStream mem(buff, size);
	return wxBitmap(wxImage(mem));
}

wxIcon libresrc_geticon(const unsigned char *buff, size_t size) {
	wxMemoryInputStream mem(buff, size);
	wxIcon icon;
	icon.CopyFromBitmap(wxBitmap(wxImage(mem)));
	return icon;
}

const std::string libresrc_getconfig(const unsigned char *config, size_t size) {
	return std::string(reinterpret_cast<const char *>(config), size);
}
