#pragma once
#include "CyberGafferSphereComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CYBERGAFFER_API UCyberGafferSphereComponent : public UStaticMeshComponent
{
	GENERATED_BODY()
public:

#if WITH_EDITOR
	virtual void PostEditComponentMove(bool bFinished) override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& propertyChangedEvent) override;
#endif
};
