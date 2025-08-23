#ifndef ASSET_H
#define ASSET_H

#include <vector>
#include <string>
#include <glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "types.h"
#include "camera.h"

struct ShaderSources
{
	std::string vertex;
	std::string fragment;

	ShaderSources(std::string vertStr, std::string fraqStr)
	{
		vertex = vertStr;
		fragment = fraqStr;
	}

	ShaderSources() {};
};

struct ShaderUniforms
{
public:
	uint transformLoc;
	uint viewLoc;
	uint projLoc;
	uint mainColorLoc;
	uint mainLightColorLoc;
	uint mainLightDirLoc;
	uint timeLoc;

	ShaderUniforms()
	{
		std::memset(this, ~0x0, sizeof(*this));
	}

	ShaderUniforms(uint shaderId)
	{
		initBasicUniforms(shaderId);
	}

	void initBasicUniforms(uint shaderId);
};

struct Shader
{
public:
	uint id;
	ShaderUniforms uniforms;

	Shader(const char* name, bool combine = true,
		const std::vector<std::string>& definesV = {},
		const std::vector<std::string>& definesF = {});

	Shader(const char* nameV, const char* nameF,
		const std::vector<std::string>& definesV = {},
		const std::vector<std::string>& definesF = {});

	Shader() 
	{
		std::memset(this, ~0x0, sizeof(*this));
	};

	uint getLoc(const GLchar* uName);

	void setTransformUniforms(const Camera& camera, const glm::mat4& transform);

	void setMainLightUniforms(const glm::vec3& lightColor, const glm::vec3& lightDir);

	void setMainColorUniform(const glm::vec3& color);
	void setCustomUniformF3(uint uniformLoc, const glm::vec3& color);
	void setCustomUniformF3(uint uniformLoc, const float color[]);
	void setCustomUniformF2(uint uniformLoc, const float color[]);
	void setCustomUniformM4(uint uniformLoc, const glm::mat4& matrix);
	void setCustomUniformF(uint uniformLoc, float value);
};

struct VertexData
{
public:
	glm::vec3 position;
	glm::vec2 texCoords;
	glm::vec3 normal;
	glm::vec3 tangent;
	glm::vec3 bitangent;

	VertexData() {};

	VertexData(glm::vec3 pos, glm::vec2 uv, glm::vec3 normal) :
		position(pos),
		texCoords(uv),
		normal(normal) {
	};
};

class Mesh {
public:
	enum Primitive
	{
		Quad
	};

	// mesh data
	std::vector<VertexData>       vertices;
	std::vector<uint>						indices;

	uint  VAO, VBO, EBO;

	Mesh(const std::vector<VertexData>& verts, const std::vector<uint>& inds) : vertices(verts), indices(inds)
	{
		setupMesh();
	};

	Mesh() // quad
	{
		vertices =
		{// positions											// uv					// normals
		VertexData(glm::vec3(1.f,  1.f, 0.f),         glm::vec2(1.f, 1.f),    glm::vec3(0.0f,  0.0f, 1.0f)),  // ( 1, 1, 0)
		VertexData(glm::vec3(1.f,  -1.f, 0.f),        glm::vec2(1.f, 0.f),    glm::vec3(0.0f,  0.0f, 1.0)),  // ( 1,-1, 0)
		VertexData(glm::vec3(-1.f, -1.f, 0.f),        glm::vec2(0.f, 0.f),	glm::vec3(0.0f,  0.0f, 1.0)),  // (-1,-1, 0)
		VertexData(glm::vec3(-1.f,  1.f, 0.f),        glm::vec2(0.f, 1.f),    glm::vec3(0.0f,  0.0f, 1.0)),  // (-1, 1, 0)
		};

		indices = {
			0, 3, 2,
			0, 2, 1
		};

		setupMesh();
	}

	void draw();
private:
	//  render data

	void setupMesh();
};

class Model
{
public:

	std::vector<Mesh> meshes;

	Model() {};

	Model(const std::vector<Mesh>& meshes) : meshes(meshes) {}

	Model(Mesh::Primitive primitive)
	{
		if (primitive == Mesh::Primitive::Quad)
		{
			meshes.push_back(Mesh());
		}
	}

	void draw();
};

class Texture
{
public:
	int width;
	int height;
	uint id;

	Texture(int width, int height, GLenum format, GLenum type);

	Texture() {std::memset(this, ~0x0, sizeof(*this));};
};
#endif