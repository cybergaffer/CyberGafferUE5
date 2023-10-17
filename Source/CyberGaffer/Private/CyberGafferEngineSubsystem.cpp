// Copyright (c) 2023 Exostartup LLC.
//
// Distributed under the terms of the MIT License (see the accompanying LICENSE.txt file or get a copy at http://opensource.org/licenses/MIT).


#include "CyberGafferEngineSubsystem.h"

#include "CommonRenderResources.h"
#include "RHICommandList.h"
#include "Engine/TextureRenderTargetCube.h"
#include "Rendering/Texture2DResource.h"
#include "PixelFormat.h"

#include "CyberGafferDataPackage.h"
#include "CyberGafferShaders.h"
#include "CyberGafferLog.h"

// The side size of the cubemap, that will be sent to the server
static constexpr uint8 TargetMipSize = 64;

// Calculate the target mip level of texture which size will be equal to the TargetMipSize
static uint8 GetTargetMipLevelId(const uint32 width) {
	// check(width >= TargetMipSize);
	return static_cast<uint8>(log2(width / TargetMipSize));
}

// Create render targetable texture with RTV flag
static FTextureRHIRef CreateRHITexture(const uint32 width, uint32 height) {
	auto textureDesc = FRHITextureCreateDesc::Create2D(TEXT("HDRI2RealityDownsampleTex"), width, height, EPixelFormat::PF_FloatRGBA);
	textureDesc.SetClearValue(FClearValueBinding::Green);
	textureDesc.SetFlags(ETextureCreateFlags::RenderTargetable);
	textureDesc.SetInitialState(ERHIAccess::RTV);
	textureDesc.SetNumMips(1);
	textureDesc.DetermineInititialState();

	if (RHIGetInterfaceType() == ERHIInterfaceType::Vulkan) {
		textureDesc.AddFlags(ETextureCreateFlags::External);
	} else {
		textureDesc.AddFlags(ETextureCreateFlags::Shared);
	}

	return GDynamicRHI->RHICreateTexture(textureDesc);
}

void UCyberGafferEngineSubsystem::Initialize(FSubsystemCollectionBase& collection) {
	Super::Initialize(collection);

	CYBERGAFFERVERB_LOG(Log, TEXT("UCyberGafferEngineSubsystem::Initialize"));
}

void UCyberGafferEngineSubsystem::Deinitialize() {
	Super::Deinitialize();

	CYBERGAFFERVERB_LOG(Log, TEXT("UCyberGafferEngineSubsystem::Deinitialize"));
}

