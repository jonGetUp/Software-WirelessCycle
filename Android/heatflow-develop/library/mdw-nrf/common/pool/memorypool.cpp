/************************************************************************************************************
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2019
 * Created by Patrice Rudaz, Thomas Sterren
 * All rights reserved.
 ************************************************************************************************************/

/************************************************************************************************************/
/* Header files                                                                                             */
/************************************************************************************************************/
#include "memorypool.h"

/************************************************************************************************************/
/* Declaration section                                                                                      */
/************************************************************************************************************/
#if (USE_MEMORYPOOL != 0)   // Set USE_MEMORYPOOL to 1 to use/enable memory pool.

#define SMALL_REFCOUNT_SIZE     (MP_SMALL_OBJECTS_COUNT * sizeof(uint8_t))
#define MEDIUM_REFCOUNT_SIZE    (MP_MEDIUM_OBJECTS_COUNT * sizeof(uint8_t))
#define BIG_REFCOUNT_SIZE       (MP_BIG_OBJECTS_COUNT * sizeof(uint8_t))

static const uint32_t SMALL_MEMORY_SIZE = MP_SMALL_OBJECTS_COUNT * MP_SMALL_OBJECTS_SIZE;
static const uint32_t MEDIUM_MEMORY_SIZE = MP_MEDIUM_OBJECTS_COUNT * MP_MEDIUM_OBJECTS_SIZE;
static const uint32_t BIG_MEMORY_SIZE = MP_BIG_OBJECTS_COUNT * MP_BIG_OBJECTS_SIZE;

static uint8_t smallMemory[SMALL_MEMORY_SIZE + SMALL_REFCOUNT_SIZE] __attribute__((aligned(4)));
static uint8_t mediumMemory[MEDIUM_MEMORY_SIZE + MEDIUM_REFCOUNT_SIZE] __attribute__((aligned(4)));
static uint8_t bigMemory[BIG_MEMORY_SIZE + BIG_REFCOUNT_SIZE] __attribute__((aligned(4)));

uint8_t* pSmallMemory = (uint8_t*)smallMemory;
uint8_t* pMediumMemory = (uint8_t*)mediumMemory;
uint8_t* pBigMemory = (uint8_t*)bigMemory;

ObjectPool<MemoryPool::SmallObject OP_TP_PARAMS_IS> MemoryPool::smallPool(MP_SMALL_OBJECTS_COUNT, &pSmallMemory);
ObjectPool<MemoryPool::MediumObject OP_TP_PARAMS_IS> MemoryPool::mediumPool(MP_MEDIUM_OBJECTS_COUNT, &pMediumMemory);
ObjectPool<MemoryPool::BigObject OP_TP_PARAMS_IS> MemoryPool::bigPool(MP_BIG_OBJECTS_COUNT, &pBigMemory);

/************************************************************************************************************/
/* PUBLIC SECTION                                                                                           */
/************************************************************************************************************/
//------------------------------------------------------------------------------------------------------------
void* MemoryPool::allocate(size_t sz)
{
    void* p = NULL;

    if(sz <= MP_SMALL_OBJECTS_SIZE)
    {
        p = smallPool.allocate();
        ASSERT(p && "Too few small objects!");
    }
    else if(sz <= MP_MEDIUM_OBJECTS_SIZE)
    {
        p = mediumPool.allocate();
        ASSERT(p && "Too few medium objects!");
    }
    else if(sz <= MP_BIG_OBJECTS_SIZE)
    {
        p = bigPool.allocate();
        ASSERT(p && "Too few big objects!");
    }
    else
    {
        ASSERT(p && "Object size too big!");
    }
    return p;
}

//------------------------------------------------------------------------------------------------------------
void MemoryPool::release(void* p)
{
    if(isSmallObject(p))
    {
        smallPool.release((SmallObject*)p);
    }
    else if(isMediumObject(p))
    {
        mediumPool.release((MediumObject*)p);
    }
    else if(isBigObject(p))
    {
        bigPool.release((BigObject*)p);
    }
    else
    {
        ASSERT(false && "Not origin from this pool!");
    }
}

//------------------------------------------------------------------------------------------------------------
MemoryPool::PoolPtr MemoryPool::getPoolByNumber(uint16_t poolNumber)
{
    switch(poolNumber)
    {
        case 0:
            return reinterpret_cast<MemoryPool::PoolPtr>(&smallPool);
        case 1:
            return reinterpret_cast<MemoryPool::PoolPtr>(&mediumPool);
        case 2:
            return reinterpret_cast<MemoryPool::PoolPtr>(&bigPool);
        default:
            ASSERT(false);
            // Just in case...
            return reinterpret_cast<MemoryPool::PoolPtr>(&bigPool);
    }
}

//------------------------------------------------------------------------------------------------------------
MemoryPool::PoolPtr MemoryPool::getPoolByObjectSize(size_t objectSize)
{
    if(objectSize <= MP_SMALL_OBJECTS_SIZE)
    {
        return reinterpret_cast<MemoryPool::PoolPtr>(&smallPool);
    }
    else if(objectSize <= MP_MEDIUM_OBJECTS_SIZE)
    {
        return reinterpret_cast<MemoryPool::PoolPtr>(&mediumPool);
    }
    else if(objectSize <= MP_BIG_OBJECTS_SIZE)
    {
        return reinterpret_cast<MemoryPool::PoolPtr>(&bigPool);
    }
    else
    {
        ASSERT(false);
        // Just in case...
        return reinterpret_cast<MemoryPool::PoolPtr>(&bigPool);
    }
}

//------------------------------------------------------------------------------------------------------------
bool MemoryPool::isSmallObject(void* p)
{
    const uint8_t* const pMem = reinterpret_cast<uint8_t*>(p);
    return (pMem >= smallMemory) && (pMem < (smallMemory + SMALL_MEMORY_SIZE));
}

//------------------------------------------------------------------------------------------------------------
bool MemoryPool::isMediumObject(void* p)
{
    const uint8_t* const pMem = reinterpret_cast<uint8_t*>(p);
    return (pMem >= mediumMemory) && (pMem < (mediumMemory + MEDIUM_MEMORY_SIZE));
}

//------------------------------------------------------------------------------------------------------------
bool MemoryPool::isBigObject(void* p)
{
    const uint8_t* const pMem = reinterpret_cast<uint8_t*>(p);
    return (pMem >= bigMemory) && (pMem < (bigMemory + BIG_MEMORY_SIZE));
}

#endif // USE_MEMORYPOOL
