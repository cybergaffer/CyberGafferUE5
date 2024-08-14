// Copyright (c) 2023 ALT LLC.
//
// Distributed under the terms of the MIT License (see the accompanying LICENSE.txt file or get a copy at http://opensource.org/licenses/MIT).


#include "CyberGafferSceneCapture.h"
#include "CyberGafferSceneCaptureComponent2D.h"
#include "CyberGafferSphereComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInterface.h"
#include "Engine/StaticMesh.h"

ACyberGafferSceneCapture::ACyberGafferSceneCapture(const FObjectInitializer& objectInitializer) : Super(objectInitializer)
{
	_cyberGafferSceneCaptureComponent2D = CreateDefaultSubobject<UCyberGafferSceneCaptureComponent2D>(TEXT("NewCyberGafferSceneCaptureComponent2D"));
	_cyberGafferSceneCaptureComponent2D->SetupAttachment(RootComponent);
	
	// static ConstructorHelpers::FObjectFinder<UStaticMesh>SphereMeshAsset(TEXT("/Script/Engine.StaticMesh'/CyberGaffer/sphere6.sphere6'"));
	UStaticMesh* sphereMesh = Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), nullptr, TEXT("/Script/Engine.StaticMesh'/CyberGaffer/sphere6.sphere6'")));

	FString pathMaterial = "Script/Engine.Material'/CyberGaffer/Materials/CyberGafferWhiteDiffuse.CyberGafferWhiteDiffuse'";
	BaseSphereMaterial = Cast<UMaterial>(StaticLoadObject(UMaterial::StaticClass(), nullptr, *pathMaterial));

	_taskSphere = CreateDefaultSubobject<UCyberGafferSphereComponent>(TEXT("TaskSphere"));
	_taskSphere->SetupAttachment(RootComponent);
	// _taskSphere->SetStaticMesh(SphereMeshAsset.Object);
	_taskSphere->SetStaticMesh(sphereMesh);
	_taskSphere->SetVisibleInSceneCaptureOnly(true);
	_taskSphere->CastShadow = false;
	
	_gizmoSphere = CreateDefaultSubobject<UCyberGafferSphereComponent>(TEXT("GizmoSphere"));
	_gizmoSphere->SetupAttachment(RootComponent);
	// _gizmoSphere->SetStaticMesh(SphereMeshAsset.Object);
	_gizmoSphere->SetStaticMesh(sphereMesh);
	_gizmoSphere->SetOnlyOwnerSee(true);
	_gizmoSphere->CastShadow = false;
	_gizmoSphere->bHiddenInGame = true;
	
	_taskSphere->SetMaterial(0, BaseSphereMaterial);
	_gizmoSphere->SetMaterial(0, BaseSphereMaterial);
	UpdateChildTransforms();
}

void ACyberGafferSceneCapture::PostLoad()
{
	Super::PostLoad();
}

void ACyberGafferSceneCapture::OnInterpToggle(bool bEnable) {
	_cyberGafferSceneCaptureComponent2D->SetVisibility(bEnable);
	_taskSphere->SetVisibility(bEnable);
	_gizmoSphere->SetVisibility(bEnable);
}

#if WITH_EDITOR
void ACyberGafferSceneCapture::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	UpdateChildTransforms();
}
#endif


void ACyberGafferSceneCapture::UpdateChildTransforms()
{
	FVector sceneCaptureComponentScale {1,1,1};
	FVector capturePosition = {-300,0,0};
	FVector spherePosition = {0,0,0};
	FVector sphereScale {0.1f,0.1f,0.1f};

	_cyberGafferSceneCaptureComponent2D->SetRelativeLocation(capturePosition);
	_cyberGafferSceneCaptureComponent2D->SetRelativeScale3D(sceneCaptureComponentScale);
	_taskSphere->SetRelativeLocation(spherePosition);
	_taskSphere->SetRelativeScale3D(sphereScale);
	_gizmoSphere->SetRelativeLocation(spherePosition);
	_gizmoSphere->SetRelativeScale3D(sphereScale);
}






