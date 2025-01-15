#include "CyberGafferSceneCapture.h"
#include "CyberGafferSceneCaptureComponent2D.h"
#include "CyberGafferSphereComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInterface.h"
#include "Engine/StaticMesh.h"

namespace {
	static const FVector SceneCaptureComponentScale (1.0, 1.0, 1.0);
	static const FVector CapturePosition (-300.0, 0.0, 0.0);
	
	static const FVector SpherePosition (0.0, 0.0, 0.0);
	static const FVector SphereScale (1.0f,1.0f,1.0f);
	static const FRotator SphereRotation (0.0f, 0.0f, 0.0f);
	static const FTransform SphereTransform (SphereRotation, SpherePosition, SphereScale);
}

ACyberGafferSceneCapture::ACyberGafferSceneCapture(const FObjectInitializer& objectInitializer) : Super(objectInitializer) {
	_cyberGafferSceneCaptureComponent2D = CreateDefaultSubobject<UCyberGafferSceneCaptureComponent2D>(TEXT("NewCyberGafferSceneCaptureComponent2D"));
	_cyberGafferSceneCaptureComponent2D->SetupAttachment(RootComponent);
	
	UStaticMesh* sphereMesh = Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), nullptr, TEXT("/Script/Engine.StaticMesh'/CyberGaffer/CyberGafferProbe.CyberGafferProbe'")));

	FString pathMaterial = "Script/Engine.Material'/CyberGaffer/Materials/CyberGafferWhiteDiffuse.CyberGafferWhiteDiffuse'";
	BaseSphereMaterial = Cast<UMaterial>(StaticLoadObject(UMaterial::StaticClass(), nullptr, *pathMaterial));

	_taskSphere = CreateDefaultSubobject<UCyberGafferSphereComponent>(TEXT("TaskSphere"));
	_taskSphere->SetupAttachment(RootComponent);
	_taskSphere->SetStaticMesh(sphereMesh);
	_taskSphere->SetVisibleInSceneCaptureOnly(true);
	_taskSphere->CastShadow = false;
	
	_gizmoSphere = CreateDefaultSubobject<UCyberGafferSphereComponent>(TEXT("GizmoSphere"));
	_gizmoSphere->SetupAttachment(RootComponent);
	_gizmoSphere->SetStaticMesh(sphereMesh);
	_gizmoSphere->SetOnlyOwnerSee(true);
	_gizmoSphere->CastShadow = false;
	_gizmoSphere->bHiddenInGame = true;
	
	_taskSphere->SetMaterial(0, BaseSphereMaterial);
	_gizmoSphere->SetMaterial(0, BaseSphereMaterial);
	UpdateChildTransforms();
}

void ACyberGafferSceneCapture::PostLoad() {
	Super::PostLoad();
}

void ACyberGafferSceneCapture::OnInterpToggle(bool bEnable) {
	_cyberGafferSceneCaptureComponent2D->SetVisibility(bEnable);
	_taskSphere->SetVisibility(bEnable);
	_gizmoSphere->SetVisibility(bEnable);
}

#if WITH_EDITOR
void ACyberGafferSceneCapture::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) {
	Super::PostEditChangeProperty(PropertyChangedEvent);
	UpdateChildTransforms();
}
#endif


void ACyberGafferSceneCapture::UpdateChildTransforms() {
	_cyberGafferSceneCaptureComponent2D->SetRelativeLocation(CapturePosition);
	_cyberGafferSceneCaptureComponent2D->SetRelativeScale3D(SceneCaptureComponentScale);

	_taskSphere->SetRelativeTransform(SphereTransform);
	_gizmoSphere->SetRelativeTransform(SphereTransform);
	
	// _taskSphere->SetRelativeLocation(SpherePosition);
	// _taskSphere->SetRelativeScale3D(SphereScale);
	// _gizmoSphere->SetRelativeLocation(SpherePosition);
	// _gizmoSphere->SetRelativeScale3D(SphereScale);
}






