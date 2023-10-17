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
		TArray<float> rgbFloatData;
		rgbFloatData.Reserve(colors.Num() * 3);
	
		for (auto i = 0; i < colors.Num(); ++i) {
			rgbFloatData.Add(colors[i].R.GetFloat());
			rgbFloatData.Add(colors[i].G.GetFloat());
			rgbFloatData.Add(colors[i].B.GetFloat());
		}
		
		const auto byteDataSize = rgbFloatData.Num() * 4;
		Data.Empty(byteDataSize);
		Data.SetNum(byteDataSize);
		std::memcpy(Data.GetData(), rgbFloatData.GetData(), byteDataSize);		
	}
};
