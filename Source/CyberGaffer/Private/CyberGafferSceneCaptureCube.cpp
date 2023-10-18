// Fill out your copyright notice in the Description page of Project Settings.

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
