#include <iostream>
#include <filesystem>
#include <assert.h>
#include <random>
#include <format>
#include <functional>
#include <chrono>
#include <map>

namespace fs = std::filesystem;

struct FileNode;

struct DirectoryNode
{
	std::vector<std::unique_ptr<FileNode>> Files;
	std::vector<std::unique_ptr<DirectoryNode>> SubDirectories;
	std::string FolderName;
	std::string ID;
	fs::path LocalPathOnDisk;

	static std::uniform_int_distribution<uint64_t> Random;
	static std::default_random_engine RandomEngine;

	DirectoryNode(fs::path localPathOnDisk)
	{
		static bool randomInitialized = false;
		if (!randomInitialized)
		{
			// TODO; Enable once the stuff is written to disk on intialize. Makes testing easier.
			//RandomEngine.seed(std::chrono::system_clock::now().time_since_epoch().count());
			randomInitialized = true;
		}

		static auto IDGenerator = std::bind(Random, RandomEngine);

		Files = std::vector<std::unique_ptr<FileNode>>();
		SubDirectories = std::vector<std::unique_ptr<DirectoryNode>>();
		FolderName = localPathOnDisk.filename().string();
		LocalPathOnDisk = localPathOnDisk.string();
		ID = FolderName + "_" + std::format("{:010}", IDGenerator());
	}

	void print()
	{
		std::cout << Files.size() << "; " << SubDirectories.size() << "; " << FolderName << "; " << ID << "; " << LocalPathOnDisk << std::endl;
	}
};

std::default_random_engine DirectoryNode::RandomEngine = std::default_random_engine();
std::uniform_int_distribution<uint64_t> DirectoryNode::Random = std::uniform_int_distribution<uint64_t>(0, std::numeric_limits<uint64_t>::max());

struct FileNode
{
	std::string FileName;
	std::string ID;

	void print()
	{
		std::cout << FileName << std::endl;
	}
};

void addToDirectoryTree(std::map<std::string, DirectoryNode*>& allDirectories, const fs::path& currentDir, DirectoryNode* currentNode)
{
	// Only directories should be added. The logic will not work with a file.
	assert(fs::is_directory(currentDir));

	allDirectories.emplace(currentNode->ID, currentNode);

	currentNode->print();

	for (const auto& p : fs::directory_iterator(currentDir))
	{
		if (p.is_directory())
		{
			auto thisNode = std::make_unique<DirectoryNode>(p);

			addToDirectoryTree(allDirectories, p, thisNode.get());

			currentNode->SubDirectories.push_back(std::move(thisNode));
		}
		else
		{
			currentNode->Files.push_back(
				std::make_unique<FileNode>(p.path().filename().string())
			);

			currentNode->Files[currentNode->Files.size() - 1]->print();
		}
		
	}
}

int main(int argc, const char* argv)
{
	fs::path testFilesPath(fs::current_path().parent_path().parent_path().append("TestFiles"));

	std::unique_ptr<DirectoryNode> root = std::make_unique<DirectoryNode>(testFilesPath);
	std::map<std::string, DirectoryNode*> allDirectories = std::map<std::string, DirectoryNode*>();

	addToDirectoryTree(allDirectories, testFilesPath, root.get());

	return 0;
}