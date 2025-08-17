#ifndef ASSETLOADER_H
#define ASSETLOADER_H

#include <string>
#include "graphics.h"

namespace AssetLoader 
{
	extern Graphics::ShaderSources loadShaderFile(const std::string& path);
	extern Graphics::ShaderSources loadShaderFiles(const std::string& baseName, const std::string& dir);
	extern Graphics::Model loadModel( const std::string& path);
}

#endif