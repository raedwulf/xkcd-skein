#ifndef _AHS_API_H_
#define _AHS_API_H_

/***********************************************************************
**
** Interface declarations of the AHS API using the Skein hash function.
**
** Source code author: Doug Whiting, 2008.
**
** This algorithm and source code is released to the public domain.
** 
************************************************************************/

#include "skein.h"

typedef enum
    {
    SUCCESS     = SKEIN_SUCCESS,
    FAIL        = SKEIN_FAIL,
    BAD_HASHLEN = SKEIN_BAD_HASHLEN
    }
    HashReturn;

typedef size_t   DataLength;                /* bit count  type */
typedef u08b_t   BitSequence;               /* bit stream type */

typedef struct
    {
    uint_t  statebits;                      /* 256, 512, or 1024 */
    union
        {
        Skein_Ctxt_Hdr_t h;                 /* common header "overlay" */
        Skein1024_Ctxt_t ctx1024;
        } u;
    }
    hashState;

/* "incremental" hashing API */
HashReturn Init  (hashState *state, int hashbitlen);
HashReturn Update(hashState *state, const BitSequence *data, DataLength databitlen);
HashReturn Final (hashState *state,       BitSequence *hashval);

/* "all-in-one" call */
HashReturn Hash  (int hashbitlen,   const BitSequence *data, 
                  DataLength databitlen,  BitSequence *hashval);

#endif  /* ifdef _AHS_API_H_ */
