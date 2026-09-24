#ifndef ISPSHAREDMEMORY_H
#define ISPSHAREDMEMORY_H
#ifdef _WIN32
#pragma once
#endif

class ISPSharedMemory
{
public:
	virtual void Init() = 0;
	virtual void *Base() = 0;
	virtual void Release() = 0;
};

#endif // ISPSHAREDMEMORY_H
