#pragma once

#include <string>
#include <format>
#include <iostream>

#include "MyRandom.h"


class FileNode
{
public:
	FileNode(std::string fileName)
	{
		FileName = fileName;
		static MyRandom idGenerator = MyRandom(false);
		ID = FileName + "_" + std::format("{:020}", idGenerator());
	}

	static std::unique_ptr<FileNode> FromId(std::string id)
	{
		std::unique_ptr<FileNode> newFileNode;
		newFileNode->ID = id;
		newFileNode->FileName = id.substr(0, id.find_last_of("_"));
		return std::move(newFileNode);
	}

	const std::string& GetID()
	{
		return ID;
	}

	void print()
	{
		std::cout << FileName << std::endl;
	}

private:

	FileNode()
	{ }

	std::string FileName;
	std::string ID;
};