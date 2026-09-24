#ifndef ISPSHAREDMEMORY_H
#define ISPSHAREDMEMORY_H
#ifdef _WIN32
#pragma once
#endif

class ISPSharedMemory
{
public:
	virtual void Init() = 0;
	void Init( size_t nBytes )
	{
		// The retained engine ABI owns the allocation size.
		(void)nBytes;
		Init();
	}
	virtual void *Base() = 0;
	virtual void Release() = 0;
};

#endif // ISPSHAREDMEMORY_H
