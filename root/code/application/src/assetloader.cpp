#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <vector>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "assetloader.h"
#include "graphics.h"
#include "config.hpp"

using namespace std;

enum ShaderParseMode
{
	ShaderParseMode_NONE = -1,
	ShaderParseMode_VERT = 0,
	ShaderParseMode_FRAG = 1
};

std::string readFile(const std::string& path)
{
	std::ifstream in(path);

	if (!in)
	{
		throw std::runtime_error("Cannot open " + path);
	}

	std::ostringstream s; 
	
	s << in.rdbuf(); 

	return s.str();
}

extern Graphics::ShaderSources AssetLoader::loadShaderFiles(const std::string& baseName)
{
	//#TODO: change this to relative path or embed shaders into the exe!!!
	std::string dir = std::string(DEFAULT_ASSET_DIR) + "/shaders";

	std::string vPath = dir + "/" + baseName + "V.shader";
	std::string fPath = dir + "/" + baseName + "F.shader";

	Graphics::ShaderSources shaderSources(readFile(vPath), readFile(fPath));

	return shaderSources;
}

Graphics::Mesh processMesh(aiMesh* mesh, const aiScene* scene)
{
	std::vector<Graphics::VertexData>	vertices;
	std::vector<uint>					 indices;

	for (uint i = 0; i < mesh->mNumVertices; ++i)
	{
		Graphics::VertexData vertexData;

		// positions
		glm::vec3 pos = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
		vertexData.position = pos;

		// normals
		if (mesh->HasNormals())
		{
			glm::vec3 n = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
			vertexData.normal = n;
		}

		// tangent and bitangent
		if (mesh->HasTangentsAndBitangents())
		{
			vertexData.tangent = glm::vec3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);

			vertexData.bitangent = glm::vec3(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z);
		}

		// texture coords (Assimp stores up to 8 sets; use the first if present)
		if (mesh->mTextureCoords[0])
		{
			glm::vec2 uv = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
			vertexData.texCoords = uv;
		}
		else
		{
			vertexData.texCoords = glm::vec2(0.0f);
		}

		vertices.push_back(vertexData);
	}


	// now walk through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
	for (uint i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		// retrieve all indices of the face and store them in the indices vector
		for (uint j = 0; j < face.mNumIndices; j++)
		{
			indices.push_back(face.mIndices[j]);
		}
	}

	// return a mesh object created from the extracted mesh data
	Graphics::Mesh oglMesh = Graphics::Mesh(vertices, indices/*, textures*/);

	return oglMesh;
}

// processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
void processNode(vector<Graphics::Mesh>* pMeshes, aiNode* node, const aiScene* scene)
{
	// process each mesh located at the current node
	for (uint i = 0; i < node->mNumMeshes; i++)
	{
		// the node object only contains indices to index the actual objects in the scene. 
		// the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		pMeshes->push_back(processMesh(mesh, scene));
	}
	// after we've processed all of the meshes (if any) we then recursively process each of the children nodes
	for (uint i = 0; i < node->mNumChildren; i++)
	{
		return processNode(pMeshes, node->mChildren[i], scene);
	}

}

extern Graphics::Model AssetLoader::loadModel(string const& nameWithExtension)
{
	std::vector<Graphics::Mesh> meshes;

	std::string dir = std::string(DEFAULT_ASSET_DIR) + "/models";
	std::string path = dir + "/" + nameWithExtension;

	// read file via ASSIMP
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
	// check for errors
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
	{
		cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
		return Graphics::Model(meshes);
	}
	// retrieve the directory path of the filepath
	string directory = dir.substr(0, dir.find_last_of('/'));

	// process ASSIMP's root node recursively

	processNode(&meshes, scene->mRootNode, scene);

	Graphics::Model model = Graphics::Model(meshes);

	return model;
}
