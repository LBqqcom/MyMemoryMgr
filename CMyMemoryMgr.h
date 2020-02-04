#pragma once
#include<iostream>

//内存块头信息
class MemoryBlockHeader
{
public:
	MemoryBlockHeader * next = nullptr;			//下一个内存块头地址
	int			id = -1;							//内存块编号
	bool		used = false;						//是否使用
	char a = '\0', b = '\0', c = '\0';				//内存对齐，保留字节,无实际作用
};
//int sizet = sizeof(MemoryBlockHeader);
//内存池基类
class CMemoryPool
{
public:
	CMemoryPool()
	{
	}
	~CMemoryPool()
	{
		if (m_ptr != nullptr)
		{
			free(m_ptr);
		}

	}
	bool Init(int _per_size, int _size = 100)
	{
		m_per_size = _per_size;
		m_size = _size;

		int perBlockMemoryRallSize = sizeof(MemoryBlockHeader) + m_per_size;
		m_ptr = malloc(perBlockMemoryRallSize*m_size);
		if (m_ptr == nullptr)
		{
			//分配内存失败，退出
			//写日志 LOG_ERROR<<<< "分配内存失败，退出" 
			std::cout << "分配内存失败，退出" << std::endl;
			exit(-1);
		}

		MemoryBlockHeader* tMemoryBlockHeader = nullptr;
		//后面0-m_size-2建立关系
		for (int i = m_size - 2; i >= 0; i--)
		{
			tMemoryBlockHeader = (MemoryBlockHeader*)((char*)m_ptr + (i* perBlockMemoryRallSize));
			tMemoryBlockHeader->used = false;
			tMemoryBlockHeader->id = i;
			tMemoryBlockHeader->next = (MemoryBlockHeader*)((char*)m_ptr + ((i + 1)* perBlockMemoryRallSize));
		}
		//最后一块内存块的next为空；
		MemoryBlockHeader* tLastMemoryBlockHeader = (MemoryBlockHeader*)((char*)m_ptr + ((m_size - 1)* perBlockMemoryRallSize));
		tLastMemoryBlockHeader->used = false;
		tLastMemoryBlockHeader->next = nullptr;
		tLastMemoryBlockHeader->id = m_size - 1;

		m_nextCanUseMemHeader = m_ptr;

		return true;
	}
	void *mallocMem(size_t size)
	{
		MemoryBlockHeader* tp=nullptr;
		if (m_nextCanUseMemHeader != nullptr)
		{
			tp = (MemoryBlockHeader*)m_nextCanUseMemHeader;
			tp->used = true;
			m_nextCanUseMemHeader = ((MemoryBlockHeader*)m_nextCanUseMemHeader)->next;
			return (void *)((char*)tp + sizeof(MemoryBlockHeader));
		}
		return nullptr;

	}
	void  freeMem(void *p)
	{
		MemoryBlockHeader* tp = nullptr;
		tp = (MemoryBlockHeader*)((char*)p - sizeof(MemoryBlockHeader));
		tp->next = (MemoryBlockHeader*)m_nextCanUseMemHeader;
		m_nextCanUseMemHeader = tp;
		
	}
	bool isInPool(void *p)
	{
		if (p > m_ptr&&p < (char*)m_ptr + ((m_size - 1) * sizeof(MemoryBlockHeader) + m_per_size))
		{
			return true;
		}
		return false;
	}
private:
	int m_per_size = 0;						//每一个内存块的大小
	int m_size = 0;							//内存块的块数
	void *m_ptr = nullptr;					//分配的内存首地址
	void *m_nextCanUseMemHeader = nullptr;	//下一个可用的内存块头地址
};

//内存管理类，有多个内存池
//0-64字节一个
//65-128字节一个
//128-512字节一个
//513-1024字节一个
//1025-4096字节一个
class CMemoryMgr
{
public:
	//单例模式
	static CMemoryMgr&Instance()
	{
		static CMemoryMgr memoryMgr;
		return memoryMgr;
	}

	bool Init()
	{
		bool bret = false;
		do {
			if (!InitMem(&mem0_64, 64, mem0_64_max_size)) break;
			if (!InitMem(&mem65_128, 128, mem65_128_max_size)) break;
			if (!InitMem(&mem129_512, 512, mem129_512_max_size)) break;
			if (!InitMem(&mem513_1024, 1024, mem513_1024_max_size)) break;
			if (!InitMem(&mem1025_10240, 10240, mem1025_10240_max_size)) break;
			bret = true;
		} while (false);

		return bret;
	}
	void *mallocMem(size_t size)
	{
		void *p=nullptr;
		if (size < 64) { p = mem0_64->mallocMem(size);         if (!p) { return nullptr; } mem0_64_used_size++;       return p; }

		if (size < 128) { p = mem65_128->mallocMem(size);      if (!p) { return nullptr; } mem65_128_used_size++;     return p;}

		if (size < 512) { p = mem129_512->mallocMem(size);      if (!p) { return nullptr; } mem129_512_used_size++;    return p;}

		if (size < 1024) { p = mem513_1024->mallocMem(size);    if (!p) { return nullptr; } mem513_1024_used_size++;   return p; }

		if (size < 10240) { p = mem1025_10240->mallocMem(size); if (!p) { return nullptr; } mem1025_10240_used_size++; return p; }

		return malloc(size);

	}
	void  freeMem(void *p)
	{
		if (mem0_64->isInPool(p)) { mem0_64->freeMem(p); mem0_64_used_size--; return; }
		if (mem65_128->isInPool(p)) { mem65_128->freeMem(p); mem65_128_used_size--; return; }
		if (mem129_512->isInPool(p)) { mem129_512->freeMem(p); mem129_512_used_size--; return; }
		if (mem513_1024->isInPool(p)) { mem513_1024->freeMem(p); mem513_1024_used_size--; return; }
		if (mem1025_10240->isInPool(p)) { mem1025_10240->freeMem(p); mem1025_10240_used_size--; return; }
		//如果都不在则，直接free掉
		free(p);
	}
private:

	CMemoryMgr(){
		Init();
	}
	~CMemoryMgr() {

	}

	CMemoryPool *mem0_64;			 //0-64字节的内存池
	int mem0_64_max_size = 100;			 //初始化时分配内存块数
	int mem0_64_used_size = 0;		 //已使用的内存块数

	CMemoryPool *mem65_128;
	int mem65_128_max_size = 100;			 //初始化时分配内存块数
	int mem65_128_used_size = 0;	 //已使用的内存块数

	CMemoryPool *mem129_512;
	int mem129_512_max_size = 100;		 //初始化时分配内存块数
	int mem129_512_used_size = 0;	 //已使用的内存块数

	CMemoryPool *mem513_1024;
	int mem513_1024_max_size = 100;		 //初始化时分配内存块数
	int mem513_1024_used_size = 0;	 //已使用的内存块数

	CMemoryPool *mem1025_10240;
	int mem1025_10240_max_size = 100;		 //初始化时分配内存块数
	int mem1025_10240_used_size = 0; //已使用的内存块数

	bool InitMem(CMemoryPool **pmem, int per_size, int max_size)
	{
		*pmem = (CMemoryPool*)malloc(sizeof(CMemoryPool));
		if (*pmem == nullptr)
		{
			//分配内存失败，退出
			//写日志 LOG_ERROR<<<< "分配内存失败，退出" 
			std::cout << "分配内存失败，退出" << std::endl;
			exit(-1);
		}
		(*pmem)->Init(per_size, max_size);

		return true;
	}
	

};

void * operator new(size_t size);
void   operator delete(void * p);

void * operator new[](size_t size);
void   operator delete[](void * p);
