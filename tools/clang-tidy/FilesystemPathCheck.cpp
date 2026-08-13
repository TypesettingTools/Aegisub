#include "FilesystemPathCheck.h"

#include <clang/AST/ASTContext.h>
#include <clang/AST/Decl.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/ASTMatchers/ASTMatchers.h>

using namespace clang::ast_matchers;

namespace aegisub::tidy {
namespace {

// Operations which touch the filesystem and therefore interpret a path. Pure
// path manipulation (for example relative_path()) is intentionally excluded.
constexpr auto restricted_operations =
    "^::std::filesystem::(absolute|canonical|copy|copy_file|copy_symlink|"
    "create_directories|create_directory|create_directory_symlink|"
    "create_hard_link|create_symlink|equivalent|exists|file_size|"
    "hard_link_count|is_block_file|is_character_file|is_directory|is_empty|"
    "is_fifo|is_other|is_regular_file|is_socket|is_symlink|last_write_time|"
    "permissions|proximate|read_symlink|relative|remove|remove_all|rename|"
    "resize_file|space|weakly_canonical)$";

bool isInsideAgiFs(clang::Decl const *decl) {
	for (auto const *context = decl->getDeclContext(); context; context = context->getParent()) {
		auto const *ns = llvm::dyn_cast<clang::NamespaceDecl>(context);
		if (!ns) continue;
		if (ns->getQualifiedNameAsString() == "agi::fs") return true;
	}
	return false;
}

} // namespace

void FilesystemPathCheck::registerMatchers(MatchFinder *finder) {
	finder->addMatcher(
		callExpr(callee(functionDecl(matchesName(restricted_operations))),
		         hasAncestor(functionDecl().bind("caller")))
		    .bind("call"),
		this);
}

void FilesystemPathCheck::check(MatchFinder::MatchResult const& result) {
	auto const *call = result.Nodes.getNodeAs<clang::CallExpr>("call");
	auto const *caller = result.Nodes.getNodeAs<clang::FunctionDecl>("caller");
	if (!call || !caller || isInsideAgiFs(caller)) return;

	diag(call->getExprLoc(),
	     "filesystem operations must go through agi::fs so UTF-8 paths and errors are handled consistently");
}

} // namespace aegisub::tidy
