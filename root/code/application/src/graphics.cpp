
#include <glad.h>
#include <stdexcept> 
#include <iostream>

#include "graphics.h"
#include "assetloader.h"
#include "camera.h"

namespace Graphics
{

	void Mesh::setupMesh()
	{
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(VertexData), &vertices[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint), &indices[0], GL_STATIC_DRAW);

		// positions
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), nullptr);

		// uvs
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, texCoords));

		// normals
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, normal));

		// tangents
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, tangent));

		// bitangents
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, bitangent));
	}

	void Mesh::draw()
	{
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, nullptr);
		glBindVertexArray(0);
	}

	void Model::draw()
	{
		for (auto it = meshes.begin(); it < meshes.end(); it++)
		{
			Mesh& mesh = *it;
			mesh.draw();
		}
	}


	static uint compileStage(GLenum type, const std::string& src, const char* label) {
		uint id = glCreateShader(type);
		const char* csrc = src.c_str();
		glShaderSource(id, 1, &csrc, nullptr);
		glCompileShader(id);

		int ok = 0;
		glGetShaderiv(id, GL_COMPILE_STATUS, &ok);
		if (!ok)
		{
			int len = 0; glGetShaderiv(id, GL_INFO_LOG_LENGTH, &len);
			std::string log(len, '\0');
			glGetShaderInfoLog(id, len, &len, log.data());
			std::cerr << "[Shader compile error] " << label << ":\n" << log << "\n";
			glDeleteShader(id);
			throw std::runtime_error("Shader compilation failed");
		}
		return id;
	}

	static uint linkProgram(uint vs, uint fs)
	{
		uint prog = glCreateProgram();
		glAttachShader(prog, vs);
		glAttachShader(prog, fs);
		glLinkProgram(prog);

		int ok = 0;
		glGetProgramiv(prog, GL_LINK_STATUS, &ok);
		if (!ok)
		{
			int len = 0; glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &len);
			std::string log(len, '\0');
			glGetProgramInfoLog(prog, len, &len, log.data());

			printf("Program link error \n %s\n", log.c_str());

			glDeleteProgram(prog);
			throw std::runtime_error("Program link failed");
		}
		glDetachShader(prog, vs);
		glDetachShader(prog, fs);
		glDeleteShader(vs);
		glDeleteShader(fs);

		return prog;
	}

	Shader::Shader(const char* name)
	{
		Graphics::ShaderSources src = AssetLoader::loadShaderFiles(name);
		uint vs = compileStage(GL_VERTEX_SHADER, src.vertex, "vertex");
		uint fs = compileStage(GL_FRAGMENT_SHADER, src.fragment, "fragment");
		id = linkProgram(vs, fs);

		uniforms = ShaderUniforms(id);
	}

	Shader::Shader(const char* nameV, const char* nameF)
	{
		Graphics::ShaderSources src = AssetLoader::loadShaderFiles(nameV, nameF);
		uint vs = compileStage(GL_VERTEX_SHADER, src.vertex, "vertex");
		uint fs = compileStage(GL_FRAGMENT_SHADER, src.fragment, "fragment");
		id = linkProgram(vs, fs);

		uniforms = ShaderUniforms(id);
	}

}