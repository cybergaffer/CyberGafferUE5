// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Engine/SceneCapture.h"
#include "CyberGafferSceneCaptureCube.generated.h"

UCLASS(HideCategories = (Collision, Material, Attachment, Actor))
class CYBERGAFFER_API ACyberGafferSceneCaptureCube : public ASceneCapture {
	GENERATED_UCLASS_BODY()
private:
	UPROPERTY(Category = DecalActor, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true", DisplayName = "CyberGafferSceneCaptureComponentCube"))
	TObjectPtr<class UCyberGafferSceneCaptureComponentCube> _cyberGafferSceneCaptureComponentCube;

public:
	UFUNCTION(BlueprintCallable, Category = "Rendering")
	void OnInterpToggle(bool bEnable);

	class UCyberGafferSceneCaptureComponentCube* GetCyberGafferSceneCaptureComponentCube() const { return _cyberGafferSceneCaptureComponentCube; };
	
};
