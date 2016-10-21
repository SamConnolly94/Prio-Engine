#include "AssimpManager.h"

CAssimpManager::CAssimpManager()
{
}


CAssimpManager::~CAssimpManager()
{
}

bool CAssimpManager::LoadMyModelFromFile(char * filepath)
{
	Assimp::Importer importer;

	// Read in the file, store this mesh in the scene.
	const aiScene* mpScene = importer.ReadFile(filepath,
								aiProcess_CalcTangentSpace |
								aiProcess_Triangulate |
								aiProcess_JoinIdenticalVertices |
								aiProcess_SortByPType);

	// If scene hasn't been initialised then something has gone wrong!
	if (!mpScene)
	{
		mpLogger->GetLogger().WriteLine("Failed to create scene.");
		mpLogger->GetLogger().WriteLine(importer.GetErrorString());
		return false;
	}
}
