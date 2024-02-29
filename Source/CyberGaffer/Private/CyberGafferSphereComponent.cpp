#include "CyberGafferSphereComponent.h"
#include "CyberGafferSceneCapture.h"

#if WITH_EDITOR
void UCyberGafferSphereComponent::PostEditChangeProperty(FPropertyChangedEvent& propertyChangedEvent) {
	Super::PostEditChangeProperty(propertyChangedEvent);

	AActor* OwnerActor = GetOwner();
	ACyberGafferSceneCapture* captureActor = Cast<ACyberGafferSceneCapture>(OwnerActor);
	captureActor->UpdateChildTransforms();
}

void UCyberGafferSphereComponent::PostEditComponentMove(bool bFinished)
{
	Super::PostEditComponentMove(bFinished);

	AActor* OwnerActor = GetOwner();
	ACyberGafferSceneCapture* captureActor = Cast<ACyberGafferSceneCapture>(OwnerActor);
	captureActor->UpdateChildTransforms();
}

#endif