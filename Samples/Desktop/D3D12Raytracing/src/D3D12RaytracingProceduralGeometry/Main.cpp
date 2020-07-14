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

#include "stdafx.h"

#define OUR_SAMPLE
#ifdef OUR_SAMPLE
    #include "ProceduralRtxEngineSample.h"
#else
    #include "D3D12RaytracingProceduralGeometry.h"
#endif


_Use_decl_annotations_
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
#ifdef OUR_SAMPLE
    ProceduralRtxEngineSample sample(1280, 720, L"D3D12 Raytracing - Procedural Geometry");
#else
    D3D12RaytracingProceduralGeometry sample(1280, 720, L"D3D12 Raytracing - Procedural Geometry");
#endif
    return Win32Application::Run(&sample, hInstance, nCmdShow);
}
