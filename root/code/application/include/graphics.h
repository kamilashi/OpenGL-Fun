#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <glm/glm.hpp>
#include <vector>
#include <string>
#include "types.h"

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

	struct VertexData 
	{
	public:
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 texCoords;
	};

	class Mesh {
	public:
		// mesh data
		std::vector<Graphics::VertexData>       vertices;
		std::vector<uint>		  indices;

		uint /* VAO,*/ VBO, EBO;

		Mesh(const std::vector<Graphics::VertexData>& verts, const std::vector<uint>& inds) : vertices(verts), indices(inds)
		{
			setupMesh();
		}
	private:
		//  render data

		void setupMesh();
	};

	class Model 
	{
	public:
		std::vector<Graphics::Mesh> meshes;

        Model(const std::vector<Graphics::Mesh>& meshes) : meshes(meshes) {}
	};
}

#endif // !GRAPHICSH