// Copyright (c) 2023 Exostartup LLC.
//
// Distributed under the terms of the MIT License (see the accompanying LICENSE.txt file or get a copy at http://opensource.org/licenses/MIT).


#include "CyberGafferSceneCaptureComponentCube.h"

#include "Engine/TextureRenderTargetCube.h"
#include "Engine/Engine.h"
#include "Engine/Texture.h"
#include "Engine/TextureDefines.h"
#include "Math/UnrealMathUtility.h"
#include "PixelFormat.h"

#include "CyberGafferLog.h"

// Sets default values for this component's properties
UCyberGafferSceneCaptureComponentCube::UCyberGafferSceneCaptureComponentCube() {
	PrimaryComponentTick.bCanEverTick = true;

	// TODO: find a way to mark this properties uneditable. Using the ClearPropertyFlags(CPF_Edit) will change the parent class as well. 
	bCaptureRotation = true;
	bCaptureEveryFrame = true;
	CaptureSource = SCS_FinalColorHDR;
}

void UCyberGafferSceneCaptureComponentCube::BeginPlay() {
	Super::BeginPlay();

	CheckTextureTarget();
	
	InitializeSubsystem();
}

void UCyberGafferSceneCaptureComponentCube::UpdateSceneCaptureContents(FSceneInterface* scene) {
	if (TextureTarget == nullptr) {
		return;
	}
	
	if (_subsystem == nullptr) {
		if (!InitializeSubsystem()) {
			CYBERGAFFER_LOG(Warning, TEXT("UCyberGafferSceneCaptureComponentCube::UpdateSceneCaptureContents: Subsystem is null"));
			return;
		}
	}
	
	Super::UpdateSceneCaptureContents(scene);

	if (!FMath::IsPowerOfTwo(TextureTarget->SizeX)) {
		CYBERGAFFER_LOG(Error, TEXT("UCyberGafferSceneCaptureComponentCube::UpdateSceneCaptureContents: the target cube texture side size must be power of 2. Recommended size: 512 or 1024"));
		return;
	}

	if (!ProcessCubeMap) {
		return;
	}
	
	_subsystem->OnUpdateSceneCaptureContentsEnqueued(ServerIpAddress, ServerPort, TextureTarget);
}

bool UCyberGafferSceneCaptureComponentCube::InitializeSubsystem() {
	if (GEngine == nullptr) {
		CYBERGAFFER_LOG(Warning, TEXT("UCyberGafferSceneCaptureComponentCube::InitializeSubsystem: GEngine is null"));
		return false;
	}

	_subsystem = GEngine->GetEngineSubsystem<UCyberGafferEngineSubsystem>();
	return _subsystem != nullptr;
}

#if WITH_EDITOR
void UCyberGafferSceneCaptureComponentCube::PostEditChangeProperty(FPropertyChangedEvent& propertyChangedEvent) {
	Super::PostEditChangeProperty(propertyChangedEvent);
	
	const FName memberPropertyName = (propertyChangedEvent.MemberProperty != NULL) ? propertyChangedEvent.MemberProperty->GetFName() : NAME_None;
	
	if (memberPropertyName.IsEqual("TextureTarget")) {
		CheckTextureTarget();
		return;
	}

	if (memberPropertyName.IsEqual("CaptureSource")) {
		if (CaptureSource.GetValue() != SCS_FinalColorHDR) {
			CYBERGAFFER_LOG(Warning, TEXT("UCyberGafferSceneCaptureComponentCube::PostEditChangeProperty: Capture Source must be Final Color (HDR) in Linear Working Color Space, fixing it"));
			CaptureSource = SCS_FinalColorHDR;
		}
		return;
	}

	if (memberPropertyName.IsEqual("bCaptureRotation")) {
		if (!bCaptureRotation) {
			CYBERGAFFER_LOG(Warning, TEXT("UCyberGafferSceneCaptureComponentCube::PostEditChangeProperty: Capture Rotation must be on, fixing it"));
			bCaptureRotation = true;
		}
	}
}
#endif

void UCyberGafferSceneCaptureComponentCube::CheckTextureTarget() {
	CYBERGAFFERVERB_LOG(Log, TEXT("UCyberGafferSceneCaptureComponentCube::CheckTextureTarget"));
	
	if (TextureTarget == nullptr) {
		return;
	}

	bool textureUpdated = false;

	if (!TextureTarget->bHDR) {
		CYBERGAFFER_LOG(Warning, TEXT("UCyberGafferSceneCaptureComponentCube::CheckTextureTarget: TextureRenderTargetCube must have bHDR property on, fixing it"));
		TextureTarget->bHDR = true;
		textureUpdated = true;
	}

	auto sizeX = TextureTarget->SizeX;
	if (!FMath::IsPowerOfTwo(sizeX)) {
		CYBERGAFFER_LOG(Warning, TEXT("UCyberGafferSceneCaptureComponentCube::CheckTextureTarget: TextureRenderTargetCube size must be power of 2, fixing it"));
		sizeX = FMath::RoundUpToPowerOfTwo(sizeX);
		textureUpdated = true;
		TextureTarget->SizeX = sizeX;
	}

	auto format = TextureTarget->GetFormat();
	if (format != PF_FloatRGBA) {
		CYBERGAFFER_LOG(Warning, TEXT("UCyberGafferSceneCaptureComponentCube::CheckTextureTarget: TextureRenderTargetCube format must be RGBA16F, fixing it"));
		format = PF_FloatRGBA;
		textureUpdated = true;
	}

	const auto compressionSettings = TextureTarget->CompressionSettings;
	if (compressionSettings.GetValue() != TC_HDR) {
		CYBERGAFFER_LOG(Warning, TEXT("UCyberGafferSceneCaptureComponentCube::CheckTextureTarget: TextureRenderTargetCube compression settings must be HDR (RGBA16F), fixing it"));
		TextureTarget->CompressionSettings = TC_HDR;
		textureUpdated = true;
	}

	if (textureUpdated) {
		TextureTarget->ReleaseResource();
		TextureTarget->Init(sizeX, format);
		TextureTarget->UpdateResourceImmediate(true);
	}
}

