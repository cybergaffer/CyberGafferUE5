// Copyright (c) 2023 ALT LLC.
//
// Distributed under the terms of the MIT License (see the accompanying LICENSE.txt file or get a copy at http://opensource.org/licenses/MIT).


#include "CyberGafferSceneCaptureCube.h"
#include "CyberGafferSceneCaptureComponentCube.h"

#include "UObject/ConstructorHelpers.h"


ACyberGafferSceneCaptureCube::ACyberGafferSceneCaptureCube(const FObjectInitializer& objectInitializer) : Super(objectInitializer) {
	_cyberGafferSceneCaptureComponentCube = CreateDefaultSubobject<UCyberGafferSceneCaptureComponentCube>(TEXT("NewCyberGafferSceneCaptureComponentCube"));
	_cyberGafferSceneCaptureComponentCube->SetupAttachment(RootComponent);
}

void ACyberGafferSceneCaptureCube::OnInterpToggle(bool bEnable) {
	_cyberGafferSceneCaptureComponentCube->SetVisibility(bEnable);
}
