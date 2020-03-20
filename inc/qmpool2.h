/*
 * qmpool_Original.h
 *
 *  Created on: Mar 17, 2020
 *      Author: pablo
 */

#ifndef EXAMPLES_C_APP2_INC_QMPOOL2_H_
#define EXAMPLES_C_APP2_INC_QMPOOL2_H_

#include "sapi.h"

 //structure representing a free block in the Native QF Memory Pool */
typedef struct QFreeBlock {
    struct QFreeBlock * volatile next;
} QFreeBlock;

/*! access element at index @p i_ from the base pointer @p base_ */
#define QF_PTR_AT_(base_, i_)   ((base_)[(i_)])

// Memory pool element to allocate correctly aligned storage for QMPool class.
#define QF_MPOOL_EL(evType_) struct { void *sto_[((sizeof(evType_) - 1U)/sizeof(void*)) + 1U]; }

//The data type to store the block-size based on the macro #QF_MPOOL_SIZ_SIZE.
//The dynamic range of this data type determines the maximum size of blocks that can be managed by the native QF event pool.
#ifndef QF_MPOOL_SIZ_SIZE
    #define QF_MPOOL_SIZ_SIZE 2 // macro to override the default ::QMPoolSize size. Valid values 1, 2, or 4; default 2
#endif
#if (QF_MPOOL_SIZ_SIZE   == 1)
    typedef uint8_t QMPoolSize;
#elif (QF_MPOOL_SIZ_SIZE == 2)
    typedef uint16_t QMPoolSize;
#elif (QF_MPOOL_SIZ_SIZE == 4)
    typedef uint32_t QMPoolSize;
#else
    #error "QF_MPOOL_SIZ_SIZE defined incorrectly, expected 1, 2, or 4"
#endif

//The data type to store the block-counter based on the macro #QF_MPOOL_CTR_SIZE.
// The dynamic range of this data type determines the maximum number of blocks that can be stored in the pool.
#ifndef QF_MPOOL_CTR_SIZE
    #define QF_MPOOL_CTR_SIZE 2   //macro to override the default ::QMPoolCtr size. Valid values 1, 2, or 4; default 2
#endif
#if (QF_MPOOL_CTR_SIZE   == 1)
    typedef uint8_t QMPoolCtr;
#elif (QF_MPOOL_CTR_SIZE == 2)
    typedef uint16_t QMPoolCtr;
#elif (QF_MPOOL_CTR_SIZE == 4)
    typedef uint32_t QMPoolCtr;
#else
    #error "QF_MPOOL_CTR_SIZE defined incorrectly, expected 1, 2, or 4"
#endif

typedef struct {
    void * volatile     free_head; // The head of the linked list of free blocks */
    void *              start;     // the original start this pool */
    void *              end;       // the last memory block managed by this memory pool */
    QMPoolSize          blockSize; // maximum block size (in bytes) */
    QMPoolCtr           nTot;      // total number of blocks */
    QMPoolCtr volatile  nFree;     // number of free blocks remaining */
    QMPoolCtr           nMin;      // watewrmark minima
} QMPool;

void           QMPool_init         ( QMPool * const me ,void * const poolSto       ,uint_fast32_t poolSize ,uint_fast16_t blockSize );
void *         QMPool_get          ( QMPool * const me ,uint_fast16_t const margin                                                  );
void           QMPool_put          ( QMPool * const me ,void * b                                                                     );
uint_fast16_t  QMPool_getMin       ( QMPool * const me                                                                              );
char *         Print_Qmpool_Struct ( QMPool * Q        ,char * S                                                                     );

 //#endif /* QMPOOL_H */


#endif /* EXAMPLES_C_APP2_INC_QMPOOL2_H_ */
