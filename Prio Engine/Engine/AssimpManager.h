#ifndef ASSIMPMODEL_H
#define ASSIMPMODEL_H

#include "Dependencies/assimp-3.3.1/include/assimp/Importer.hpp"
#include "Dependencies/assimp-3.3.1/include\assimp/scene.h"
#include "Dependencies/assimp-3.3.1/include/assimp/postprocess.h"

#include "VertexTypeManager.h"

	class CAssimpManager
	{
	public:
		CAssimpManager();
		~CAssimpManager();

		bool LoadMyModelFromFile(char* filepath);
	private:
		CLogger* mpLogger;
	};
#endif