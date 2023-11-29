// Copyright (c) 2023 ALT LLC.
//
// Distributed under the terms of the MIT License (see the accompanying LICENSE.txt file or get a copy at http://opensource.org/licenses/MIT).


#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"

#include "Engine/TextureRenderTargetCube.h"
#include "Templates/SharedPointer.h"
#include "RHIResources.h"

#include "CyberGafferDataSender.h"

#include "CyberGafferEngineSubsystem.generated.h"

/**
 *  
 */
UCLASS()
class UCyberGafferEngineSubsystem : public UEngineSubsystem {
	GENERATED_BODY()
public:
	// USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& collection) override;
	virtual void Deinitialize() override;

	// Add commands for the render thread to process a result scene capture texture and send it to the server.
	// Should be called right after the scene capture component added his task to the render queue,
	// so the texture processing will be started with minimal delay after the texture is ready.
	void OnUpdateSceneCaptureContentsEnqueued(const FString serverIp, const uint32_t port, UTextureRenderTargetCube* texture);
	void OnUpdateSceneCaptureContentsEnqueued(FString serverIp, uint32_t port, UTextureRenderTarget2D* texture);

private:
	TSharedRef<FCyberGafferDataSender> _dataSender = MakeShared<FCyberGafferDataSender>();
};
