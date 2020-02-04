#pragma once
#include<iostream>

//�ڴ��ͷ��Ϣ
class MemoryBlockHeader
{
public:
	MemoryBlockHeader * next = nullptr;			//��һ���ڴ��ͷ��ַ
	int			id = -1;							//�ڴ����
	bool		used = false;						//�Ƿ�ʹ��
	char a = '\0', b = '\0', c = '\0';				//�ڴ���룬�����ֽ�,��ʵ������
};
//int sizet = sizeof(MemoryBlockHeader);
//�ڴ�ػ���
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
			//�����ڴ�ʧ�ܣ��˳�
			//д��־ LOG_ERROR<<<< "�����ڴ�ʧ�ܣ��˳�" 
			std::cout << "�����ڴ�ʧ�ܣ��˳�" << std::endl;
			exit(-1);
		}

		MemoryBlockHeader* tMemoryBlockHeader = nullptr;
		//����0-m_size-2������ϵ
		for (int i = m_size - 2; i >= 0; i--)
		{
			tMemoryBlockHeader = (MemoryBlockHeader*)((char*)m_ptr + (i* perBlockMemoryRallSize));
			tMemoryBlockHeader->used = false;
			tMemoryBlockHeader->id = i;
			tMemoryBlockHeader->next = (MemoryBlockHeader*)((char*)m_ptr + ((i + 1)* perBlockMemoryRallSize));
		}
		//���һ���ڴ���nextΪ�գ�
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
	int m_per_size = 0;						//ÿһ���ڴ��Ĵ�С
	int m_size = 0;							//�ڴ��Ŀ���
	void *m_ptr = nullptr;					//������ڴ��׵�ַ
	void *m_nextCanUseMemHeader = nullptr;	//��һ�����õ��ڴ��ͷ��ַ
};

//�ڴ�����࣬�ж���ڴ��
//0-64�ֽ�һ��
//65-128�ֽ�һ��
//128-512�ֽ�һ��
//513-1024�ֽ�һ��
//1025-4096�ֽ�һ��
class CMemoryMgr
{
public:
	//����ģʽ
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
		//�����������ֱ��free��
		free(p);
	}
private:

	CMemoryMgr(){
		Init();
	}
	~CMemoryMgr() {

	}

	CMemoryPool *mem0_64;			 //0-64�ֽڵ��ڴ��
	int mem0_64_max_size = 100;			 //��ʼ��ʱ�����ڴ����
	int mem0_64_used_size = 0;		 //��ʹ�õ��ڴ����

	CMemoryPool *mem65_128;
	int mem65_128_max_size = 100;			 //��ʼ��ʱ�����ڴ����
	int mem65_128_used_size = 0;	 //��ʹ�õ��ڴ����

	CMemoryPool *mem129_512;
	int mem129_512_max_size = 100;		 //��ʼ��ʱ�����ڴ����
	int mem129_512_used_size = 0;	 //��ʹ�õ��ڴ����

	CMemoryPool *mem513_1024;
	int mem513_1024_max_size = 100;		 //��ʼ��ʱ�����ڴ����
	int mem513_1024_used_size = 0;	 //��ʹ�õ��ڴ����

	CMemoryPool *mem1025_10240;
	int mem1025_10240_max_size = 100;		 //��ʼ��ʱ�����ڴ����
	int mem1025_10240_used_size = 0; //��ʹ�õ��ڴ����

	bool InitMem(CMemoryPool **pmem, int per_size, int max_size)
	{
		*pmem = (CMemoryPool*)malloc(sizeof(CMemoryPool));
		if (*pmem == nullptr)
		{
			//�����ڴ�ʧ�ܣ��˳�
			//д��־ LOG_ERROR<<<< "�����ڴ�ʧ�ܣ��˳�" 
			std::cout << "�����ڴ�ʧ�ܣ��˳�" << std::endl;
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
