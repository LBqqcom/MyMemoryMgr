# MyMemoryMgr 介绍：
非常简单的内存池管理实现

这是用C++实现的一个内存池管理工具
包含三个文件：
CMyMemoryMgr.h     内存池管理直接在类里面实现
CMyMemoryMgr.cpp   重载全局函数new和delete操作符
MyMemoryMgrTestMain.cpp    内存池管理的测试例子

使用： (1)把CMyMemoryMgr.h和CMyMemoryMgr.cpp放到自己的工程中
      (2)在文件头部#include"CMyMemoryMgr.h"
      (3)在代码的任何地方都像以前一样的使用new和delete操作

说明： 重载全局函数new和delete操作符，实现在new和delete时调用我们写的new和delete函数，
      我们的new和delete函数中，调用 CMemoryMgr::Instance().mallocMem(size);分配内存。
      
      CMemoryMgr类：是一个单例类，每个CMemoryMgr有多个内存池：0-64字节一个，65-128字节一个，128-512字节一个，
                    513-1024字节一个，1025-10240字节一个。超过10240的直接调用malloc分配free释放。
      
      CMemoryPool类：具体的一个内存池，每块内存大小为：m_per_size字节，有m_size块。
                    初始化时分配一块大的内存，安排好0的next指向1,1的next指向2，i的next指向i+1，最后的next指向nullptr。
                    每个内存块包含内存块头信息和实际分配给用户使用的内存。
      内存池示意图：           
                              | <——————-实际分配给用户使用的内存块的地址
                     ——————————————————————
                    | 1       |            |   <-实际分配给用户使用的内存块
                     —————————————————————— 
                    | 2       |            |
                     ——————————————————————     
                    | ...     |            |
                     ——————————————————————    
                    | n       |            |
                     ——————————————————————    
     内存块示意图：
                      内存块头信息         实际分配给用户使用的内存块
                     ______________________________________________
                    | MemoryBlockHeader  |                        |
                     ——————————————————————————————————————————————
                     
