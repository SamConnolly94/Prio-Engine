#include "AssimpManager.h"

CAssimpManager::CAssimpManager()
{
}


CAssimpManager::~CAssimpManager()
{
}

bool CAssimpManager::LoadModelFromFile(const std::string& pFile)
{
	Assimp::Importer importer;

	// Read in the file, store this mesh in the scene.
	mpScene = importer.ReadFile(pFile,
								aiProcess_CalcTangentSpace |
								aiProcess_Triangulate |
								aiProcess_JoinIdenticalVertices |
								aiProcess_SortByPType);

	// If scene hasn't been initialised then something has gone wrong!
	if (!mpScene)
	{
		mpLogger->GetLogger().WriteLine(importer.GetErrorString());
		mpLogger->GetLogger().WriteLine("Failed to create scene.");
		return false;
	}

	// Store the mesh on a list.
	int i = mpScene->mNumMeshes - 1; // Adjust the number of meshes so we can apply to an array.
	mpMeshes.push_back(mpScene->mMeshes[i]);

	return true;
}

/* Returns a list of all meshes loaded by Assimp. */
std::list<aiMesh* > CAssimpManager::GetMeshes()
{
	return mpMeshes;
}

aiMesh* CAssimpManager::GetLastLoadedMesh()
{
	return mpMeshes.back();
}
