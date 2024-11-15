#pragma once

UENUM()
enum class ECyberGafferSuperSampling : uint8 {
	None = 1 UMETA(DisplayName = "None"),
	x4 = 2 UMETA(DisplayName = "4x"),
	x16 = 4 UMETA(DisplayName = "16x"),
	x64 = 8 UMETA(DisplayName = "64x")
};