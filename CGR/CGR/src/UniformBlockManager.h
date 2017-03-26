#ifndef __UNIFORM_BLOCK_MANAGER_H__
#define __UNIFORM_BLOCK_MANAGER_H__

#include "Singleton.h"
#include <map>
#include <vector>

class UniformBlock;

class UniformBlockManager : public Singleton<UniformBlockManager>
{
public:
	~UniformBlockManager();
	void Close();
	UniformBlock* GetBlock(const std::string& blockname);
	bool CreateBlock(const std::string& blockName, const std::vector<std::string>& uniformNames);

	bool CheckBlockUniformExists(const char* name);

private:
	std::map<std::string, UniformBlock*> m_Blocks;
};

#endif