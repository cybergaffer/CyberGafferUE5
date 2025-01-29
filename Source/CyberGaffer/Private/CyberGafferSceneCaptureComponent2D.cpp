#include "CyberGafferSceneCaptureComponent2D.h"
#include "Engine/Engine.h"
#include "Engine/Texture.h"
#include "Engine/TextureDefines.h"
#include "Math/UnrealMathUtility.h"
#include "PixelFormat.h"
#include "CyberGafferLog.h"
#include "CyberGafferSceneCapture.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Math/Color.h"
#include "PrimitiveSceneProxy.h"
#include "SceneManagement.h"
#include "Kismet/GameplayStatics.h"

UCyberGafferSceneCaptureComponent2D::UCyberGafferSceneCaptureComponent2D() {
	PrimaryComponentTick.bCanEverTick = true;

	// TODO: find a way to mark this properties uneditable. Using the ClearPropertyFlags(CPF_Edit) will change the parent class as well. 
	bCaptureEveryFrame = true;
	CaptureSource = SCS_FinalColorHDR;
	
	CheckCaptureSettings();
}

void UCyberGafferSceneCaptureComponent2D::BeginPlay() {
	Super::BeginPlay();

	CheckTextureTarget();
	CheckCaptureSettings();
	InitializeSubsystem();
	UpdateFOV();
}

APostProcessVolume* UCyberGafferSceneCaptureComponent2D::FindPostProcessVolume() {
	APostProcessVolume* result = nullptr;
	
	TArray<AActor*> postProcessVolumes;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APostProcessVolume::StaticClass(), postProcessVolumes);
	if (postProcessVolumes.Num() == 0) {
		// CYBERGAFFER_LOG(Log, TEXT("PP Volume not found"));
		GlobalPostProcessVolume = nullptr;
		return nullptr;
	}

	// Assume that the first unbound post process volume is what we need.
	for (auto postProcessVolumeActor : postProcessVolumes) {
		auto postProcessVolume = Cast<APostProcessVolume>(postProcessVolumeActor);
		if (postProcessVolume) {
			if (postProcessVolume->bUnbound) {
				result = postProcessVolume;
				break;
			}
		}
	}
	
	CYBERGAFFER_LOG(Log, TEXT("UCyberGafferSceneCaptureComponent2D::FindPostProcessVolume: found post process volume, name: %s"), *result->GetName());
	return result;
}

#if WITH_EDITOR
void UCyberGafferSceneCaptureComponent2D::OnComponentCreated() {
	Super::OnComponentCreated();
	CheckTextureTarget();
	CheckCaptureSettings();
	InitializeSubsystem();
	UpdateFOV();
}
#endif


void UCyberGafferSceneCaptureComponent2D::CheckCaptureSettings() {
	CYBERGAFFERVERB_LOG(Log, TEXT("UCyberGafferSceneCaptureComponent2D::CheckCaptureSettings"));

	// if(ProjectionType.GetValue() != ECameraProjectionMode::Type::Perspective) {
	// 	ProjectionType = ECameraProjectionMode::Type::Perspective;
	// }
	
	// PostProcessSettings = {};

	PostProcessSettings.bOverride_DynamicGlobalIlluminationMethod = true;
	PostProcessSettings.DynamicGlobalIlluminationMethod = EDynamicGlobalIlluminationMethod::Type::Lumen;
	
	PostProcessSettings.bOverride_LumenSurfaceCacheResolution = true;
	// PostProcessSettings.LumenSurfaceCacheResolution = 1;

	PostProcessSettings.bOverride_ReflectionMethod = true;
	PostProcessSettings.ReflectionMethod = EReflectionMethod::Lumen;
	
	PostProcessSettings.bOverride_LumenRayLightingMode = true;
	bUseRayTracingIfEnabled = 1;
	
	if (CaptureSource.GetValue() != SCS_FinalColorHDR) {
		CYBERGAFFER_LOG(Warning, TEXT("UCyberGafferSceneCaptureComponent2D::PostEditChangeProperty: Capture Source must be Final Color (HDR) in Linear Working Color Space, fixing it"));
		CaptureSource = SCS_FinalColorHDR;
	}
}

