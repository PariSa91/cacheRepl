#ifndef REPL_STATE_H
#define REPL_STATE_H

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This file is distributed as part of the Cache Replacement Championship     //
// workshop held in conjunction with ISCA'2010.                               //
//                                                                            //
//                                                                            //
// Everyone is granted permission to copy, modify, and/or re-distribute       //
// this software.                                                             //
//                                                                            //
// Please contact Aamer Jaleel <ajaleel@gmail.com> should you have any        //
// questions                                                                  //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#include <cstdlib>
#include <cassert>
#include "utils.h"
#include "crc_cache_defs.h"
#include <iostream>

using namespace std;
#define NUM_CONF  1048576
#define numsets_smplr 1024
#define assoc_smplr 12
#define MAX_CONF   127
#define MIN_CONF   0
#define YOUTH_FACTOR   20
#define BYPASS_FACTOR   20

#define DUELING_FACT 1000


// Replacement Policies Supported
typedef enum
{
    CRC_REPL_LRU        = 0,
    CRC_REPL_RANDOM     = 1,
    CRC_REPL_CONTESTANT = 2
} ReplacemntPolicy;

// Replacement State Per Cache Line
typedef struct
{
    UINT32  LRUstackposition;
    bool isDead;
    // CONTESTANTS: Add extra state per cache line here
   // int age;
    
} LINE_REPLACEMENT_STATE;

typedef struct
{
    
    unsigned long long int pc, tag ;
    bool isValid;
    bool prediction;
    UINT32  LRUstackposition;
    
}Entity_smplr;





// The implementation for the cache replacement policy
class CACHE_REPLACEMENT_STATE
{
public:
    LINE_REPLACEMENT_STATE   **repl;
    
    int predictor[NUM_CONF] ;
    
    Entity_smplr **sampler;
private:
    
    UINT32 numsets;
    UINT32 assoc;
    UINT32 replPolicy;
    
    COUNTER mytimer;  // tracks # of references to the cache
    
    // CONTESTANTS:  Add extra state for cache here
    int missSamplr1;
    int missSamplr2;
public:
    ostream & PrintStats(ostream &out);
    
    // The constructor CAN NOT be changed
    CACHE_REPLACEMENT_STATE( UINT32 _sets, UINT32 _assoc, UINT32 _pol );
    
    INT32 GetVictimInSet( UINT32 tid, UINT32 setIndex, const LINE_STATE *vicSet, UINT32 assoc, Addr_t PC, Addr_t paddr, UINT32 accessType );
    
    void   UpdateReplacementState( UINT32 setIndex, INT32 updateWayID );
    
    void   SetReplacementPolicy( UINT32 _pol ) { replPolicy = _pol; }
    void   IncrementTimer() { mytimer++; }
    
    void   UpdateReplacementState( UINT32 setIndex, INT32 updateWayID, const LINE_STATE *currLine,
                                  UINT32 tid, Addr_t PC, UINT32 accessType, bool cacheHit );
    
    ~CACHE_REPLACEMENT_STATE(void);
    
private:
    
   
    void   InitReplacementState();
    INT32  Get_Random_Victim( UINT32 setIndex );
    
    INT32  Get_LRU_Victim( UINT32 setIndex );
    INT32  Get_My_Victim( UINT32 setIndex  , Addr_t PC , Addr_t paddr);
    void   UpdateLRU( UINT32 setIndex, INT32 updateWayID );
    void   UpdateMyPolicy( UINT32 setIndex, INT32 updateWayID ,  Addr_t PC , bool cacheHit , const LINE_STATE *currLine);
    
    void update_LRU_sampler(UINT32 setIndex_smplr, INT32 updateWayID_smplr);
    int get_LRU_sampler(UINT32 setIndex_smplr);
    bool inSampler1(UINT32 setIndex);
    bool inSampler2(UINT32 setIndex);
    unsigned long long int hash(UINT32 pc);
    
};

#endif
