#pragma once
#ifdef _DEBUG
#include "crtdbg.h"

#define DBGNEW new (_NORMAL_BLOCK, __FILE__, __LINE__)

class MemoryLeak
{
public:
    ~MemoryLeak()
    {
        _CrtDumpMemoryLeaks();
    }
};

const MemoryLeak cml;
#endif