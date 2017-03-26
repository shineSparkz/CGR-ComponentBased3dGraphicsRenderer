#include "UniformBlockManager.h"

#include "LogFile.h"
#include "UniformBlock.h"

UniformBlockManager::~UniformBlockManager()
{
}

void UniformBlockManager::Close()
{
	for (auto i = m_Blocks.begin(); i != m_Blocks.end(); ++i)
	{
		SAFE_CLOSE(i->second);
	}

	m_Blocks.clear();
}

bool UniformBlockManager::CreateBlock(const std::string& blockName, const std::vector<std::string>& uniformNames)
{
	// First see if this block exists
	auto result = m_Blocks.find(blockName);
	if (result != m_Blocks.end())
	{
		WRITE_LOG("Tried to create uniform block twice: " + blockName, "error");
		return false;
	}

	m_Blocks[blockName] = new UniformBlock();

	for (auto i = uniformNames.begin(); i != uniformNames.end(); ++i)
	{
		if (!m_Blocks[blockName]->AddUniform((*i)))
		{
			return false;
		}

		WRITE_LOG("Uniform block manager added " + (*i) + " to " + blockName + " uniform block", "good");
	}

	return true;
}

UniformBlock* UniformBlockManager::GetBlock(const std::string& blockname)
{
	auto result = m_Blocks.find(blockname);
	return result == m_Blocks.end() ? nullptr : result->second;
}

bool UniformBlockManager::CheckBlockUniformExists(const char* checkName)
{
	for (auto block = m_Blocks.begin(); block != m_Blocks.end(); ++block)
	{
		auto names = block->second->GetUniformNames();
		for (auto name = names.begin(); name != names.end(); ++name)
		{
			if (std::string((*name)) == std::string(checkName))
			{
				return true;
			}
		}
	}

	return false;
}