#include "AnimMesh.h"

#include <map>
#include "Material.h"
#include "Texture.h"
#include "ResourceManager.h"
#include "ShaderProgram.h"
#include "ResId.h"
#include "OpenGlLayer.h"


anim_t AnimMesh::animlist[21] =
{
	// first, last, fps
	{ 0,  39,  9 },	// STAND
	{ 40,  45, 10 },	// RUN
	{ 46,  53, 10 },	// ATTACK
	{ 54,  57,  7 },	// PAIN_A
	{ 58,  61,  7 },	// PAIN_B
	{ 62,  65,  7 },	// PAIN_C
	{ 66,  71,  7 },	// JUMP
	{ 72,  83,  7 },	// FLIP
	{ 84,  94,  7 },	// SALUTE
	{ 95, 111, 10 },	// FALLBACK
	{ 112, 122,  7 },	// WAVE
	{ 123, 134,  6 },	// POINTIING
	{ 135, 153, 10 },	// CROUCH_STAND
	{ 154, 159,  7 },	// CROUCH_WALK
	{ 160, 168, 10 },	// CROUCH_ATTACK
	{ 196, 172,  7 },	// CROUCH_PAIN
	{ 173, 177,  5 },	// CROUCH_DEATH
	{ 178, 183,  7 },	// DEATH_FALLBACK
	{ 184, 189,  7 },	// DEATH_FALLFORWARD
	{ 190, 197,  7 },	// DEATH_FALLBACKSLOW
	{ 198, 198,  5 },	// BOOM
};

std::string sMD2AnimationNames[MAX_ANIMATIONS] =
{
	"Stand",
	"Run",
	"Attack",
	"Pain A",
	"Pain B",
	"Pain C",
	"Jump",
	"Flip",
	"Salute",
	"Fallback",
	"Wave",
	"Pointing",
	"Crouch Stand",
	"Crouch Walk",
	"Crouch Attack",
	"Crouch Pain",
	"Crouch Death",
	"Death Fallback",
	"Death Fall Forward",
	"Death Fallback Slow",
	"Boom"
};

#pragma warning( once : 4305 )

float3 anorms[NUM_VERT_NORMALS] =
{
#include "anorms.h"
};

AnimMesh::AnimMesh() :
	m_RenderModes(),
	m_NumRenderVertices(),
	m_VAO(0)
{
}

AnimMesh::~AnimMesh()
{
}

void AnimMesh::Close()
{
	for (auto i = m_AnimData.begin(); i != m_AnimData.end(); ++i)
	{
		OpenGLLayer::clean_GL_buffer(&i->vbo, 1);
	}

	OpenGLLayer::clean_GL_vao(&m_VAO, 1);
}

