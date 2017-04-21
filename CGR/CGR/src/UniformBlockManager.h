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
	UniformBlock* GetBlock(const std::string& blockname);
	bool CheckBlockUniformExists(const char* name);

private:
	bool CreateBlock(const std::string& blockName, const std::vector<std::string>& uniformNames);
	void Close();

private:
	friend class							Renderer;
	std::map<std::string, UniformBlock*>	m_Blocks;
};

#endif