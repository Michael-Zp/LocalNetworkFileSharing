#include "DirectoryNode.h"
#include "SharedFolder.h"

void DirectoryNode::AddToDirectoryTree(DirectoryNode* currentNode, SharedFolder& rootFolder)
{
	// Only directories should be added. The logic will not work with a file.
	assert(fs::is_directory(currentNode->LocalPathOnDisk));

	rootFolder.SaveDirectoryMetaData(currentNode);

	currentNode->print();

	for (const auto& p : fs::directory_iterator(currentNode->LocalPathOnDisk))
	{
		if (p.is_directory())
		{
			auto thisNode = std::make_unique<DirectoryNode>(p.path(), rootFolder);

			currentNode->SubDirectories.push_back(std::move(thisNode));
		}
		else
		{
			auto newFile = std::make_unique<FileNode>(p.path().filename().string());
			newFile->print();
			rootFolder.SaveFileMetaData(newFile.get());
			currentNode->Files.emplace(newFile->GetID(), std::move(newFile));
		}
	}
}