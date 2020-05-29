#pragma once

#include "Payloads.h"
#include "AttribStructs.h"
#include "RootArguments.h"

typedef UINT16 Index;

struct Vertex
{
	XMFLOAT3 position;
	XMFLOAT3 normal;
};

#define MAX_RAY_RECURSION_DEPTH 3