#ifndef ASSIMPMODEL_H
#define ASSIMPMODEL_H

#include "Dependencies/assimp-3.3.1/include/assimp/Importer.hpp"
#include "Dependencies/assimp-3.3.1/include\assimp/scene.h"
#include "Dependencies/assimp-3.3.1/include/assimp/postprocess.h"

#include "VertexTypeManager.h"
#include <list>

	class CAssimpManager
	{
	public:
		CAssimpManager();
		~CAssimpManager();

		const aiMesh* LoadModelFromFile(const std::string& pFile);
		const aiScene* GetScene();
	private:
		CLogger* mpLogger;
	};
#endif