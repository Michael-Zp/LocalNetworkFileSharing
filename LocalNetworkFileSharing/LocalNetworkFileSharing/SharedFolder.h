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
		Root = std::make_unique<DirectoryNode>(localPathOnDisk, *this);
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

	char* Serialize()
	{
		uint64_t bufSize = DirectoriesCount * sizeof(char) + DirectoriesSize + FilesCount * sizeof(char) + FilesSize;
		char* serializeBuffer = new char[bufSize];

		uint64_t currentPos = 0;

		SerializeFolder(Root.get(), serializeBuffer, currentPos, bufSize);
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

			uint64_t sizeOfFileId = currentNode->GetID().size();

			// Size of ID
			memcpy_s(buf + pos, bufSize - pos, &sizeOfFileId, sizeof(uint64_t));
			pos += sizeof(uint64_t);

			// ID string
			memcpy_s(buf + pos, bufSize - pos, currentNode->GetID().data(), sizeOfFileId);
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
