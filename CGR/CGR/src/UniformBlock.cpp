#include "UniformBlock.h"

#include "OpenGlLayer.h"
#include "LogFile.h"

UniformBlock::UniformBlock() :
	m_Buffer(nullptr)
{
}

UniformBlock::~UniformBlock()
{
}

void UniformBlock::Close()
{
	SAFE_DELETE_ARRAY(m_Buffer);
	OpenGLLayer::clean_GL_buffer(&m_UBO, 1);
}

bool UniformBlock::IsBound() const
{
	return m_Bound;
}

bool UniformBlock::ShouldUpdateGPU() const
{
	return m_ShouldUpdatGPU;
}

std::vector<const char*> UniformBlock::GetUniformNames()
{
	// Build and return 
	std::vector<const char*> names;
	for (auto i = m_Uniforms.begin(); i != m_Uniforms.end(); ++i)
	{
		names.push_back(i->first.c_str());
	}

	return names;
}

bool UniformBlock::AddUniform(const std::string& uniformname)
{
	for (auto i = m_Uniforms.begin(); i != m_Uniforms.end(); ++i)
	{
		std::string name = (i->first);
		if (name == uniformname)
		{
			WRITE_LOG("Tried to add same uniform name", "error");
			return false;
		}
	}

	m_Uniforms[uniformname] = BlockData();

	return true;
}

bool UniformBlock::allocBlock(GLuint* shaderProg, const char* name)
{
	// Dbl checkd
	if (m_Bound)
		return false;

	// TODO : Store our name, out interface through manager
 	m_UboIndex = glGetUniformBlockIndex(*shaderProg, name);

	if (m_UboIndex == GL_INVALID_INDEX)
	{
		WRITE_LOG("Uniform block error", "warning");
		return false;
	}

	// Find out buff size
	m_BuffSize = -1;
	glGetActiveUniformBlockiv(*shaderProg, m_UboIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &m_BuffSize);

	if (m_BuffSize <= 0)
	{
		WRITE_LOG("Buff size of uniform block invalid", "error");
		return false;
	}

	// How many uniforms in this block
	int numUniformsInBlock = -1;
	glGetActiveUniformBlockiv(*shaderProg, m_UboIndex, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &numUniformsInBlock);

	// This should have been pre-asigned
	std::vector<const char*> names = this->GetUniformNames();

	// Check that this matches
	if (numUniformsInBlock != (int)names.size())
	{
		std::stringstream ss;
		ss << "The uniform block: " << name << " number of uniforms found in shader compilation does not match those set in the uniform block object";
		WRITE_LOG(ss.str(), "error");
		return false;
	}

	// Build all the info needed for the buffers now that all relevant data has been aquired
	GLuint* indices = new GLuint[numUniformsInBlock];
	GLint* sizes = new GLint[numUniformsInBlock];
	GLint* offsets = new GLint[numUniformsInBlock];
	GLint* types = new GLint[numUniformsInBlock];

	// Get Indices
	glGetUniformIndices(*shaderProg, numUniformsInBlock, names.data(), indices);

	// Get Offsets
	glGetActiveUniformsiv(*shaderProg, numUniformsInBlock, indices, GL_UNIFORM_OFFSET, offsets);

	// Get Size
	glGetActiveUniformsiv(*shaderProg, numUniformsInBlock, indices, GL_UNIFORM_SIZE, sizes);

	// Get Types
	glGetActiveUniformsiv(*shaderProg, numUniformsInBlock, indices, GL_UNIFORM_TYPE, types);

	// Assign the info of each sub block with info from GL
	for (int i = 0; i < numUniformsInBlock; ++i)
	{
		this->addBlockData(names[i], sizes[i] * OpenGLLayer::glTypeSize(types[i]), offsets[i]);
	}

	// Allocate Buffer
	m_Buffer = new byte[m_BuffSize];
	memset(m_Buffer, 0, m_BuffSize);

	// Set Data to GPU -- TODO Map
	glGenBuffers(1, &m_UBO);
	glBindBuffer(GL_UNIFORM_BUFFER, m_UBO);
	
	glBufferData(GL_UNIFORM_BUFFER, m_BuffSize, m_Buffer, GL_DYNAMIC_DRAW);
	
	//glBufferData(GL_UNIFORM_BUFFER, m_BuffSize, NULL, GL_WRITE_ONLY);
	//m_Buffer = (byte*)glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
	//glUnmapBuffer(GL_UNIFORM_BUFFER);

	glBindBufferBase(GL_UNIFORM_BUFFER, m_UboIndex, m_UBO);

	// flag this
	m_Bound = true;

	// Clean up temp
	delete[] indices;
	delete[] sizes;
	delete[] offsets;
	delete[] types;

	return true;
}

void UniformBlock::SetValue(const std::string& uniformName, void* value)
{
	auto block = m_Uniforms.find(uniformName);
	if (block != m_Uniforms.end())
	{
		memcpy(m_Buffer + block->second.offset, value, block->second.size);
		m_ShouldUpdatGPU = true;
	}
}

bool UniformBlock::addBlockData(const std::string& uniformName, GLint size, GLint offset)
{
	auto result = m_Uniforms.find(uniformName);

	if (result == m_Uniforms.end())
	{
		WRITE_LOG("Uniform name does not exist", "error");
		return false;
	}

	result->second.size = size;
	result->second.offset = offset;

	return true;
}

void UniformBlock::Bind()
{
	glBindBuffer(GL_UNIFORM_BUFFER, m_UBO);
	glBufferData(GL_UNIFORM_BUFFER, m_BuffSize, m_Buffer, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, m_UboIndex, m_UBO);
	//glBindBuffer(GL_UNIFORM_BUFFER, 0);
	m_ShouldUpdatGPU = false;
}
