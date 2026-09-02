#include "FilesystemPathCheck.h"

#include <clang-tidy/ClangTidyModule.h>
#include <clang-tidy/ClangTidyModuleRegistry.h>

namespace aegisub::tidy {

class AegisubModule final : public clang::tidy::ClangTidyModule {
public:
	void addCheckFactories(clang::tidy::ClangTidyCheckFactories &checks) override {
		checks.registerCheck<FilesystemPathCheck>("aegisub-filesystem-path");
	}
};

} // namespace aegisub::tidy

static clang::tidy::ClangTidyModuleRegistry::Add<aegisub::tidy::AegisubModule>
    registration("aegisub-module", "Aegisub project checks");

// Anchor the module so that the linker cannot discard its registration.
volatile int AegisubClangTidyModuleAnchorSource = 0;
