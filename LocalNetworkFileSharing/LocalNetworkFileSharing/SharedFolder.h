#pragma once

#include <filesystem>
#include <map>
#include <stack>

#include "DirectoryNode.h"

namespace fs = std::filesystem;

class SharedFolder
{
public:
	SharedFolder(fs::path localPathOnDisk)
	{
		Root = std::make_unique<DirectoryNode>(localPathOnDisk, *this);
	}

	static SharedFolder Deserialize(uint64_t dataSize, char* data)
	{
		uint64_t pos = 0;

		SharedFolder newSharedFolder;
		
		newSharedFolder.Root = ReadDirectory(data, pos, dataSize, newSharedFolder);

		std::stack<DirectoryNode*> directoryHierarchy;
		directoryHierarchy.push(newSharedFolder.Root.get());

		while (pos < dataSize)
		{
			if (data[pos] & (char)SerializeFlags::DirChange)
			{
				if (data[pos] & (char)SerializeFlags::GoDown)
				{
					++pos;
					std::unique_ptr<DirectoryNode> newDir = ReadDirectory(data, pos, dataSize, newSharedFolder);
					directoryHierarchy.top()->AddDir(std::move(newDir));
					directoryHierarchy.push(newDir.get());
				}
				else if (data[pos] & (char)SerializeFlags::GoUp)
				{
					++pos;
					directoryHierarchy.pop();
				}
				else
				{
					// Should not happen
					assert(false);
				}
			}
			else if (data[pos] & (char)SerializeFlags::Directory)
			{
				++pos;
				directoryHierarchy.top()->AddDir(std::move(ReadDirectory(data, pos, dataSize, newSharedFolder)));
			}
			else
			{
				// Should not happen
				assert(false);
			}
		}

		return newSharedFolder;
	}

	static std::unique_ptr<DirectoryNode> ReadDirectory(char* data, uint64_t& pos, uint64_t dataSize, SharedFolder& root)
	{
		assert(CheckBounds<char>(pos, dataSize));

		// Always has to be a folder as the root
		assert(data[pos] == (char)SerializeFlags::Directory);
		++pos;

		assert(CheckBounds<uint64_t>(pos, dataSize));
		uint64_t* idSize = reinterpret_cast<uint64_t*>(&(data[pos]));
		pos += sizeof(uint64_t);

		assert(CheckBounds(*idSize, pos, dataSize));
		std::string id = std::string(&(data[pos]), *idSize);
		pos += *idSize;

		std::unique_ptr<DirectoryNode> newDir = DirectoryNode::FromId(id);

		newDir->print();
		++root.DirectoriesCount;
		root.DirectoriesSize += *idSize;
		root.AllDirectories.emplace(id, newDir.get());

		ReadFiles(data, pos, dataSize, newDir.get(), root);

		return std::move(newDir);
	}

	static void ReadFiles(char* data, uint64_t& pos, uint64_t dataSize, DirectoryNode* parent, SharedFolder& root)
	{
		if (!CheckBounds<char>(pos, dataSize))
		{
			// Seems to be the end of the buffer, so just return
			return;
		}

		while (data[pos] == (char)SerializeFlags::File)
		{
			++pos;

			CheckBounds<uint64_t>(pos, dataSize);
			uint64_t* idSize = reinterpret_cast<uint64_t*>(&(data[pos]));
			pos += sizeof(uint64_t);

			CheckBounds(*idSize, pos, dataSize);
			std::string id = std::string(&(data[pos]), *idSize);
			pos += *idSize;

			std::unique_ptr<FileNode> newFile = FileNode::FromId(id);

			newFile->print();
			++root.FilesCount;
			root.FilesSize += *idSize;
			
			parent->AddFile(id, std::move(newFile));
		}
	}

	template <typename T>
	static bool CheckBounds(uint64_t pos, uint64_t size)
	{
		return CheckBounds(sizeof(T), pos, size);
	}

	static bool CheckBounds(uint64_t requiredSize, uint64_t pos, uint64_t size)
	{
		return (size - pos > requiredSize);
	}

	void SaveDirectoryMetaData(DirectoryNode* directoryNode)
	{
		DirectoriesSize += directoryNode->GetID().size();
		++DirectoriesCount;
		AllDirectories.emplace(directoryNode->GetID(), directoryNode);
	}

	void SaveFileMetaData(FileNode* fileNode)
	{
		FilesSize += fileNode->GetID().size();;
		++FilesCount;
	}

	std::pair<uint64_t, char*> Serialize()
	{
		uint64_t dirFlagsAndSpaceForSize = DirectoriesCount * (sizeof(char) * 2 + sizeof(uint64_t)); // char * 2 because we need one flag to GoDown and one to GoUp for each folder
		uint64_t fileFlagsAndSpaceForSizes = FilesCount * (sizeof(char) + sizeof(uint64_t));
		uint64_t bufSize = dirFlagsAndSpaceForSize + DirectoriesSize +  fileFlagsAndSpaceForSizes + FilesSize;
		char* serializeBuffer = new char[bufSize];

		uint64_t currentPos = 0;

		SerializeFolder(Root.get(), serializeBuffer, currentPos, bufSize);

		return std::make_pair(bufSize, serializeBuffer);
	}

	void SerializeFolder(DirectoryNode* currentNode, char* buf, uint64_t& pos, const uint64_t bufSize)
	{
		// Next entry is dir
		buf[pos] = (char)SerializeFlags::Directory;
		++pos;
		
		uint64_t sizeOfDirId = currentNode->GetID().size();
		
		// Size of ID
		memcpy_s(buf + pos, bufSize - pos, &sizeOfDirId, sizeof(uint64_t));
		pos += sizeof(uint64_t);

		// ID string
		memcpy_s(buf + pos, bufSize - pos, currentNode->GetID().data(), sizeOfDirId);
		pos += sizeOfDirId;


		// Files
		auto files = currentNode->GetFiles();

		for (const auto& file : *files)
		{	
			// Next entry is file
			buf[pos] = (char)SerializeFlags::File;
			++pos;

			uint64_t sizeOfFileId = file.first.size();

			// Size of ID
			memcpy_s(buf + pos, bufSize - pos, &sizeOfFileId, sizeof(uint64_t));
			pos += sizeof(uint64_t);

			// ID string
			memcpy_s(buf + pos, bufSize - pos, file.first.data(), sizeOfFileId);
			pos += sizeOfFileId;
		}


		// Sub folders
		auto subFolders = currentNode->GetSubDirs();

		if (subFolders->size() > 0)
		{
			buf[pos] = (char)SerializeFlags::DirChange | (char)SerializeFlags::GoDown;
			++pos;

			for (const auto& subFolder : *subFolders)
			{
				SerializeFolder(subFolder.get(), buf, pos, bufSize);
			}
		}

		buf[pos] = (char)SerializeFlags::DirChange | (char)SerializeFlags::GoUp;
		++pos;
	}

private:

	uint64_t DirectoriesSize;
	uint64_t DirectoriesCount;
	uint64_t FilesSize;
	uint64_t FilesCount;

	std::unique_ptr<DirectoryNode> Root;
	std::map<std::string, DirectoryNode*> AllDirectories;

	SharedFolder() : DirectoriesSize(0), DirectoriesCount(0), FilesSize(0), FilesCount(0)
	{ }

	enum class SerializeFlags : char
	{
		File		= 0x00,
		Directory	= 0x01,
		DirChange	= 0x02,
		NoChange	= 0x04,
		GoDown		= 0x08, // current/New
		GoUp		= 0x10, // current/../New
	};
};
