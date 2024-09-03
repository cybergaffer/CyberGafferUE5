#pragma once

#include "CoreMinimal.h"
#include "Components/SceneCaptureComponentCube.h"

#include "CyberGafferEngineSubsystem.h"

#include "CyberGafferSceneCaptureComponentCube.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CYBERGAFFER_API UCyberGafferSceneCaptureComponentCube : public USceneCaptureComponentCube {
	GENERATED_BODY()

public:
	UCyberGafferSceneCaptureComponentCube();

	// IP address of machine, where the CyberGaffer server is running.
	// In case of same machine "127.0.0.1" or "localhost" can be used. 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Server settings")
	FString ServerIpAddress = "127.0.0.1";

	// Port of CyberGaffer server
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Server settings")
	int32 ServerPort = 8080;

	// Should the cubemap be processed and sent to the CyberGaffer server
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CyberGaffer")
	bool ProcessCubeMap = true;

protected:
	virtual void BeginPlay() override;

public:
	virtual void UpdateSceneCaptureContents(FSceneInterface* scene) override;

#if WITH_EDITOR
	void PostEditChangeProperty(FPropertyChangedEvent& propertyChangedEvent) override;
#endif
	

private:
	UPROPERTY()
	UCyberGafferEngineSubsystem* _subsystem = nullptr;

	bool InitializeSubsystem();
	void CheckTextureTarget();
};
