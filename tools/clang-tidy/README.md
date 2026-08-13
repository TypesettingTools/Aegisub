# Aegisub clang-tidy checks

Project-specific checks live in a loadable clang-tidy module. The initial
`aegisub-filesystem-path` check reserves filesystem-touching
`std::filesystem` calls for the implementation of `agi::fs`. Pure lexical path
operations, path construction, and `status()` are not restricted. The latter
has a single intentional caller in the access adapter and does not justify an
exception in the first check.

## Candidate boundaries

The following boundaries are promising, but should only become checks after an
inventory and cleanup leaves a small, explicit adapter surface.

### System calls

Direct operating-system APIs belong in `libaegisub` platform adapters. A check
should cover POSIX file/process/environment calls and Win32 APIs, allow calls
inside the narrow `agi::fs`, `agi::acs`, process, and platform integration
implementations which own their error and encoding contracts, and reject them
in application/UI code. C and third-party library APIs are not syscalls merely
because they accept `char *`; the matcher should use canonical declarations
from system headers rather than names alone.

### wx string conversions

`wxString` belongs at the wxWidgets UI boundary. Conversion to or from UTF-8 is
allowed when data crosses between wxWidgets controls/events and UTF-8 core
types, or when calling a C API with a documented UTF-8 contract. It should be
rejected as an intermediate representation in `libaegisub`, and temporary
`utf8_str()`/`wx_str()` buffers must not escape the full expression which owns
them. Matchers should distinguish conversion direction and the callee's
parameter type; banning conversion method names globally would flag valid UI
adapters.
