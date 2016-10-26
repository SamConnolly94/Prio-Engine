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

		bool LoadModelFromFile(const std::string& pFile);
		std::list<aiMesh* > GetMeshes();
		aiMesh* GetLastLoadedMesh();
		const aiScene* GetScene() { return mpScene;};
	private:
		std::list<aiMesh* > mpMeshes;
		const aiScene* mpScene;
		CLogger* mpLogger;
	};
#endif