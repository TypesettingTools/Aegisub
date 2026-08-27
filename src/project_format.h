#pragma once

#include <libaegisub/fs.h>

class ProjectDocument;

/// Strict, versioned serialization for Aegisub's editable project document.
class ProjectFormat final {
public:
	static constexpr int Version = 1;
	static void Read(ProjectDocument& target, agi::fs::path const& filename);
	static void Write(ProjectDocument const& source, agi::fs::path const& filename);
};
