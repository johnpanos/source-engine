//========= Copyright Valve Corporation, All rights reserved. ============//
// Sensitivity: errors of different domains must not compare.
#include "foundation/error.h"
enum class FileStatus
{
	NotFound,
};
enum class NetStatus
{
	NotFound,
};
enum class Operation
{
	Open,
};
int main()
{
	const foundation::Error<FileStatus, Operation> file{};
	const foundation::Error<NetStatus, Operation> net{};
	return file == net ? 0 : 1;
}
