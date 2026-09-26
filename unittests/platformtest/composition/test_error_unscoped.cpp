//========= Copyright Valve Corporation, All rights reserved. ============//
// Sensitivity: an error's status must be a domain's scoped enum, not an integer.
#include "foundation/error.h"
enum class Operation
{
	Open,
};
int main()
{
	foundation::Error<int, Operation> error{};
	return error.nativeCode;
}
