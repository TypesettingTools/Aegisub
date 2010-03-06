#include "libresrc.h"

wxBitmap libresrc_getimage(const unsigned char *buff, size_t size) {
	wxMemoryInputStream mem(buff, size);
	wxImage image(mem);
	return wxBitmap(image);
}

const std::string libresrc_getconfig(const unsigned char *config, size_t size) {
        std::string str((char*)config, size);
        return str.c_str();
}
