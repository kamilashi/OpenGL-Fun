
#include <string>

struct ShaderSources {
	std::string vertex;
	std::string fragment;

	ShaderSources(std::string vertStr, std::string fraqStr)
	{
		vertex = vertStr;
		fragment = fraqStr;
	}
};

namespace ShaderSystem 
{
	extern ShaderSources loadShaderFile(const std::string& path);
	extern ShaderSources loadShaderFiles(const std::string& baseName, const std::string& dir);
}
