// Copyright (c) 2023 ALT LLC.
//
// Distributed under the terms of the MIT License (see the accompanying LICENSE.txt file or get a copy at http://opensource.org/licenses/MIT).


#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Engine/SceneCapture.h"
#include "CyberGafferSceneCapture.generated.h"

UCLASS(HideCategories = (Collision, Material, Attachment, Actor))
class CYBERGAFFER_API ACyberGafferSceneCapture : public ASceneCapture {
	GENERATED_UCLASS_BODY()
private:
	UPROPERTY(Category = DecalActor, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true", DisplayName = "CyberGafferSceneCaptureComponentCube"))
	TObjectPtr<class UCyberGafferSceneCaptureComponent2D> _cyberGafferSceneCaptureComponent2D;
	UPROPERTY(Category = DecalActor, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true", DisplayName = "CyberGafferSceneCaptureTaskSphere"))
	TObjectPtr<class UStaticMeshComponent> _taskSphere;
	UPROPERTY(Category = DecalActor, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true", DisplayName = "CyberGafferSceneCaptureTaskSphere"))
	TObjectPtr<class UStaticMeshComponent> _gizmoSphere;
	UMaterial* BaseSphereMaterial;
	
public:
	UFUNCTION(BlueprintCallable, Category = "Rendering")
	void OnInterpToggle(bool bEnable);

	class UCyberGafferSceneCaptureComponentCube* GetCyberGafferSceneCaptureComponentCube() const { return nullptr; };
	void BeginPlay() override;
	void PostActorCreated() override;
	
};