void UCyberGafferSceneCaptureComponent2D::UpdateFOV() {
	AActor* ownerActor = GetOwner();
	const float scale = ownerActor->GetActorScale3D().X;
	
	const double distanceToSphere = 300 * scale;
	const double radiusSphere = 5 * scale;

	switch (ProjectionType) {
		case ECameraProjectionMode::Perspective: {
			FOVAngle =  FMath::RadiansToDegrees(2 * asin(radiusSphere/distanceToSphere));
			break;
		}
			
		case ECameraProjectionMode::Orthographic: {
			OrthoWidth = radiusSphere * 2.0;
			break;
		}
		default: {
			CYBERGAFFER_LOG(Error, TEXT("UCyberGafferSceneCaptureComponent2D::UpdateFOV: unknown projection type %i"), ProjectionType);
			break;
		};
	}
	
	bOverride_CustomNearClippingPlane = true;
	CustomNearClippingPlane = (distanceToSphere - radiusSphere) - 1; //Additional indentation for fixing rendering errors.
}

void UCyberGafferSceneCaptureComponent2D::CheckTextureTarget() {
	CYBERGAFFERVERB_LOG(Log, TEXT("UCyberGafferSceneCaptureComponent2D::CheckTextureTarget"));

	auto targetSize = 128 * (uint8)SuperSampling;
	auto targetFormat = PF_FloatRGBA;
	
	if (TextureTarget == nullptr) {
		TextureTarget = NewObject<UTextureRenderTarget2D>();
		TextureTarget->InitCustomFormat(targetSize,targetSize, targetFormat, true);
		TextureTarget->UpdateResourceImmediate();
		
		if (TextureTarget == nullptr) {
			CYBERGAFFER_LOG(Error, TEXT("Failed to create UTextureRenderTarget2D"));
			return;
		}

		TextureTarget->TargetGamma = 0.0f;
		TextureTarget->AddressX = TA_Wrap;
		TextureTarget->AddressY = TA_Wrap;
		TextureTarget->ClearColor = FLinearColor::Black;
		
		// TODO: check if this is required by pipeline, MipGenSettings only presented in Editor
// #if WITH_EDITOR
// 		TextureTarget->MipGenSettings = TMGS_NoMipmaps;
// #endif
	}
	
	bool textureUpdated = false;

	if (!TextureTarget->bHDR_DEPRECATED) {
		CYBERGAFFER_LOG(Warning, TEXT("UCyberGafferSceneCaptureComponent2D::CheckTextureTarget: TextureRenderTargetCube must have bHDR property on, fixing it"));
		TextureTarget->bHDR_DEPRECATED = true;
		textureUpdated = true;
	}
	
	if (TextureTarget->SizeX != TextureTarget->SizeY || TextureTarget->SizeX != targetSize) {
		CYBERGAFFER_LOG(Warning, TEXT("UCyberGafferSceneCaptureComponent2D::CheckTextureTarget: TextureRenderTargetCube size must be power of 2, fixing it"));
		textureUpdated = true;
		TextureTarget->SizeX = targetSize;
		TextureTarget->SizeY = targetSize;
	}

	auto format = TextureTarget->GetFormat();
	if (format != targetFormat) {
		CYBERGAFFER_LOG(Warning, TEXT("UCyberGafferSceneCaptureComponent2D::CheckTextureTarget: TextureRenderTargetCube format must be RGBA16F, fixing it"));
		format = targetFormat;
		textureUpdated = true;
	}

	const auto compressionSettings = TextureTarget->CompressionSettings;
	if (compressionSettings.GetValue() != TC_HDR) {
		CYBERGAFFER_LOG(Warning, TEXT("UCyberGafferSceneCaptureComponent2D::CheckTextureTarget: TextureRenderTargetCube compression settings must be HDR (RGBA16F), fixing it"));
		TextureTarget->CompressionSettings = TC_HDR;
		textureUpdated = true;
	}

	if (textureUpdated) {
		TextureTarget->ReleaseResource();
		TextureTarget->InitCustomFormat(targetSize,targetSize, format, true);
		TextureTarget->UpdateResourceImmediate(true);
	}
}

