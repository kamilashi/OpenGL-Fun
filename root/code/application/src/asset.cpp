#include <stdexcept> 
#include <iostream>

#include "asset.h"
#include "types.h"
#include "assetloader.h"

void ShaderUniforms::initBasicUniforms(uint shaderId)
{
	transformLoc = glGetUniformLocation(shaderId, "uTransform");
	viewLoc = glGetUniformLocation(shaderId, "uView");
	projLoc = glGetUniformLocation(shaderId, "uProjection");

	mainColorLoc = glGetUniformLocation(shaderId, "uMainColor");

	mainLightColorLoc = glGetUniformLocation(shaderId, "uMainLightColor");
	mainLightDirLoc = glGetUniformLocation(shaderId, "uMainLightDirection");

	timeLoc = glGetUniformLocation(shaderId, "uTime");
}

uint compileStage(GLenum type, const std::string& src, const char* label) {
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

uint linkProgram(uint vs, uint fs)
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

Shader::Shader(const char* name, bool combine,
	const std::vector<std::string>& definesV,
	const std::vector<std::string>& definesF)
{
	ShaderSources src = AssetLoader::loadShaderFiles(name, definesV, definesF);
	uint vs = compileStage(GL_VERTEX_SHADER, src.vertex, "vertex");
	uint fs = compileStage(GL_FRAGMENT_SHADER, src.fragment, "fragment");
	id = linkProgram(vs, fs);

	uniforms = ShaderUniforms(id);
}

Shader::Shader(const char* nameV,
	const char* nameF,
	const std::vector<std::string>& definesV,
	const std::vector<std::string>& definesF)
{
	ShaderSources src = AssetLoader::loadShaderFiles(nameV, nameF, definesV, definesF);
	uint vs = compileStage(GL_VERTEX_SHADER, src.vertex, "vertex");
	uint fs = compileStage(GL_FRAGMENT_SHADER, src.fragment, "fragment");
	id = linkProgram(vs, fs);

	uniforms = ShaderUniforms(id);
}

uint Shader::getLoc(const GLchar* uName)
{
	return glGetUniformLocation(id, uName);
}

void Shader::setTransformUniforms(const Camera& camera, const glm::mat4& transform)
{
	glUniformMatrix4fv(uniforms.transformLoc, 1, GL_FALSE, glm::value_ptr(transform));
	glUniformMatrix4fv(uniforms.viewLoc, 1, GL_FALSE, glm::value_ptr(camera.viewMatrix));
	glUniformMatrix4fv(uniforms.projLoc, 1, GL_FALSE, glm::value_ptr(camera.projMatrix));
}

void Shader::setMainLightUniforms(const glm::vec3& lightColor, const glm::vec3& lightDir)
{
	glUniform3f(uniforms.mainLightColorLoc, lightColor.x, lightColor.y, lightColor.z);
	glUniform3f(uniforms.mainLightDirLoc, lightDir.x, lightDir.y, lightDir.z);
}

void Shader::setMainColorUniform(const glm::vec3& color)
{
	glUniform3f(uniforms.mainColorLoc, color.x, color.y, color.z);
}

void Shader::setCustomUniformF3(uint uniformLoc, const glm::vec3& color)
{
	glUniform3f(uniformLoc, color.x, color.y, color.z);
}

void Shader::setCustomUniformM4(uint uniformLoc, const glm::mat4& matrix)
{
	glUniformMatrix4fv(uniformLoc, 1, GL_FALSE, glm::value_ptr(matrix));
}

void Shader::setCustomUniformF(uint uniformLoc, float value)
{
	glUniform1f(uniformLoc, value);
}

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

Texture::Texture(int width, int height, GLenum format, GLenum type) :
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