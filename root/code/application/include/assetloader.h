#ifndef ASSETLOADER_H
#define ASSETLOADER_H

#include <string>
#include "graphics.h"

namespace AssetLoader 
{
	extern Graphics::ShaderSources loadShaderFiles(const std::string& baseName);
	extern Graphics::ShaderSources loadShaderFiles(const std::string& baseNameVert, const std::string& baseNameFrag);
	extern Graphics::Model loadModel(std::string const& nameWithExtension);
}

#endif