#include <filesystem>

#include "SharedFolder.h"

namespace fs = std::filesystem;

int main(int argc, const char* argv)
{
	auto sharedFolder = SharedFolder(fs::current_path().parent_path().parent_path().append("TestFiles"));

	auto serialized = sharedFolder.Serialize();

	std::string testStr = std::string(serialized.second, serialized.first);

	auto deserialized = SharedFolder::Deserialize(serialized.first, serialized.second);

	return 0;
}