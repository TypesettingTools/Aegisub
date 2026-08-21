#include <filesystem>

namespace agi::fs {
bool Exists(std::filesystem::path const& path) {
	return std::filesystem::exists(path); // allowed: wrapper implementation
}
}

bool bypasses_wrapper(std::filesystem::path const& path) {
	return std::filesystem::exists(path);
// CHECK-MESSAGES: :[[@LINE-1]]:9: warning: filesystem operations must go through agi::fs
}

auto lexical_operation(std::filesystem::path const& path) {
	return path.lexically_normal(); // allowed: does not touch the filesystem
}

auto status_operation(std::filesystem::path const& path) {
	return std::filesystem::status(path); // intentionally outside the first rule
}
