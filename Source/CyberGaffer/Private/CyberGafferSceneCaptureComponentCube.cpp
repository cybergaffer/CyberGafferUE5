// Copyright (c) 2023 Exostartup LLC.
//
// Distributed under the terms of the MIT License (see the accompanying LICENSE.txt file or get a copy at http://opensource.org/licenses/MIT).


#include "CyberGafferSceneCaptureComponentCube.h"

#include "Engine/TextureRenderTargetCube.h"

#include "CyberGafferLog.h"

// Sets default values for this component's properties
UCyberGafferSceneCaptureComponentCube::UCyberGafferSceneCaptureComponentCube() {
	PrimaryComponentTick.bCanEverTick = true;
}

void UCyberGafferSceneCaptureComponentCube::BeginPlay() {
	Super::BeginPlay();	
	InitializeSubsystem();
}

void UCyberGafferSceneCaptureComponentCube::TickComponent(float deltaTime, ELevelTick tickType, FActorComponentTickFunction* thisTickFunction) {
	Super::TickComponent(deltaTime, tickType, thisTickFunction);
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

