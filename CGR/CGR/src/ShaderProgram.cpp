#include "ShaderProgram.h"

#include "OpenGlLayer.h"
#include "UniformBlockManager.h"
#include "UniformBlock.h"


ShaderProgram::ShaderProgram() :
	m_ShaderProgram(0),
	m_Uniforms()
{
}

ShaderProgram::~ShaderProgram()
{
}

bool ShaderProgram::CreateProgram(const std::vector<Shader>& shaders, const std::string& fragout_identifier, GLuint frag_loc)
{
	// Link shaders
	m_ShaderProgram = glCreateProgram();

	std::vector<Shader>::const_iterator it;
	bool has_fragment_shader = false;

	for (it = shaders.begin(); it != shaders.end(); ++it)
	{
		if (it->m_ShaderType == GL_FRAGMENT_SHADER)
			has_fragment_shader = true;

		glAttachShader(m_ShaderProgram, (*it).m_Shader);

		for (size_t j = 0; j < (*it).m_Attributes.size(); ++j)
		{
			glBindAttribLocation(m_ShaderProgram, (*it).m_Attributes[j].layout_location,
				(*it).m_Attributes[j].name.c_str());

			if (OpenGLLayer::check_GL_error())
			{
				WRITE_LOG("Error: binding attrib location in shader", "error");
				return false;
			}
		}
	}

	// Bind Frag
	if (has_fragment_shader)
	{
		glBindFragDataLocation(m_ShaderProgram, frag_loc, fragout_identifier.c_str());
		if (OpenGLLayer::check_GL_error())
		{
			WRITE_LOG("Error: binding frag data location in shader", "error");
			return false;
		}
	}

	glLinkProgram(m_ShaderProgram);

	GLint link_status = 0;
	glGetProgramiv(m_ShaderProgram, GL_LINK_STATUS, &link_status);

	if (link_status != GL_TRUE)
	{
		const int string_length = 1024;
		GLchar log[string_length] = "";
		glGetProgramInfoLog(m_ShaderProgram, string_length, NULL, log);
		std::stringstream logger;
		logger << log << std::endl;
		WRITE_LOG(logger.str(), "error");
		return false;
	}

	// -- Look for uniform blocks and loop through count
	int uniform_block_total = -1;
	glGetProgramiv(m_ShaderProgram, GL_ACTIVE_UNIFORM_BLOCKS, &uniform_block_total);
	for (int i = 0; i < uniform_block_total; ++i)
	{
		// Resolve name of this uniform block
		char name[100];
		int name_len = -1;
		glGetActiveUniformBlockName(m_ShaderProgram,
			GLuint(i),
			sizeof(name) - 1,
			&name_len,
			name);

		// Check if uniform block manager knows about this name
		UniformBlockManager* ubm = UniformBlockManager::Instance();
		if (ubm)
		{
			UniformBlock* block = ubm->GetBlock(name);

			if (block)
			{
				if (!block->IsBound())
				{
					// Create
					if (!block->allocBlock(&m_ShaderProgram, name))
					{
						// Log 
						return false;
					}
				}
				else
				{
					std::stringstream ss;
					ss << " The uniform block with name: " << name << " has already being bound";
					WRITE_LOG(ss.str(), "normal");
				}
			}
			else
			{
				std::stringstream ss;
				ss << " The uniform block with name: " << name << " does not exist in the uniform block manager";
				WRITE_LOG(ss.str(), "error");
			}
		}
	}

	// Populate m_Uniforms
	int total = -1;
	glGetProgramiv(m_ShaderProgram, GL_ACTIVE_UNIFORMS, &total);

	for (int i = 0; i < total; ++i)
	{
		int name_len = -1, num = -1;
		GLenum type = GL_ZERO;
		char name[100];

		glGetActiveUniform(
			m_ShaderProgram,
			GLuint(i),
			sizeof(name) - 1,
			&name_len,
			&num,
			&type,
			name);

		name[name_len] = 0;
		GLuint location = glGetUniformLocation(m_ShaderProgram, name);

		// Check not a uniform block
		UniformBlockManager* ubm = UniformBlockManager::Instance();
		if (ubm)
		{
			if (!ubm->CheckBlockUniformExists(name))
			{
				// Check it doesn't exist here
				if (m_Uniforms.find(name) == m_Uniforms.end())
				{
					Uniform* ufm = new Uniform(location, (UniformTypes)type);
					m_Uniforms[name] = ufm;
				}
			}
		}
	}

	// Keep these in-case want to reload
	m_Shaders = shaders;

	return true;
}

bool ShaderProgram::Reload()
{
	std::vector<Shader> temp_shaders;
	
	for (auto shader = m_Shaders.begin(); shader != m_Shaders.end(); ++shader)
	{
		if (!shader->LoadShader(shader->m_SourceFile.c_str()))
		{
			return false;
		}

		temp_shaders.push_back(*shader);
	}

	return this->CreateProgram(temp_shaders, "frag_colour", 0);
}

Uniform* ShaderProgram::GetUniformByName(const std::string& name)
{
	auto i = m_Uniforms.find(name);
	return ((i != m_Uniforms.end()) ? i->second : nullptr);
}

void ShaderProgram::Close()
{
	for (auto s = m_Shaders.begin(); s != m_Shaders.end(); ++s)
	{
		s->Close();
	}

	for (auto i = m_Uniforms.begin(); i != m_Uniforms.end(); ++i)
	{
		SAFE_DELETE(i->second);
	}

	m_Uniforms.clear();

	// Clean GL stuff
	OpenGLLayer::clean_GL_program(&m_ShaderProgram);
}

void ShaderProgram::Use()
{
	glUseProgram(m_ShaderProgram);
}