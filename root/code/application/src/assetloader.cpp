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
	//#TODO: change this to relative path or embed shaders into the exe
	std::string dir = std::string(DEFAULT_ASSET_DIR) + "/shaders";

	std::string vPath = dir + "/" + baseName + "V.shader";
	std::string fPath = dir + "/" + baseName + "F.shader";

	Graphics::ShaderSources shaderSources(readFile(vPath), readFile(fPath));

	return shaderSources;
}

extern Graphics::ShaderSources AssetLoader::loadShaderFiles(const std::string& baseNameVert, const std::string& baseNameFrag)
{
	std::string dir = std::string(DEFAULT_ASSET_DIR) + "/shaders";

	std::string vPath = dir + "/" + baseNameVert + "V.shader";
	std::string fPath = dir + "/" + baseNameFrag + "F.shader";

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


	for (uint i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (uint j = 0; j < face.mNumIndices; j++)
		{
			indices.push_back(face.mIndices[j]);
		}
	}

	Graphics::Mesh oglMesh = Graphics::Mesh(vertices, indices/*, textures*/);

	return oglMesh;
}

void processNode(vector<Graphics::Mesh>* pMeshes, aiNode* node, const aiScene* scene)
{
	for (uint i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		pMeshes->push_back(processMesh(mesh, scene));
	}

	for (uint i = 0; i < node->mNumChildren; i++)
	{
		return processNode(pMeshes, node->mChildren[i], scene);
	}
}

extern Graphics::Model AssetLoader::loadModel(string const& nameWithExtension)
{
	std::string dir = std::string(DEFAULT_ASSET_DIR) + "/models";
	std::string path = dir + "/" + nameWithExtension;

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
		return Graphics::Model();
	}

	string directory = dir.substr(0, dir.find_last_of('/'));

	std::vector<Graphics::Mesh> meshes;

	// process ASSIMP's root node recursively
	processNode(&meshes, scene->mRootNode, scene);

	Graphics::Model model = Graphics::Model(meshes);

	return model;
}
