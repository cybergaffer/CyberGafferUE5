// Copyright (c) 2023 ALT LLC.
//
// Distributed under the terms of the MIT License (see the accompanying LICENSE.txt file or get a copy at http://opensource.org/licenses/MIT).


#include "CyberGafferSceneCapture.h"
#include "CyberGafferSceneCaptureComponent2D.h"
#include "MaterialDomain.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Interfaces/IPluginManager.h"
#include "Materials/MaterialExpressionConstant3Vector.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

ACyberGafferSceneCapture::ACyberGafferSceneCapture(const FObjectInitializer& objectInitializer) : Super(objectInitializer)
{
	_cyberGafferSceneCaptureComponent2D = CreateDefaultSubobject<UCyberGafferSceneCaptureComponent2D>(TEXT("NewCyberGafferSceneCaptureComponent2D"));
	_cyberGafferSceneCaptureComponent2D->SetupAttachment(RootComponent);

	FVector capturePosition = {-240,0,0};
	FVector spherePosition = {0,0,0};
	
	_cyberGafferSceneCaptureComponent2D->SetRelativeLocation(capturePosition);

	static ConstructorHelpers::FObjectFinder<UStaticMesh>SphereMeshAsset(TEXT("StaticMesh'/Engine/BasicShapes/Sphere.Sphere'"));

	FString pathMaterial = "Script/Engine.Material'/CyberGaffer/Materials/WhiteDiffuse.WhiteDiffuse'";
	BaseSphereMaterial = Cast<UMaterial>(StaticLoadObject(UMaterial::StaticClass(), nullptr, *pathMaterial));

	_taskSphere = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TaskSphere"));
	_taskSphere->SetupAttachment(RootComponent);
	_taskSphere->SetStaticMesh(SphereMeshAsset.Object);
	_taskSphere->SetVisibleInSceneCaptureOnly(true);
	_taskSphere->SetRelativeLocation(spherePosition);
	
	_gizmoSphere = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GizmoSphere"));
	_gizmoSphere->SetupAttachment(RootComponent);
	_gizmoSphere->SetStaticMesh(SphereMeshAsset.Object);
	_gizmoSphere->SetOnlyOwnerSee(true);
	_gizmoSphere->SetRelativeLocation(spherePosition);
}

void ACyberGafferSceneCapture::BeginPlay()
{
	
}


void ACyberGafferSceneCapture::OnInterpToggle(bool bEnable) {
	_cyberGafferSceneCaptureComponent2D->SetVisibility(bEnable);
	_taskSphere->SetVisibility(bEnable);
}

void ACyberGafferSceneCapture::PostActorCreated()
{
	//UMaterialInstanceDynamic* sphereMaterial = UMaterialInstanceDynamic::Create(BaseSphereMaterial, this, "CyberGafferSphereMaterial");
	//sphereMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor::White);
	//sphereMaterial->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor::Black);

	
	//BaseSphereMaterial->Parent = nullptr;
	
	_taskSphere->SetMaterial(0, BaseSphereMaterial);
	_gizmoSphere->SetMaterial(0, BaseSphereMaterial);
	
}