bool AnimMesh::Load(const char* sFilename, ResourceManager* resMan, unsigned materialSet)
{
	// Load Mesh file
	FILE* mesh_file = fopen(sFilename, "rb");

	if (!mesh_file)
	{
		WRITE_LOG("anim mesh file not found", "error");
		return false;
	}

	// Read header
	md2_t header;
	fread(&header, sizeof(md2_t), 1, mesh_file);

	// Read Data
	char* buffer = new char[header.num_frames * header.framesize];
	fseek(mesh_file, header.ofs_frames, SEEK_SET);
	fread(buffer, sizeof(char), header.num_frames * header.framesize, mesh_file);

	// Data for extraction
	std::vector<std::vector<Vec3>>	vertices;
	std::vector<std::vector<int>>	normals;
	std::vector<int>				glCommands;
	std::vector<Vec2>				texcoords;

	std::vector<AnimVert>			BUFF_DATA;

	// Allocate space for vertices
	vertices.resize(header.num_frames, std::vector<Vec3>(header.num_xyz));

	// And normals
	normals.resize(header.num_frames, std::vector<int>(header.num_xyz));

	// Extract vertices and normals from frame data
	for (int i = 0; i < header.num_frames; ++i)
	{
		// Convert buffer to frame_t pointer
		frame_t* frame_ptr = (frame_t*)&buffer[header.framesize * i];

		// Loop Each Vertex in this frame
		for (int j = 0; j < header.num_xyz; ++j)
		{
			vertices[i][j].x = frame_ptr->translate[0] + (float(frame_ptr->verts[j].v[0]) * frame_ptr->scale[0]);
			vertices[i][j].y = frame_ptr->translate[1] + (float(frame_ptr->verts[j].v[1]) * frame_ptr->scale[1]);
			vertices[i][j].z = frame_ptr->translate[2] + (float(frame_ptr->verts[j].v[2]) * frame_ptr->scale[2]);

			normals[i][j] = frame_ptr->verts[j].lightnormalindex;
		}
	}

	// Read in OpenGL commands
	glCommands.resize(header.num_glcmds);
	fseek(mesh_file, header.ofs_glcmds, SEEK_SET);
	fread(&glCommands[0], sizeof(int), header.num_glcmds, mesh_file);

	// And start with creating VBOs for vertices, textue coordinates and normals
	m_AnimData.resize(header.num_frames);
	for (int i = 0; i < header.num_frames; ++i)
	{
		glGenBuffers(1, &m_AnimData[i].vbo);
	}

	// Only need one VBO for texcoords
	GLuint texVbo;
	glGenBuffers(1, &texVbo);

	// Loop through GL commands and populate buffer data
	int i = 0;
	while (1)
	{
		int action = glCommands[i];
		if (action == 0)
			break;

		// Extract rendering mode
		int renderMode = action < 0 ? GL_TRIANGLE_FAN : GL_TRIANGLE_STRIP; 

		// And number of vertices
		int numVertices = action < 0 ? -action : action; 
		
		i++;

		// Remember the values
		m_RenderModes.push_back(renderMode);
		m_NumRenderVertices.push_back(numVertices);

		for (int j = 0; j < numVertices; ++j)
		{
			// Extract texture coordinates
			float s = *((float*)(&glCommands[i++])); 
			float t = *((float*)(&glCommands[i++]));
			// Flip t, because it is (from some reasons) stored from top to bottom
			t = 1.0f - t; 

			int vi = glCommands[i++];

			// Add texture coords to VBO
			texcoords.push_back(Vec2(s, t));

			for (int k = 0; k < header.num_frames; ++k)
			{
				auto n = anorms[normals[k][vi]];

				m_AnimData[k].buffer.push_back(AnimVert{
					vertices[k][vi],
					Vec3(n[0],n[1],n[2]) });
			}
		}
	}

	// Now all necessary data are extracted, let's create VAO for rendering MD2 model
	glGenVertexArrays(1, &m_VAO);
	glBindVertexArray(m_VAO);

	for (int i = 0; i < header.num_frames; ++i)
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_AnimData[i].vbo);
		glBufferData(GL_ARRAY_BUFFER, m_AnimData[i].buffer.size() * sizeof(AnimVert), m_AnimData[i].buffer.data(), GL_STATIC_DRAW);

		// Get min, max, and centre vertices
		Vec3 tempMin((float)MAX_TYPE(float));
		Vec3 tempMax((float)-MAX_TYPE(float));

		for (int j = 0; j < m_AnimData[i].buffer.size(); ++j)
		{
			if ((m_AnimData[i].buffer[j].pos.x < tempMin.x))
			{
				tempMin.x = m_AnimData[i].buffer[j].pos.x;
			}
			if ((m_AnimData[i].buffer[j].pos.y < tempMin.y))
			{
				tempMin.y = m_AnimData[i].buffer[j].pos.y;
			}
			if ((m_AnimData[i].buffer[j].pos.z < tempMin.z))
			{
				tempMin.z = m_AnimData[i].buffer[j].pos.z;
			}

			if ((m_AnimData[i].buffer[j].pos.x > tempMax.x))
			{
				tempMax.x = m_AnimData[i].buffer[j].pos.x;
			}
			if ((m_AnimData[i].buffer[j].pos.y > tempMax.y))
			{
				tempMax.y = m_AnimData[i].buffer[j].pos.y;
			}
			if ((m_AnimData[i].buffer[j].pos.z > tempMax.z))
			{
				tempMax.z = m_AnimData[i].buffer[j].pos.z;
			}
		}

		m_AnimData[i].min = tempMin;
		m_AnimData[i].max = tempMax;
		m_AnimData[i].centre = (tempMin + tempMax) / 2.0f;

		m_AnimData[i].buffer.clear();
	}

	// Vertex and normals data parameters
	glBindBuffer(GL_ARRAY_BUFFER, m_AnimData[0].vbo);

	// Vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(Vec3), 0);
	
	// Vertices for next keyframe, now we can set it to same VBO
	glEnableVertexAttribArray(3); 
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(Vec3), 0);

	// Normal vectors
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(Vec3), (void*)(sizeof(Vec3)));

	// Normals for next keyframe, now we can set it to same VBO
	glEnableVertexAttribArray(4); 
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(Vec3), (void*)(sizeof(Vec3)));

	// Texture coordinates
	glBindBuffer(GL_ARRAY_BUFFER, texVbo);
	glBufferData(GL_ARRAY_BUFFER, texcoords.size() * sizeof(Vec2), texcoords.data(), GL_STATIC_DRAW);

	// Texture coordinates
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vec2), 0);

	// Find texture name (modelname.jpg, modelname.png...)
	std::string sPath = sFilename;
	int index = sPath.find_last_of("\\/");
	std::string sDirectory = index != -1 ? sPath.substr(0, index + 1) : "";
	std::string sPureFilename = index != -1 ? sPath.substr(index + 1) : sFilename;

	std::string sTextureExtensions[] = { "jpg", "jpeg", "png", "bmp", "tga" };
	index = sPureFilename.find_last_of(".");
	if (index != -1)
	{
		std::string sStripped = sPureFilename.substr(0, index + 1);
		for (int i = 0; i < 5; ++i)
		{
			std::string sTry = sDirectory + sStripped + sTextureExtensions[i];

			Texture* t = resMan->LoadTexture(sTry, GL_TEXTURE0);
			if (t)
			{
				std::map<unsigned, Material*> mat;
				mat[0] = new Material();
				mat[0]->diffuse_map = t;
				resMan->AddMaterialSet(materialSet, mat);
				break;
			}
		}
	}

	fclose(mesh_file);

	SAFE_DELETE_ARRAY(buffer);
	return true;
}
