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
		Root = std::make_unique<DirectoryNode>(localPathOnDisk, AllDirectories);
	}

private:
	std::unique_ptr<DirectoryNode> Root;
	std::map<std::string, DirectoryNode*> AllDirectories;
};
