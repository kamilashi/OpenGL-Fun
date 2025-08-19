#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <string>
#include "types.h"
#include "camera.h"

namespace Graphics 
{
	struct ShaderSources 
	{
		std::string vertex;
		std::string fragment;

		ShaderSources(std::string vertStr, std::string fraqStr)
		{
			vertex = vertStr;
			fragment = fraqStr;
		}
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

		void initBasicUniforms(uint shaderId)
		{
			transformLoc = glGetUniformLocation(shaderId, "uTransform");
			viewLoc = glGetUniformLocation(shaderId, "uView");
			projLoc = glGetUniformLocation(shaderId, "uProjection");

			mainColorLoc = glGetUniformLocation(shaderId, "uMainColor");

			mainLightColorLoc = glGetUniformLocation(shaderId, "uMainLightColor");
			mainLightDirLoc = glGetUniformLocation(shaderId, "uMainLightDirection");

			timeLoc = glGetUniformLocation(shaderId, "uTime");
		}
	};

	struct Shader
	{
	public:
		uint id;
		ShaderUniforms uniforms;

		Shader(const char* name);

		Shader(const char* nameV, const char* nameF);


		uint getLoc(const GLchar* uName)
		{
			return glGetUniformLocation(id, uName);
		}

		void setTransformUniforms(const Camera& camera, const glm::mat4& transform)
		{
			glUniformMatrix4fv(uniforms.transformLoc, 1, GL_FALSE, glm::value_ptr(transform));
			glUniformMatrix4fv(uniforms.viewLoc, 1, GL_FALSE, glm::value_ptr(camera.viewMatrix));
			glUniformMatrix4fv(uniforms.projLoc, 1, GL_FALSE, glm::value_ptr(camera.projMatrix));
		}

		void setMainLightUniforms(const glm::vec3& lightColor, const glm::vec3& lightDir)
		{
			glUniform3f(uniforms.mainLightColorLoc, lightColor.x, lightColor.y, lightColor.z);
			glUniform3f(uniforms.mainLightDirLoc, lightDir.x, lightDir.y, lightDir.z);
		}

		void setMainColorUniform(const glm::vec3& color)
		{
			glUniform3f(uniforms.mainColorLoc, color.x, color.y, color.z);
		}

		static void setCustomUniformF3(uint uniformLoc, const glm::vec3& color)
		{
			glUniform3f(uniformLoc, color.x, color.y, color.z);
		}

		static void setCustomUniformM4(uint uniformLoc, const glm::mat4& matrix)
		{
			glUniformMatrix4fv(uniformLoc, 1, GL_FALSE, glm::value_ptr(matrix));
		}

		static void setCustomUniformF(uint uniformLoc, float value)
		{
			glUniform1f(uniformLoc, value);
		}
	};

	struct VertexData 
	{
	public:
		glm::vec3 position;
		glm::vec2 texCoords;
		glm::vec3 normal;
		glm::vec3 tangent;
		glm::vec3 bitangent;

		VertexData(){}

		VertexData(glm::vec3 pos, glm::vec2 uv, glm::vec3 normal) :
			position(pos),
			texCoords(uv),
			normal(normal) {}
	};

	class Mesh {
	public:
		enum Primitive
		{
			Quad
		};

		// mesh data
		std::vector<Graphics::VertexData>       vertices;
		std::vector<uint>						indices;

		uint  VAO, VBO, EBO;

		Mesh(const std::vector<Graphics::VertexData>& verts, const std::vector<uint>& inds) : vertices(verts), indices(inds)
		{
			setupMesh();
		}

		Mesh() // quad
		{
			vertices = 
			{// positions											// uv					// normals
			Graphics::VertexData(glm::vec3(1.f,  1.f, 0.f),         glm::vec2(1.f, 1.f),    glm::vec3(0.0f,  0.0f, -1.0f)),  // ( 1, 1, 0)
			Graphics::VertexData(glm::vec3(1.f,  -1.f, 0.f),        glm::vec2(1.f, 0.f),    glm::vec3(0.0f,  0.0f, -1.0)),  // ( 1,-1, 0)
			Graphics::VertexData(glm::vec3(-1.f, -1.f, 0.f),        glm::vec2(0.f, 0.f),	glm::vec3(0.0f,  0.0f, -1.0)),  // (-1,-1, 0)
			Graphics::VertexData(glm::vec3(-1.f,  1.f, 0.f),        glm::vec2(0.f, 1.f),    glm::vec3(0.0f,  0.0f, -1.0)),  // (-1, 1, 0)
			};

			indices = { 0, 1, 2,   0, 2, 3 };

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

		std::vector<Graphics::Mesh> meshes;


		Model() {}

		Model(const std::vector<Graphics::Mesh>& meshes) : meshes(meshes) {}

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

		Texture(int width, int height, GLenum format, GLenum type) :
			width(width),
			height(height)
		{
			glGenTextures(1, &id);
			glBindTexture(GL_TEXTURE_2D, id);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
				width, height, 0, format, type, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		}
	};


	static void bindDepthTexture(uint textureId, uint frameBufferId)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, frameBufferId);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, textureId, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	static void blitToTexture(Texture texture, uint frameBufferId)
	{
		glViewport(0, 0, texture.width, texture.height);
		glBindFramebuffer(GL_FRAMEBUFFER, frameBufferId);
	}
}

#endif // !GRAPHICSH