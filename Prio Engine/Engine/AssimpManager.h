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

		bool LoadModelFromFile(char* filepath);
		std::list<aiMesh* > GetMeshes();
		aiMesh* GetLastLoadedMesh();
	private:
		std::list<aiMesh* > mpMeshes;
		const aiScene* mpScene;
		CLogger* mpLogger;
	};
#endif