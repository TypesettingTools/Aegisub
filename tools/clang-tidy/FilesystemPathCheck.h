#pragma once

#include <clang-tidy/ClangTidyCheck.h>

namespace aegisub::tidy {

class FilesystemPathCheck final : public clang::tidy::ClangTidyCheck {
public:
	using ClangTidyCheck::ClangTidyCheck;
	void registerMatchers(clang::ast_matchers::MatchFinder *finder) override;
	void check(clang::ast_matchers::MatchFinder::MatchResult const& result) override;
};

} // namespace aegisub::tidy
