#include "AssimpManager.h"

CAssimpManager::CAssimpManager()
{
}


CAssimpManager::~CAssimpManager()
{
}

bool CAssimpManager::LoadModelFromFile(char * filepath)
{
	Assimp::Importer importer;

	// Read in the file, store this mesh in the scene.
	mpScene = importer.ReadFile(filepath,
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

	// Store the mesh on a list.
	for (unsigned int i = 0; i < mpScene->mNumMeshes; i++)
	{
		mpMeshes.push_back(mpScene->mMeshes[i]);
	}
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
