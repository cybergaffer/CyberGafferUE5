#pragma once

#include "CoreMinimal.h"
#include "Components/SceneCaptureComponent2D.h"

#include "CyberGafferEngineSubsystem.h"
#include "CyberGafferSuperSampling.h"

#include "CyberGafferSceneCaptureComponent2D.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CYBERGAFFER_API UCyberGafferSceneCaptureComponent2D : public USceneCaptureComponent2D {
	GENERATED_BODY()

public:
	UCyberGafferSceneCaptureComponent2D();
	
	// IP address of machine, where the CyberGaffer server is running.
	// In case of same machine "127.0.0.1" or "localhost" can be used. 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Server settings")
	FString ServerIpAddress = "127.0.0.1";

	// Port of CyberGaffer server
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Server settings")
	int32 ServerPort = 8080;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Server settings")
	ECyberGafferSuperSampling SuperSampling = ECyberGafferSuperSampling::x64;
	
	UCyberGafferEngineSubsystem* _subsystem;

	// Should the cubemap be processed and sent to the CyberGaffer server
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CyberGaffer")
	//bool ProcessCubeMap = true;

#if WITH_EDITOR
	virtual void PostEditComponentMove(bool bFinished) override;
#endif

protected:
	void CheckTextureTarget();
	bool InitializeSubsystem();
	virtual void PostEditChangeProperty(FPropertyChangedEvent& propertyChangedEvent) override;
	virtual void UpdateSceneCaptureContents(FSceneInterface* scene) override;
	void CheckCaptureSettings();
	virtual void BeginPlay() override;
};