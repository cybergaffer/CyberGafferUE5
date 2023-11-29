#include "CyberGafferSceneCaptureComponent2D.h"

#include "Engine/TextureRenderTargetCube.h"
#include "Engine/Engine.h"
#include "Engine/Texture.h"
#include "Engine/TextureDefines.h"
#include "Math/UnrealMathUtility.h"
#include "PixelFormat.h"

#include "CyberGafferLog.h"
#include "LumenVisualizationData.h"
#include "Engine/TextureRenderTarget2D.h"

#include "PrimitiveSceneProxy.h"
#include "SceneManagement.h"

UCyberGafferSceneCaptureComponent2D::UCyberGafferSceneCaptureComponent2D() {
	PrimaryComponentTick.bCanEverTick = true;

	// TODO: find a way to mark this properties uneditable. Using the ClearPropertyFlags(CPF_Edit) will change the parent class as well. 
	bCaptureEveryFrame = true;
	CaptureSource = SCS_FinalColorHDR;

	FString pathTexture = "/Script/Engine.TextureRenderTarget2D'/CyberGaffer/TaskRender.TaskRender'";
	TextureTarget = Cast<UTextureRenderTarget2D>(StaticLoadObject(UTextureRenderTarget2D::StaticClass(), nullptr, *pathTexture));
	
	CheckCaptureSettings();
}

void UCyberGafferSceneCaptureComponent2D::BeginPlay() {
	Super::BeginPlay();

	CheckCaptureSettings();
	CheckTextureTarget();
	InitializeSubsystem();
}

void UCyberGafferSceneCaptureComponent2D::CheckCaptureSettings()
{
	CYBERGAFFERVERB_LOG(Log, TEXT("UCyberGafferSceneCaptureComponent2D::CheckCaptureSettings"));

	if(ProjectionType.GetValue() != ECameraProjectionMode::Type::Orthographic)
	{
		ProjectionType = ECameraProjectionMode::Type::Orthographic;
	}

	OrthoWidth = 100;
	PostProcessSettings = {};

	PostProcessSettings.bOverride_DynamicGlobalIlluminationMethod = true;
	PostProcessSettings.DynamicGlobalIlluminationMethod = EDynamicGlobalIlluminationMethod::Type::Lumen;
	
	PostProcessSettings.bOverride_LumenSurfaceCacheResolution = true;
	PostProcessSettings.LumenSurfaceCacheResolution= 1;

	PostProcessSettings.bOverride_ReflectionMethod = true;
	PostProcessSettings.ReflectionMethod = EReflectionMethod::Lumen;
}

void UCyberGafferSceneCaptureComponent2D::CheckTextureTarget()
{
	CYBERGAFFERVERB_LOG(Log, TEXT("UCyberGafferSceneCaptureComponent2D::CheckTextureTarget"));

	if (TextureTarget == nullptr) {
		return;
	}

	bool textureUpdated = false;

	if (!TextureTarget->bHDR_DEPRECATED) {
		CYBERGAFFER_LOG(Warning, TEXT("UCyberGafferSceneCaptureComponent2D::CheckTextureTarget: TextureRenderTargetCube must have bHDR property on, fixing it"));
		TextureTarget->bHDR_DEPRECATED = true;
		textureUpdated = true;
	}

	auto sizeX = TextureTarget->SizeX;
	if (!FMath::IsPowerOfTwo(sizeX)) {
		CYBERGAFFER_LOG(Warning, TEXT("UCyberGafferSceneCaptureComponent2D::CheckTextureTarget: TextureRenderTargetCube size must be power of 2, fixing it"));
		sizeX = FMath::RoundUpToPowerOfTwo(sizeX);
		textureUpdated = true;
		TextureTarget->SizeX = sizeX;
	}

	auto format = TextureTarget->GetFormat();
	if (format != PF_FloatRGBA) {
		CYBERGAFFER_LOG(Warning, TEXT("UCyberGafferSceneCaptureComponent2D::CheckTextureTarget: TextureRenderTargetCube format must be RGBA16F, fixing it"));
		format = PF_FloatRGBA;
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
		TextureTarget->InitCustomFormat(sizeX,sizeX, format, true);
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
	
	if (memberPropertyName.IsEqual("TextureTarget")) {
		CheckTextureTarget();
		return;
	}

	CheckCaptureSettings();

	if (memberPropertyName.IsEqual("CaptureSource")) {
		if (CaptureSource.GetValue() != SCS_FinalColorHDR) {
			CYBERGAFFER_LOG(Warning, TEXT("UCyberGafferSceneCaptureComponentCube::PostEditChangeProperty: Capture Source must be Final Color (HDR) in Linear Working Color Space, fixing it"));
			CaptureSource = SCS_FinalColorHDR;
		}
		return;
	}
}
#endif

void UCyberGafferSceneCaptureComponent2D::UpdateSceneCaptureContents(FSceneInterface* scene) {

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
	
	_subsystem->OnUpdateSceneCaptureContentsEnqueued(ServerIpAddress, ServerPort, TextureTarget);
}

#if WITH_EDITOR
void UCyberGafferSceneCaptureComponent2D::PostEditComponentMove(bool bFinished)
{
	Super::PostEditComponentMove(bFinished);
	CYBERGAFFER_LOG(Log, TEXT("PostEditComponentMove"));
}
#endif