#ifndef AGI_PRE
#include <string>

#include <wx/bitmap.h>
#include <wx/icon.h>
#include <wx/image.h>
#include <wx/mstream.h>
#endif

#include "bitmap.h"
#include "default_config.h"

wxBitmap libresrc_getimage(const unsigned char *image, size_t size);
wxIcon libresrc_geticon(const unsigned char *image, size_t size);
#define GETIMAGE(a) libresrc_getimage(a, sizeof(a))
#define GETICON(a) libresrc_geticon(a, sizeof(a))

const std::string libresrc_getconfig(const unsigned char *config, size_t size);
#define GET_DEFAULT_CONFIG(a) libresrc_getconfig(a, sizeof(a))
