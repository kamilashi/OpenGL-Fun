#ifndef ASSETLOADER_H
#define ASSETLOADER_H

#include <string>
#include "asset.h"

namespace AssetLoader 
{
	extern ShaderSources loadShaderFiles(
		const std::string& baseName,
		const std::vector<std::string>& definesV, 
		const std::vector<std::string>& definesF);

	extern ShaderSources loadShaderFiles(
		const std::string& baseNameVert, 
		const std::string& baseNameFrag, 
		const std::vector<std::string>& definesV, 
		const std::vector<std::string>& definesF);

	extern Model loadModel(std::string const& nameWithExtension);
}

#endif