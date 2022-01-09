#include <iostream>
#include <filesystem>
#include <assert.h>

namespace fs = std::filesystem;

struct FileNode
{
	std::vector<std::unique_ptr<FileNode>> Children;
	std::string internalPath;
	fs::path fullPath;

	void print()
	{
		std::cout << fullPath << ";" << internalPath << std::endl;
	}
};

void addToDirectoryTree(const fs::path& currentDir, FileNode* currentNode)
{
	// Only directories should be added. The logic will not work with a file.
	assert(fs::is_directory(currentDir));

	// Should not be platform specific, but rather specific for the application, so use a default / as the separator.
	std::unique_ptr<FileNode> thisNode = std::make_unique<FileNode>(std::vector<std::unique_ptr<FileNode>>(), currentNode->internalPath + currentDir.filename().string() + "/", currentDir);

	currentNode->print();

	for (const auto& p : fs::directory_iterator(currentDir))
	{
		if (p.is_directory())
		{
			addToDirectoryTree(p, thisNode.get());
		}
		else
		{
			currentNode->Children.push_back(
				std::make_unique<FileNode>(
					std::vector<std::unique_ptr<FileNode>>(),
					currentNode->internalPath + currentDir.filename().string(),
					p.path())
			);

			currentNode->Children[currentNode->Children.size() - 1]->print();
		}
		
	}

	currentNode->Children.push_back(std::move(thisNode));
}

int main(int argc, const char* argv)
{
	fs::path testFilesPath(fs::current_path().parent_path().parent_path().append("TestFiles"));

	std::unique_ptr<FileNode> root = std::make_unique<FileNode>(std::vector<std::unique_ptr<FileNode>>(), "/", testFilesPath);

	addToDirectoryTree(testFilesPath, root.get());

	return 0;
}