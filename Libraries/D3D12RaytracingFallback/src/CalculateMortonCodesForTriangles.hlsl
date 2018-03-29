//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************
#define HLSL
#include "CalculateMortonCodesBindings.h"
#include "RayTracingHelper.hlsli"

RWStructuredBuffer<Triangle> InputBuffer : UAV_REGISTER(MortonCodeCalculatorInputBufferRegister);

float3 GetCentroid(uint elementIndex)
{
    Triangle tri = InputBuffer[elementIndex];
    return (tri.v0 + tri.v1 + tri.v2) / 3.0;
}
