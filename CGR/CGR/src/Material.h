#ifndef __MATERIAL_H__
#define __MATERIAL_H__

#include "types.h"
#include "Texture.h"

struct Material
{
	Texture* diffuse_map{ nullptr };
	Texture* normal_map{ nullptr };

	void Bind()
	{
		if (diffuse_map)
			diffuse_map->Bind();
		if (normal_map)
			normal_map->Bind();
	}

	void Clean()
	{
		SAFE_DELETE(normal_map);
		SAFE_DELETE(diffuse_map);
	}
};

#endif
