#include <wx/mstream.h>
#include <wx/bitmap.h>
#include <wx/image.h>

#include <sstream>

#include "bitmap.h"
#include "default_config.h"

wxBitmap libresrc_getimage(const unsigned char *image, size_t size);
#define GETIMAGE(a) libresrc_getimage(a, sizeof(a))

const std::string libresrc_getconfig(const char *config, size_t size);
#define CET_DEFAULT_CONFIG(a) libresrc_getconfig(a, sizeof(a))
