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

	static void Deserialize(uint64_t dataSize, char* data)
	{
		uint64_t pos = 0;

		// Always has to be a folder as the root
		assert(data[pos] == (char)SerializeFlags::Directory);
		++pos;

		uint64_t* rootSize = reinterpret_cast<uint64_t*>(&(data[pos]));
		pos += sizeof(uint64_t);

		std::string rootId = std::string(&(data[pos]), *rootSize);

		SharedFolder newSharedFolder;

		newSharedFolder.Root = std::make_unique<DirectoryNode>(rootId);

		return;
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

	SharedFolder()
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
