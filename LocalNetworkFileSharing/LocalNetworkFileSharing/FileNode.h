#pragma once

#include <string>
#include <format>
#include <iostream>

#include "MyRandom.h"


struct FileNode
{
	std::string FileName;
	std::string ID;

	FileNode(std::string fileName)
	{
		FileName = fileName;
		static MyRandom idGenerator = MyRandom(false);
		ID = FileName + "_" + std::format("{:020}", idGenerator());
	}

	void print()
	{
		std::cout << FileName << std::endl;
	}

public:
	const std::string& GetId()
	{
		return ID;
	}
};