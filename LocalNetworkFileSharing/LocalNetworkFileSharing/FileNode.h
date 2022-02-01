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

	const std::string& GetID()
	{
		return ID;
	}

	void print()
	{
		std::cout << FileName << std::endl;
	}

private:
	std::string FileName;
	std::string ID;
};