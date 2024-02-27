// Copyright (c) 2023 ALT LLC.
//
// Distributed under the terms of the MIT License (see the accompanying LICENSE.txt file or get a copy at http://opensource.org/licenses/MIT).


#pragma once

#include "CoreMinimal.h"

struct FCyberGafferDataPackage {
public:
	FString ServerIpAddress = "localhost";
	uint32_t ServerPort = 8080;
	TArray<uint8> Data;

	FCyberGafferDataPackage() = default;
	~FCyberGafferDataPackage() = default;

	FCyberGafferDataPackage(const FString& serverIp, const uint32_t serverPort, TArray<uint8> data) :
		ServerIpAddress(serverIp),
		ServerPort(serverPort),
		Data(MoveTemp(data)) {}

	FCyberGafferDataPackage(const FString& serverIp, const uint32_t serverPort, const TArray<FFloat16Color>& colors) :
		ServerIpAddress(serverIp),
		ServerPort(serverPort)
	{
		TArray<float> rgbaFloatData;
		rgbaFloatData.Reserve(colors.Num() * 4);
	
		for (auto i = 0; i < colors.Num(); ++i) {
			rgbaFloatData.Add(colors[i].R.GetFloat());
			rgbaFloatData.Add(colors[i].G.GetFloat());
			rgbaFloatData.Add(colors[i].B.GetFloat());
			rgbaFloatData.Add(1);
		}
		
		const auto byteDataSize = rgbaFloatData.Num() * 4;
		Data.Empty(byteDataSize);
		Data.SetNum(byteDataSize);
		std::memcpy(Data.GetData(), rgbaFloatData.GetData(), byteDataSize);		
	}
};
