/***********************************************************************
**
** Implementation of the AHS API using the Skein hash function.
**
** Source code author: Doug Whiting, 2008.
**
** This algorithm and source code is released to the public domain.
** 
************************************************************************/

#include <string.h>     /* get the memcpy/memset functions */
#include "skein.h"      /* get the Skein API definitions   */
#include "SHA3api_ref.h"/* get the  AHS  API definitions   */

/******************************************************************/
/*     AHS API code                                               */
/******************************************************************/

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* select the context size and init the context */
HashReturn Init(hashState *state, int hashbitlen)
    {
      state->statebits = 64*SKEIN1024_STATE_WORDS;
      return Skein1024_Init(&state->u.ctx1024,(size_t) hashbitlen);
    }

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* process data to be hashed */
HashReturn Update(hashState *state, const BitSequence *data, DataLength databitlen)
    {
    /* only the final Update() call is allowed do partial bytes, else assert an error */
    Skein_Assert((state->u.h.T[1] & SKEIN_T1_FLAG_BIT_PAD) == 0 || databitlen == 0, FAIL);

    Skein_Assert(state->statebits % 256 == 0 && (state->statebits-256) < 1024,FAIL);
    if ((databitlen & 7) == 0)  /* partial bytes? */
        {
            return Skein1024_Update(&state->u.ctx1024,data,databitlen >> 3);
        }
    else
        {   /* handle partial final byte */
        size_t bCnt = (databitlen >> 3) + 1;                  /* number of bytes to handle (nonzero here!) */
        u08b_t b,mask;

        mask = (u08b_t) (1u << (7 - (databitlen & 7)));       /* partial byte bit mask */
        b    = (u08b_t) ((data[bCnt-1] & (0-mask)) | mask);   /* apply bit padding on final byte */

        Skein1024_Update(&state->u.ctx1024,data,bCnt-1); /* process all but the final byte    */
        Skein1024_Update(&state->u.ctx1024,&b  ,  1   ); /* process the (masked) partial byte */
        Skein_Set_Bit_Pad_Flag(state->u.h);                    /* set tweak flag for the final call */
        
        return SUCCESS;
        }
    }

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* finalize hash computation and output the result (hashbitlen bits) */
HashReturn Final(hashState *state, BitSequence *hashval)
    {
    Skein_Assert(state->statebits % 256 == 0 && (state->statebits-256) < 1024,FAIL);
    return Skein1024_Final(&state->u.ctx1024,hashval);
    }

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* all-in-one hash function */
HashReturn Hash(int hashbitlen, const BitSequence *data, /* all-in-one call */
                DataLength databitlen,BitSequence *hashval)
    {
    hashState  state;
    HashReturn r = Init(&state,hashbitlen);
    if (r == SUCCESS)
        { /* these calls do not fail when called properly */
        r = Update(&state,data,databitlen);
        Final(&state,hashval);
        }
    return r;
    }
