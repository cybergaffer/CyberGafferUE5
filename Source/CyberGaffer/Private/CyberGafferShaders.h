// Copyright (c) 2023 Exostartup LLC.
//
// Distributed under the terms of the MIT License (see the accompanying LICENSE.txt file or get a copy at http://opensource.org/licenses/MIT).


#pragma once

#include "CoreMinimal.h"

#include "GlobalShader.h"
#include "Shader.h"
#include "ShaderParameterMacros.h"
#include "ShaderParameterStruct.h"
#include "ShaderParameterUtils.h"

#include "SceneView.h"

#include "Serialization/MemoryLayout.h"

class FPackCubeMap_VS : public FGlobalShader {
	DECLARE_SHADER_TYPE(FPackCubeMap_VS, Global);

public:
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& parameters) { return true; }

	FPackCubeMap_VS() = default;
	FPackCubeMap_VS(const ShaderMetaType::CompiledShaderInitializerType& initializer) : FGlobalShader(initializer) {}

	void SetParameters(FRHICommandList& rhiCmdList, FRHIUniformBuffer* viewUniformBuffer){
		FGlobalShader::SetParameters<FViewUniformShaderParameters>(rhiCmdList, rhiCmdList.GetBoundVertexShader(), viewUniformBuffer);
	}
};

class FPackCubeMap_PS : public FGlobalShader {
	DECLARE_SHADER_TYPE(FPackCubeMap_PS, Global);

public:
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& parameters) { return true; }

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& parameters, FShaderCompilerEnvironment& outEnvironment) {
		FGlobalShader::ModifyCompilationEnvironment(parameters, outEnvironment);
	}

	FPackCubeMap_PS() = default;
	FPackCubeMap_PS(const ShaderMetaType::CompiledShaderInitializerType& initializer) : FGlobalShader(initializer) {
		SourceCubemap.Bind(initializer.ParameterMap, TEXT("SourceCubemap"));
		SourceCubemapSampler.Bind(initializer.ParameterMap, TEXT("SourceCubemapSampler"));
	}

	/*void SetParameters(FRHICommandList& rhiCmdList, const FTexture* texture) {
		SetTextureParameter(rhiCmdList, rhiCmdList.GetBoundPixelShader(), SourceCubemap, SourceCubemapSampler, texture);
	}*/

	void SetParameters(FRHICommandList& rhiCmdList, FRHITexture* textureRHI, FRHISamplerState* samplerStateRHI) {
		SetTextureParameter(rhiCmdList, rhiCmdList.GetBoundPixelShader(), SourceCubemap, SourceCubemapSampler, samplerStateRHI, textureRHI);
	}

private:
	LAYOUT_FIELD(FShaderResourceParameter, SourceCubemap);
	LAYOUT_FIELD(FShaderResourceParameter, SourceCubemapSampler);
};

class FMipGeneration_PS : public FGlobalShader {
	DECLARE_SHADER_TYPE(FMipGeneration_PS, Global);

public:
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& parameters) { return true; }

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& parameters, FShaderCompilerEnvironment& outEnvironment) {
		FGlobalShader::ModifyCompilationEnvironment(parameters, outEnvironment);
	}

	FMipGeneration_PS() = default;
	FMipGeneration_PS(const ShaderMetaType::CompiledShaderInitializerType& initializer) : FGlobalShader(initializer) {
		SourceTexture.Bind(initializer.ParameterMap, TEXT("SourceTexture"));
		SourceTextureSampler.Bind(initializer.ParameterMap, TEXT("SourceTextureSampler"));
	}

	void SetParameters(FRHICommandList& rhiCmdList, FRHITexture* textureRHI, FRHISamplerState* samplerStateRHI) {
		SetTextureParameter(rhiCmdList, rhiCmdList.GetBoundPixelShader(), SourceTexture, SourceTextureSampler, samplerStateRHI, textureRHI);
	}

private:
	LAYOUT_FIELD(FShaderResourceParameter, SourceTexture);
	LAYOUT_FIELD(FShaderResourceParameter, SourceTextureSampler);
};