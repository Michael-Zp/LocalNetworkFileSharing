#include <filesystem>

#include "SharedFolder.h"

namespace fs = std::filesystem;

std::string GrabFileList(SharedFolder sf)
{

}

int main(int argc, const char* argv)
{
	SharedFolder(fs::current_path().parent_path().parent_path().append("TestFiles"));

	return 0;
}