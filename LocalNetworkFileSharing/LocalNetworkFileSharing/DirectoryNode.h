#pragma once

#include <vector>
#include <filesystem>
#include <random>
#include <functional>
#include <iostream>
#include <map>
#include <assert.h>

#include "MyRandom.h"
#include "FileNode.h"

namespace fs = std::filesystem;

class DirectoryNode
{

public:
	DirectoryNode(fs::path localPathOnDisk, std::map<std::string, DirectoryNode*>& allDirectories)
	{
		static MyRandom idGenerator = MyRandom(false);

		Files = std::map<std::string, std::unique_ptr<FileNode>>();
		SubDirectories = std::vector<std::unique_ptr<DirectoryNode>>();
		FolderName = localPathOnDisk.filename().string();
		LocalPathOnDisk = localPathOnDisk.string();

		// 18446744073709551615 = uint64::max = 20 digits
		ID = FolderName + "_" + std::format("{:020}", idGenerator());

		AddToDirectoryTree(allDirectories, this);
	}

	void print()
	{
		std::cout << Files.size() << "; " << SubDirectories.size() << "; " << FolderName << "; " << ID << "; " << LocalPathOnDisk << std::endl;
	}

private:

	std::map<std::string, std::unique_ptr<FileNode>> Files;
	std::vector<std::unique_ptr<DirectoryNode>> SubDirectories;
	std::string FolderName;
	std::string ID;
	fs::path LocalPathOnDisk;

	void AddToDirectoryTree(std::map<std::string, DirectoryNode*>& allDirectories, DirectoryNode* currentNode);
};