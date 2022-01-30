#include "DirectoryNode.h"

void DirectoryNode::AddToDirectoryTree(DirectoryNode* currentNode, std::map<std::string, DirectoryNode*>& allDirectories, uint64_t& directoriesSize, uint64_t& filesSize)
{
	// Only directories should be added. The logic will not work with a file.
	assert(fs::is_directory(currentNode->LocalPathOnDisk));

	allDirectories.emplace(currentNode->ID, currentNode);

	currentNode->print();

	for (const auto& p : fs::directory_iterator(currentNode->LocalPathOnDisk))
	{
		if (p.is_directory())
		{
			auto thisNode = std::make_unique<DirectoryNode>(p.path(), allDirectories);

			AddToDirectoryTree(thisNode.get(), allDirectories, directoriesSize, filesSize);

			currentNode->SubDirectories.push_back(std::move(thisNode));
		}
		else
		{
			auto newFile = std::make_unique<FileNode>(p.path().filename().string());
			newFile->print();
			currentNode->Files.emplace(newFile->GetId(), std::move(newFile));
		}

	}
}