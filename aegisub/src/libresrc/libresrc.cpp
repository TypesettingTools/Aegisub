#include "libresrc.h"

wxBitmap libresrc_getimage(const unsigned char *buff, size_t size) {
	wxMemoryInputStream mem(buff, size);
	wxImage image(mem);
	return wxBitmap(image);
}

const std::string libresrc_getconfig(const char *config, size_t size) {
	std::string str(config, size);
	return str;
}
