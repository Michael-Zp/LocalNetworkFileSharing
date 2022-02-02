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

class SharedFolder;

namespace fs = std::filesystem;

class DirectoryNode
{

public:
	DirectoryNode(fs::path localPathOnDisk, SharedFolder& rootFolder)
	{
		static MyRandom idGenerator = MyRandom(false);

		Files = std::map<std::string, std::unique_ptr<FileNode>>();
		SubDirectories = std::vector<std::unique_ptr<DirectoryNode>>();
		FolderName = localPathOnDisk.filename().string();
		LocalPathOnDisk = localPathOnDisk.string();

		// 18446744073709551615 = uint64::max = 20 digits
		ID = FolderName + "_" + std::format("{:020}", idGenerator());

		AddToDirectoryTree(this, rootFolder);
	}

	static std::unique_ptr<DirectoryNode> FromId(std::string id)
	{
		std::unique_ptr<DirectoryNode> newDirNode;
		newDirNode->Files = std::map<std::string, std::unique_ptr<FileNode>>();
		newDirNode->SubDirectories = std::vector<std::unique_ptr<DirectoryNode>>();
		newDirNode->ID = id;
		newDirNode->FolderName = id.substr(0, id.find_last_of("_"));
		return std::move(newDirNode);
	}

	const std::string& GetID()
	{
		return ID;
	}

	void print()
	{
		std::cout << Files.size() << "; " << SubDirectories.size() << "; " << FolderName << "; " << ID << "; " << LocalPathOnDisk << std::endl;
	}

	const std::map<std::string, std::unique_ptr<FileNode>>* GetFiles()
	{
		return &Files;
	}

	void AddFile(std::string id, std::unique_ptr<FileNode> fileNode)
	{
		Files.emplace(id, std::move(fileNode));
	}

	const std::vector<std::unique_ptr<DirectoryNode>>* GetSubDirs()
	{
		return &SubDirectories;
	}

private:

	DirectoryNode()
	{ }

	std::map<std::string, std::unique_ptr<FileNode>> Files;
	std::vector<std::unique_ptr<DirectoryNode>> SubDirectories;
	std::string FolderName;
	std::string ID;
	fs::path LocalPathOnDisk;

	void AddToDirectoryTree(DirectoryNode* currentNode, SharedFolder& rootFolder);
};