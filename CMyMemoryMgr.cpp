#include "CMyMemoryMgr.h"


void * operator new(size_t size)
{
	//return malloc(size);
	return CMemoryMgr::Instance().mallocMem(size);
}

void operator delete(void * p)
{
	//free(p);
	CMemoryMgr::Instance().freeMem(p);
}

void * operator new[](size_t size)
{
	//return malloc(size);
	return CMemoryMgr::Instance().mallocMem(size);
}

void operator delete[](void * p)
{
	//free(p);
	CMemoryMgr::Instance().freeMem(p);
}