void UCyberGafferEngineSubsystem::OnUpdateSceneCaptureContentsEnqueued(const FString serverIp, const uint32_t port, UTextureRenderTargetCube* texture) {
	CYBERGAFFERVERB_LOG(Log, TEXT("UCyberGafferEngineSubsystem::OnUpdateSceneCaptureContentsEnqueued"));
	
	auto dataSender = _dataSender;

	ENQUEUE_RENDER_COMMAND(HDRI2REALITY_ProcessEnvironment)(
		[texture, dataSender, serverIp, port](FRHICommandListImmediate& rhiCmdList)
		{
			auto textureRHI = texture->GetResource() ? texture->GetResource()->TextureRHI : nullptr;
			if (textureRHI == nullptr) {
				CYBERGAFFER_LOG(Error, TEXT("UCyberGafferEngineSubsystem::OnUpdateSceneCaptureContentsEnqueued: invalid texture"));
				return;
			}

			// Render target size should be a 1/2 of the actual cube texture if possible to avoid additional mip level generation
			auto sideSize = textureRHI->GetDesc().Extent.X;

			check(FMath::IsPowerOfTwo(sideSize));
			
			if (sideSize >= TargetMipSize * 2) {
				sideSize = sideSize / 2;
			}

			// Create render target texture to store all 6 sides of cubemap
			auto destTexture = CreateRHITexture(sideSize, sideSize * 6);
			
			rhiCmdList.Transition(FRHITransitionInfo(textureRHI, ERHIAccess::Unknown, ERHIAccess::SRVGraphics));
			// rhiCmdList.Transition(FRHITransitionInfo(destTexture, ERHIAccess::Unknown, ERHIAccess::RTV));

			FRHIRenderPassInfo renderPassInfo(destTexture, ERenderTargetActions::Load_Store);
			// TODO: On Vulkan the positive and negative Y faces of the cubemap need to be flipped
			// TODO: check \Engine\Plugins\Runtime\Oculus\OculusVR\Source\OculusHMD\PrivateOculusHMD_CustomPresent.cpp 475

			rhiCmdList.BeginRenderPass(renderPassInfo, TEXT("CyberGaffer: render cubemap to 1x6 format"));
			{
				FGlobalShaderMap* shaderMap = GetGlobalShaderMap(GMaxRHIFeatureLevel);
				TShaderMapRef<FPackCubeMap_VS> vertexShader(shaderMap);
				TShaderMapRef<FPackCubeMap_PS> pixelShader(shaderMap);

				rhiCmdList.SetViewport(0, 0, 0.0f, destTexture->GetDesc().Extent.X, destTexture->GetDesc().Extent.Y, 1.0f);

				FGraphicsPipelineStateInitializer graphicsPSOInit;
				rhiCmdList.ApplyCachedRenderTargets(graphicsPSOInit);
				graphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
				graphicsPSOInit.RasterizerState = TStaticRasterizerState<>::GetRHI();
				graphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();
				graphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GFilterVertexDeclaration.VertexDeclarationRHI;
				graphicsPSOInit.BoundShaderState.VertexShaderRHI = vertexShader.GetVertexShader();
				graphicsPSOInit.BoundShaderState.PixelShaderRHI = pixelShader.GetPixelShader();
				graphicsPSOInit.PrimitiveType = PT_TriangleList;
				SetGraphicsPipelineState(rhiCmdList, graphicsPSOInit, 0);

				FRHISamplerState* samplerState = TStaticSamplerState<SF_Bilinear>::GetRHI();
				
				pixelShader->SetParameters(rhiCmdList, textureRHI, samplerState);

				FIntPoint targetBufferSize(destTexture->GetDesc().Extent.X, destTexture->GetDesc().Extent.Y);
				
				IRendererModule* rendererModule = &FModuleManager::GetModuleChecked<IRendererModule>("Renderer");
				rendererModule->DrawRectangle(
					rhiCmdList,
					0,
					0,
					destTexture->GetDesc().Extent.X,
					destTexture->GetDesc().Extent.Y,
					0,
					0,
					1,
					1,
					targetBufferSize,
					FIntPoint(1,1),
					vertexShader,
					EDRF_Default
				);
			}

			rhiCmdList.EndRenderPass();
			
			TArray<FTextureRHIRef> mips;
			
			uint8 targetMip = 0;
			if (sideSize > TargetMipSize) {
				targetMip = GetTargetMipLevelId(sideSize);
			}
			
			mips.Empty(targetMip + 1);
			mips.Add(destTexture);
			
			// Downsample texture, if we have bigger texture width than we need
			if (sideSize > TargetMipSize) {
				for (uint8 i = 1; i <= targetMip; ++i) {
					const auto prevTexture = mips[i - 1];
					mips.Add(CreateRHITexture(prevTexture->GetDesc().Extent.X / 2, prevTexture->GetDesc().Extent.Y / 2));
					const auto currentTexture = mips[i];

					rhiCmdList.Transition(FRHITransitionInfo(prevTexture, ERHIAccess::Unknown, ERHIAccess::SRVGraphics));
					// rhiCmdList.Transition(FRHITransitionInfo(currentTexture, ERHIAccess::Unknown, ERHIAccess::RTV));

					FRHIRenderPassInfo mipRenderPassInfo(currentTexture, ERenderTargetActions::Load_Store);
					FString renderPassName = FString::Format(TEXT("CyberGaffer: render mip level {0}"), {i});
					rhiCmdList.BeginRenderPass(mipRenderPassInfo, *renderPassName);
					{
						FGlobalShaderMap* shaderMap = GetGlobalShaderMap(GMaxRHIFeatureLevel);
						TShaderMapRef<FPackCubeMap_VS> vertexShader(shaderMap);
						TShaderMapRef<FMipGeneration_PS> pixelShader(shaderMap);
						
						rhiCmdList.SetViewport(0, 0, 0.0f, currentTexture->GetDesc().Extent.X, currentTexture->GetDesc().Extent.Y, 1.0f);

						FGraphicsPipelineStateInitializer graphicsPSOInit;
						rhiCmdList.ApplyCachedRenderTargets(graphicsPSOInit);
						graphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
						graphicsPSOInit.RasterizerState = TStaticRasterizerState<>::GetRHI();
						graphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();
						graphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GFilterVertexDeclaration.VertexDeclarationRHI;
						graphicsPSOInit.BoundShaderState.VertexShaderRHI = vertexShader.GetVertexShader();
						graphicsPSOInit.BoundShaderState.PixelShaderRHI = pixelShader.GetPixelShader();
						graphicsPSOInit.PrimitiveType = PT_TriangleList;
						SetGraphicsPipelineState(rhiCmdList, graphicsPSOInit, 0);

						FRHISamplerState* samplerState = TStaticSamplerState<SF_Bilinear>::GetRHI();
						
						pixelShader->SetParameters(rhiCmdList, prevTexture, samplerState);

						FIntPoint targetBufferSize(currentTexture->GetDesc().Extent.X, currentTexture->GetDesc().Extent.Y);
						
						IRendererModule* rendererModule = &FModuleManager::GetModuleChecked<IRendererModule>("Renderer");
						rendererModule->DrawRectangle(
							rhiCmdList,
							0,
							0,
							currentTexture->GetDesc().Extent.X,
							currentTexture->GetDesc().Extent.Y,
							0,
							0,
							1,
							1,
							targetBufferSize,
							FIntPoint(1,1),
							vertexShader,
							EDRF_Default
						);
					}

					rhiCmdList.EndRenderPass();
				}
			}

			// Read target mip level and put it's data to the sender
			FReadSurfaceDataFlags flags = {};
			flags.SetMip(0);
			flags.SetLinearToGamma(false);

			TArray<FFloat16Color> data;
			FIntRect rect(0, 0, TargetMipSize, TargetMipSize * 6);
			rhiCmdList.ReadSurfaceFloatData(mips[targetMip], rect, data, flags);

			dataSender->SetPackageToSend(FCyberGafferDataPackage(serverIp, port, data));
		}
	);
}

