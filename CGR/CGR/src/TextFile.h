#ifndef __TEXT_FILE_READER_H__
#define __TEXT_FILE_READER_H__

#include "types.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

class TextFile
{
public:
	TextFile();
	~TextFile();

	bool LoadFileAsLinesToBuffer(const std::string& filename);
	static std::string LoadFileIntoStr(const std::string& filename);

	std::string GetDataBuffer(dword index) const;
	int32 GetDataAsInt(dword idx) const;

	const std::vector<std::string>& GetBuffer() const { return buffer; }

private:
	std::vector<std::string> buffer;
};

#endif

