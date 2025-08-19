#ifndef ASSETLOADER_H
#define ASSETLOADER_H

#include <string>
#include "graphics.h"

namespace AssetLoader 
{
	extern Graphics::ShaderSources loadShaderFiles(
		const std::string& baseName,
		const std::vector<std::string>& definesV, 
		const std::vector<std::string>& definesF);

	extern Graphics::ShaderSources loadShaderFiles(
		const std::string& baseNameVert, 
		const std::string& baseNameFrag, 
		const std::vector<std::string>& definesV, 
		const std::vector<std::string>& definesF);

	extern Graphics::Model loadModel(std::string const& nameWithExtension);
}

#endif