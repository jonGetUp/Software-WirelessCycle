#ifndef MEMORYPOOL_CONFIG_H
#define MEMORYPOOL_CONFIG_H

/*
 * $Id: memorypool-config.h 234 2017-04-27 07:11:48Z patrice.rudaz $
 * $Author: patrice.rudaz $
 * $Revision: 110 $
 */

#define USE_MEMORYPOOL				1		///< Set to 1 if you need to use the memory pool (class MemoryPool).
#define MP_INTERRUPT_SAFE			1		///< Set to 1 to make the memory pool interrupt safe.

// Object sizes to be handled by the memory pool
#define MP_SMALL_OBJECTS_SIZE		8		///< Maximum size for small objects
#define MP_MEDIUM_OBJECTS_SIZE		16		///< Maximum size for medium objects
#define MP_BIG_OBJECTS_SIZE			32		///< Maximum size for big objects

// Amount of objects to be handled by the memory pool
#define MP_SMALL_OBJECTS_COUNT		16		///< Amount of small objects
#define MP_MEDIUM_OBJECTS_COUNT		16		///< Amount of medium objects
#define MP_BIG_OBJECTS_COUNT		16		///< Amount of big objects

#endif // MEMORYPOOL_CONFIG_H
