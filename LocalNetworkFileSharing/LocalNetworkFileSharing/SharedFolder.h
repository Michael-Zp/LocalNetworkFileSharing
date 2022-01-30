#pragma once

#include <filesystem>
#include <map>

#include "DirectoryNode.h"

namespace fs = std::filesystem;

class SharedFolder
{
public:
	SharedFolder(fs::path localPathOnDisk)
	{
		Root = std::make_unique<DirectoryNode>(localPathOnDisk, AllDirectories, DirectoriesSize, FilesSize);
	}

	std::vector<char> Serialize()
	{

	}

	static SharedFolder* Deserialize()
	{

	}

private:

	uint64_t DirectoriesSize;
	uint64_t FilesSize;

	std::unique_ptr<DirectoryNode> Root;
	std::map<std::string, DirectoryNode*> AllDirectories;
};
