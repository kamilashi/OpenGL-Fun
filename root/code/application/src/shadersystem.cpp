#include <fstream>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <iostream>
#include "shadersystem.h"

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

extern ShaderSources ShaderSystem::loadShaderFile(const std::string& path)
{
	std::ifstream in(path);

	if (!in)
	{
		throw std::runtime_error("Failed to open shader file: " + path);
	}

	std::stringstream ss[2];

	ShaderParseMode mode = ShaderParseMode_NONE;

	std::string line;
	while (std::getline(in, line)) 
	{
		if (line.rfind("#shader", 0) == 0) 
		{
			if (line.find("vertex") != std::string::npos)
			{
				mode = ShaderParseMode_VERT;
			}
			else if (line.find("fragment") != std::string::npos) 
			{
				mode = ShaderParseMode_FRAG;
			}
			else
			{
				mode = ShaderParseMode_NONE;
			}
		}
		else if (mode != ShaderParseMode_NONE)
		{
			ss[mode] << line << '\n';
		}
	}

	ShaderSources shaderSources(ss[ShaderParseMode_VERT].str(), ss[ShaderParseMode_FRAG].str());

	return shaderSources;
}

extern ShaderSources ShaderSystem::loadShaderFiles(const std::string& baseName, const std::string& dir)
{
	std::string vPath = dir + "/" + baseName + "V.shader";
	std::string fPath = dir + "/" + baseName + "F.shader";

	ShaderSources shaderSources(readFile(vPath), readFile(fPath));

	return shaderSources;
}
