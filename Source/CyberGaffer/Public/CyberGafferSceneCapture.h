#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Engine/SceneCapture.h"
#include "Materials/Material.h"

#include "CyberGafferSceneCapture.generated.h"

UCLASS(HideCategories = (Collision, Material, Attachment, Actor))
class CYBERGAFFER_API ACyberGafferSceneCapture : public ASceneCapture {
	GENERATED_UCLASS_BODY()
private:
	UPROPERTY(Category = DecalActor, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true", DisplayName = "CyberGafferSceneCaptureComponent"))
	TObjectPtr<class UCyberGafferSceneCaptureComponent2D> _cyberGafferSceneCaptureComponent2D;
	UPROPERTY(Category = DecalActor, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true", DisplayName = "CyberGafferSceneCaptureTaskSphere"))
	TObjectPtr<class UStaticMeshComponent> _taskSphere;
	UPROPERTY(Category = DecalActor, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true", DisplayName = "CyberGafferSceneCaptureTaskSphere"))
	TObjectPtr<class UStaticMeshComponent> _gizmoSphere;
	UPROPERTY()
	UMaterial* BaseSphereMaterial;
	
public:
	virtual void PostLoad() override;
	void OnInterpToggle(bool bEnable);
#if WITH_EDITOR
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	void UpdateChildTransforms();
};
