#include "AssimpManager.h"

CAssimpManager::CAssimpManager()
{
}


CAssimpManager::~CAssimpManager()
{
}

aiMesh* CAssimpManager::LoadModelFromFile(const std::string& pFile)
{
	Assimp::Importer importer;

	mpLogger->GetLogger().WriteLine("Attempting to open " + pFile + " using Assimp.");

	// Read in the file, store this mesh in the scene.
	const aiScene* scene = importer.ReadFile(pFile,
											/* aiProcess_ConvertToLeftHanded |*/
											aiProcess_JoinIdenticalVertices |
											aiProcess_Triangulate |
											aiProcess_SortByPType);
	

	// If scene hasn't been initialised then something has gone wrong!
	if (!scene)
	{
		mpLogger->GetLogger().WriteLine(importer.GetErrorString());
		mpLogger->GetLogger().WriteLine("Failed to create scene.");
		return nullptr;
	}
	
	// Store the mesh on a list.
	int i = 0; // Adjust the number of meshes so we can apply to an array.
	
	mpLogger->GetLogger().WriteLine(pFile + " has: " + std::to_string(scene->mMeshes[i]->mNumFaces) + " faces, " + std::to_string(scene->mMeshes[i]->mFaces[0].mNumIndices) + " indices and "+ std::to_string(scene->mMeshes[i]->mNumVertices) + " vertices.");
	mpLogger->GetLogger().WriteLine("No errors were found, assumed success of model loading.");

	return scene->mMeshes[i];
}

const aiScene * CAssimpManager::GetScene()
{
	Assimp::Importer importer;

	return importer.GetScene();
}
