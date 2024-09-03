#include "CyberGafferShaders.h"

IMPLEMENT_SHADER_TYPE(, FPackCubeMap_VS, TEXT("/CyberGaffer/Private/PackCubeMap.usf"), TEXT("MainVS"), SF_Vertex);
IMPLEMENT_SHADER_TYPE(, FPackCubeMap_PS, TEXT("/CyberGaffer/Private/PackCubeMap.usf"), TEXT("MainPS"), SF_Pixel);
IMPLEMENT_SHADER_TYPE(, FMipGeneration_PS, TEXT("/CyberGaffer/Private/MipGeneration.usf"), TEXT("MainPS"), SF_Pixel);
