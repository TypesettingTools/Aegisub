#include <libaegisub/exception.h>


DEFINE_BASE_EXCEPTION_NOINNER(AudioProviderError, agi::Exception);
DEFINE_SIMPLE_EXCEPTION_NOINNER(AudioOpenError, AudioProviderError, "audio/open/failed");

/// Error of some sort occurred while decoding a frame
DEFINE_SIMPLE_EXCEPTION_NOINNER(AudioDecodeError, AudioProviderError, "audio/error");


DEFINE_BASE_EXCEPTION_NOINNER(VideoProviderError, agi::Exception);
/// File could be opened, but is not a supported format
DEFINE_SIMPLE_EXCEPTION_NOINNER(VideoNotSupported, VideoProviderError, "video/open/notsupported");
/// File appears to be a supported format, but could not be opened
DEFINE_SIMPLE_EXCEPTION_NOINNER(VideoOpenError, VideoProviderError, "video/open/failed");

/// Error of some sort occurred while decoding a frame
DEFINE_SIMPLE_EXCEPTION_NOINNER(VideoDecodeError, VideoProviderError, "video/error");


