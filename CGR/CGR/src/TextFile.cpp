#include "TextFile.h"
#include <cassert>
#include "utils.h"

TextFile::TextFile() :
	buffer()
{
}

TextFile::~TextFile()
{
	if (!buffer.empty())
		buffer.clear();
}

bool TextFile::LoadFileAsLinesToBuffer(const std::string& filename)
{
	buffer.clear();
	std::ifstream fileIn(filename);

	if (fileIn.fail())
	{
		std::cout << "Error:: TextFile Reader failed to open the file.\n";
		return false;
	}

	std::string word;
	while (std::getline(fileIn, word))
	{
		buffer.push_back(word);
	}

	fileIn.close();
	return true;
}

std::string TextFile::LoadFileIntoStr(const std::string& filename)
{
	std::ifstream fp;
	fp.open(filename, std::ifstream::in);
	if (fp.is_open() == false)
	{
		return "";
	}

	std::stringstream ss;
	ss << fp.rdbuf();
	return ss.str();
}

std::string TextFile::GetDataBuffer(dword index) const
{
	assert(index < buffer.size());
	return buffer[index];
}

int32 TextFile::GetDataAsInt(dword idx) const
{
	assert(idx < buffer.size());
	return atoi(buffer[idx].c_str());
}