bool UCyberGafferSceneCaptureComponent2D::InitializeSubsystem() {
	if (GEngine == nullptr) {
		CYBERGAFFER_LOG(Warning, TEXT("UCyberGafferSceneCaptureComponent2D::InitializeSubsystem: GEngine is null"));
		return false;
	}

	_subsystem = GEngine->GetEngineSubsystem<UCyberGafferEngineSubsystem>();
	return _subsystem != nullptr;
}

#if WITH_EDITOR
void UCyberGafferSceneCaptureComponent2D::PostEditChangeProperty(FPropertyChangedEvent& propertyChangedEvent) {
	Super::PostEditChangeProperty(propertyChangedEvent);
	
	const FName memberPropertyName = (propertyChangedEvent.MemberProperty != NULL) ? propertyChangedEvent.MemberProperty->GetFName() : NAME_None;
	
	CheckTextureTarget();
	CheckCaptureSettings();
	UpdateFOV();
	
	AActor* OwnerActor = GetOwner();
	ACyberGafferSceneCapture* captureActor = Cast<ACyberGafferSceneCapture>(OwnerActor);
	captureActor->UpdateChildTransforms();
}
#endif


void UCyberGafferSceneCaptureComponent2D::UpdateSceneCaptureContents(FSceneInterface* scene) {
	CheckTextureTarget();
	
	if (TextureTarget == nullptr) {
		return;
	}
	
	if (_subsystem == nullptr) {
		if (!InitializeSubsystem()) {
			CYBERGAFFER_LOG(Warning, TEXT("UCyberGafferSceneCaptureComponent2D::UpdateSceneCaptureContents: Subsystem is null"));
			return;
		}
	}

	if (ColorGradingPostProcessMaterial != nullptr) {
		if (GlobalPostProcessVolume == nullptr) {
			GlobalPostProcessVolume = FindPostProcessVolume();
			// TODO: Do we need to create a PPV in such way?
			if (GlobalPostProcessVolume == nullptr) {
				return;
			// 	FActorSpawnParameters spawnParameters;
			// 	spawnParameters.Name = FName(TEXT("CyberGafferPostProcessVolume"));
			// 	
			// 	GlobalPostProcessVolume = Cast<APostProcessVolume>(GetWorld()->SpawnActor(APostProcessVolume::StaticClass(), &FTransform::Identity, spawnParameters));
			// 	GlobalPostProcessVolume->bEnabled = true;
			// 	GlobalPostProcessVolume->bUnbound = true;
			}
		}

		if (GlobalPostProcessVolume != nullptr) {
			GlobalPostProcessVolume->AddOrUpdateBlendable(ColorGradingPostProcessMaterial, 0.0f);
		}
	}
	
	Super::UpdateSceneCaptureContents(scene);

	if (ColorGradingPostProcessMaterial != nullptr) {
		if (GlobalPostProcessVolume != nullptr) {
			GlobalPostProcessVolume->AddOrUpdateBlendable(ColorGradingPostProcessMaterial, 1.0f);
		}
	}
	
	if (!FMath::IsPowerOfTwo(TextureTarget->SizeX)) {
		CYBERGAFFER_LOG(Error, TEXT("UCyberGafferSceneCaptureComponent2D::UpdateSceneCaptureContents: the target cube texture side size must be power of 2. Recommended size: 512 or 1024"));
		return;
	}
	
	_subsystem->OnUpdateSceneCaptureContentsEnqueued(ServerIpAddress, ServerPort, TextureTarget);
}

#if WITH_EDITOR
void UCyberGafferSceneCaptureComponent2D::PostEditComponentMove(bool bFinished) {
	Super::PostEditComponentMove(bFinished);

	AActor* OwnerActor = GetOwner();
	ACyberGafferSceneCapture* captureActor = Cast<ACyberGafferSceneCapture>(OwnerActor);
	captureActor->UpdateChildTransforms();
}
#endif