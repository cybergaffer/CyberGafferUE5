#include "CyberGafferSphereComponent.h"
#include "CyberGafferSceneCapture.h"

#if WITH_EDITOR
void UCyberGafferSphereComponent::PostEditChangeProperty(FPropertyChangedEvent& propertyChangedEvent) {
	Super::PostEditChangeProperty(propertyChangedEvent);

	AActor* ownerActor = GetOwner();
	ACyberGafferSceneCapture* captureActor = Cast<ACyberGafferSceneCapture>(ownerActor);
	captureActor->UpdateChildTransforms();
}

void UCyberGafferSphereComponent::PostEditComponentMove(bool bFinished) {
	Super::PostEditComponentMove(bFinished);

	AActor* ownerActor = GetOwner();
	ACyberGafferSceneCapture* captureActor = Cast<ACyberGafferSceneCapture>(ownerActor);
	captureActor->UpdateChildTransforms();
}

#endif