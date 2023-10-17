// Copyright (c) 2023 Exostartup LLC.
//
// Distributed under the terms of the MIT License (see the accompanying LICENSE.txt file or get a copy at http://opensource.org/licenses/MIT).


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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Server settings")
	FString ServerIpAddress = "127.0.0.1";
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Server settings")
	int32 ServerPort = 8080;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool ProcessCubeMap = true;

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float deltaTime, ELevelTick tickType, FActorComponentTickFunction* thisTickFunction) override;
	virtual void UpdateSceneCaptureContents(FSceneInterface* scene) override;

private:
	UPROPERTY()
	UCyberGafferEngineSubsystem* _subsystem = nullptr;

	bool InitializeSubsystem();
};
