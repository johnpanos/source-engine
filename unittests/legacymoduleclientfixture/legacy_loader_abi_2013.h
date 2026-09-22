// Frozen declarations for the pre-Phase-A dynamic loader ABI.
// This fixture must not include the current tier1/interface.h.
#ifndef LEGACY_LOADER_ABI_2013_H
#define LEGACY_LOADER_ABI_2013_H

class CSysModule;

enum Sys_Flags
{
	SYS_NOFLAGS = 0x00,
	SYS_NOLOAD = 0x01
};

typedef void *(*CreateInterfaceFn)( const char *pName, int *pReturnCode );

extern CSysModule *Sys_LoadModule(
	const char *pModuleName, Sys_Flags flags = SYS_NOFLAGS );
extern void Sys_UnloadModule( CSysModule *pModule );
extern CreateInterfaceFn Sys_GetFactory( CSysModule *pModule );

#endif // LEGACY_LOADER_ABI_2013_H
