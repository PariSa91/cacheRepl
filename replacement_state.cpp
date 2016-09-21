#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sys/mman.h>
#include <map>
#include <iostream>

using namespace std;

#include "replacement_state.h"

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

/*
 ** This file implements the cache replacement state. Users can enhance the code
 ** below to develop their cache replacement ideas.
 **
 */


////////////////////////////////////////////////////////////////////////////////
// The replacement state constructor:                                         //
// Inputs: number of sets, associativity, and replacement policy to use       //
// Outputs: None                                                              //
//                                                                            //
// DO NOT CHANGE THE CONSTRUCTOR PROTOTYPE                                    //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
CACHE_REPLACEMENT_STATE::CACHE_REPLACEMENT_STATE( UINT32 _sets, UINT32 _assoc, UINT32 _pol )
{
    
    numsets    = _sets;
    assoc      = _assoc;
    replPolicy = _pol;
    
    mytimer    = 0;
    
    InitReplacementState();
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// The function prints the statistics for the cache                           //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
ostream & CACHE_REPLACEMENT_STATE::PrintStats(ostream &out)
{
    
    out<<"=========================================================="<<endl;
    out<<"=========== Replacement Policy Statistics ================"<<endl;
    out<<"=========================================================="<<endl;
    
    // CONTESTANTS:  Insert your statistics printing here
    
    return out;
    
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This function initializes the replacement policy hardware by creating      //
// storage for the replacement state on a per-line/per-cache basis.           //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

void CACHE_REPLACEMENT_STATE::InitReplacementState()
{
    // Create the state for sets, then create the state for the ways
    
    repl  = new LINE_REPLACEMENT_STATE* [ numsets ];
    
    sampler = new Entity_smplr* [ numsets_smplr];
    
    
    // ensure that we were able to create replacementvim  state
    
    assert(repl);
    //initialize predictor
    for ( int i = 0 ; i < NUM_CONF ; i++){
        predictor[i] = MAX_CONF;
    }
    
    // Create the state for the sets
    for(UINT32 setIndex=0; setIndex<numsets; setIndex++)
    {
        repl[ setIndex ]  = new LINE_REPLACEMENT_STATE[ assoc ];
        
        for(UINT32 way=0; way<assoc; way++)
        {
            // initialize stack position (for true LRU)
            repl[ setIndex ][ way ].LRUstackposition = way;
            repl[ setIndex ][ way ].isDead = false;
            //     repl[ setIndex ][ way ].age = MAX_CONF;
        }
    }
    
    for(UINT32 setIndex=0; setIndex<numsets_smplr; setIndex++)
    {
        sampler[ setIndex ]  = new Entity_smplr[ assoc ];
        
        for(UINT32 way=0; way<assoc_smplr; way++)
        {
            sampler[setIndex][way].isValid = false;
            sampler[ setIndex ][ way ].pc = -1;
            sampler[setIndex][way].tag = -1 ;
            sampler[setIndex][way].LRUstackposition = way;
            
            
        }
    }
    //    int missSamplr1 = 0 ;
    //    int missSamplr2 = 0 ;
    // ?????????
    
    //vim speeif (replPolicy != CRC_REPL_CONTESTANT) return;
    
    
    
    // Contestants:  ADD INITIALIZATION FOR YOUR HARDWARE HERE
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This function is called by the cache on every cache miss. The input        //
// argument is the set index. The return value is the physical way            //
// index for the line being replaced.                                         //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
INT32 CACHE_REPLACEMENT_STATE::GetVictimInSet( UINT32 tid, UINT32 setIndex, const LINE_STATE *vicSet, UINT32 assoc, Addr_t PC, Addr_t paddr, UINT32 accessType ) {
    // If no invalid lines, then replace based on replacement policy
    if( replPolicy == CRC_REPL_LRU )
    {
        return Get_LRU_Victim( setIndex );
    }
    else if( replPolicy == CRC_REPL_RANDOM )
    {
        return Get_Random_Victim( setIndex );
    }
    else if( replPolicy == CRC_REPL_CONTESTANT )
    {
        // Contestants:  ADD YOUR VICTIM SELECTION FUNCTION HERE
        return Get_My_Victim (setIndex , PC, paddr);
    }
    
    // We should never here here
    
    assert(0);
    
    return -1;
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This function is called by the cache after every cache hit/miss            //
// The arguments are: the set index, the physical way of the cache,           //
// the pointer to the physical line (should contestants need access           //
// to information of the line filled or hit upon), the thread id              //
// of the request, the PC of the request, the accesstype, and finall          //
// whether the line was a cachehit or not (cacheHit=true implies hit)         //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
void CACHE_REPLACEMENT_STATE::UpdateReplacementState(
                                                     UINT32 setIndex, INT32 updateWayID, const LINE_STATE *currLine,
                                                     UINT32 tid, Addr_t PC, UINT32 accessType, bool cacheHit )
{
    //fprintf (stderr, "ain't I a stinker? %lld\n", get_cycle_count ());
    //fflush (stderr);
    // What replacement policy?
    if( replPolicy == CRC_REPL_LRU )
    {
        UpdateLRU( setIndex, updateWayID );
    }
    else if( replPolicy == CRC_REPL_RANDOM )
    {
        // Random replacement requires no replacement state update
    }
    else if( replPolicy == CRC_REPL_CONTESTANT )
    {
        UpdateLRU( setIndex, updateWayID );
        UpdateMyPolicy(setIndex,  updateWayID ,  PC, cacheHit , currLine);
        // Contestants:  ADD YOUR UPDATE REPLACEMENT STATE FUNCTION HERE
        // Feel free to use any of the input parameters to make
        // updates to your replacement policy
    }
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//////// HELPER FUNCTIONS FOR REPLACEMENT UPDATE AND VICTIM SELECTION //////////
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This function finds the LRU victim in the cache set by returning the       //
// cache block at the bottom of the LRU stack. Top of LRU stack is '0'        //
// while bottom of LRU stack is 'assoc-1'                                     //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
INT32 CACHE_REPLACEMENT_STATE::Get_LRU_Victim( UINT32 setIndex )
{
    // Get pointer to replacement state of current set
    
    LINE_REPLACEMENT_STATE *replSet = repl[ setIndex ];
    INT32   lruWay   = 0;
    
    // Search for victim whose stack position is assoc-1
    
    for(UINT32 way=0; way<assoc; way++) {
        if (replSet[way].LRUstackposition == (assoc-1)) {
            lruWay = way;
            break;
        }
    }
    
    // return lru way
    
    return lruWay;
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This function finds a random victim in the cache set                       //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
INT32 CACHE_REPLACEMENT_STATE::Get_Random_Victim( UINT32 setIndex )
{
    INT32 way = (rand() % assoc);
    
    return way;
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This function implements the LRU update routine for the traditional        //
// LRU replacement policy. The arguments to the function are the physical     //
// way and set index.                                                         //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

void CACHE_REPLACEMENT_STATE::UpdateLRU( UINT32 setIndex, INT32 updateWayID )
{
    // Determine current LRU stack position
    UINT32 currLRUstackposition = repl[ setIndex ][ updateWayID ].LRUstackposition;
    
    // Update the stack position of all lines before the current line
    // Update implies incremeting their stack positions by one
    
    for(UINT32 way=0; way<assoc; way++) {
        
        
        if( repl[setIndex][way].LRUstackposition < currLRUstackposition ) {
            repl[setIndex][way].LRUstackposition++;
        }
    }
    
    // Set the LRU stack position of new line to be zero
    repl[ setIndex ][ updateWayID ].LRUstackposition = 0;
    //  // //     // cout<< " HI" ;
}

INT32 CACHE_REPLACEMENT_STATE::Get_My_Victim( UINT32 setIndex , Addr_t pc , Addr_t paddr) {
    
    if ( predictor[hash(pc )] < BYPASS_FACTOR ){
        return -1;
    }
    //  if the block is not predicted dead:
    //  search for a dead block in the set
    
    //    int minInd = -1 ;
    //    int minAge = 100;
    //    for ( int i = 0 ; i < assoc ; i++){
    //        if ( repl[setIndex][i].age < minAge && repl[setIndex][i].isDead){
    //            // reset the isDead variable because we are bringing a new block in
    //            minInd = i ;
    //            minAge = repl[setIndex][i].age ;
    //        }
    //
    //    }
    //    if ( minInd > 0 ){
    //            // cout<< " Here I found a dead: " << minInd<< endl ;
    //      return minInd;
    //    }
    for ( int i = 0 ; i < assoc ; i++){
        if ( repl[setIndex][i].isDead == true){
            //  repl[setIndex][i].isDead = false; // reset the isDead variable because we are bringing a new block in
            //     // cout<< " Here I found a dead: " << i << endl ;
            return i; // relace the dead block
        }
        
    }
    
    
    int l =Get_LRU_Victim(setIndex);
    //     // cout<< " No  DEAD DETECTED we choose this by lru: " << l<< endl ;
    
    //there is no dead block in the set
    // replace the lru
    return l;
    
    
}
unsigned long long int CACHE_REPLACEMENT_STATE::hash(UINT32 pc)
{
    return (pc  % NUM_CONF );
    
}
void CACHE_REPLACEMENT_STATE::update_LRU_sampler(UINT32 setIndex_smplr, INT32 updateWayID_smplr){
    
    //update the lru position in sampler
    UINT32 currLRUstackposition_smplr = sampler[setIndex_smplr][updateWayID_smplr].LRUstackposition;
    for(UINT32 way=0; way<assoc_smplr; way++) {
        
        if( sampler[setIndex_smplr][way].LRUstackposition < currLRUstackposition_smplr ) {
            sampler[setIndex_smplr][way].LRUstackposition++;
        }
    }
    
    // Set the LRU stack position of new line to be zero
    sampler[setIndex_smplr][ currLRUstackposition_smplr ].LRUstackposition = 0;
    
    //end updating the LRU in sampler
}
int CACHE_REPLACEMENT_STATE::get_LRU_sampler(UINT32 setIndex_smplr){
    //find the lru in the sampler to replace
    int lru = 0;
    
    for ( int j = 0 ; j < assoc_smplr ; j++)
    {
        if ( sampler[setIndex_smplr][j].isValid== false)
        {
            return j;
        }
    }
    
    for ( int j = 0 ; j < assoc_smplr ; j++)
    {
        if ( sampler[setIndex_smplr][j].LRUstackposition == (assoc_smplr-1))
        {
            lru = j;
        }
        break;
    }
    return lru;
}


//bool CACHE_REPLACEMENT_STATE::inSampler1(UINT32 setIndex){
//    if ( setIndex < (0.05) * numsets_smplr )
//        return true;
//    else
//        return false;
//}
//bool CACHE_REPLACEMENT_STATE::inSampler2(UINT32 setIndex){
//    if ( setIndex > ((0.05)*numsets_smplr ) &&  ((0.10) * numsets_smplr)> setIndex)
//        return true;
//    else
//        return false;
//}

void CACHE_REPLACEMENT_STATE::UpdateMyPolicy( UINT32 setIndex, INT32 updateWayID ,  Addr_t pc , bool cacheHit , const LINE_STATE *currLine) {
    
    int updateWayID_smplr = -1 ;
    int index_prdctr = -1;
    int setIndex_smplr = setIndex / (numsets/numsets_smplr);
    bool found = false ;
    
    
    //update the sampler if the set is in the sampler
    
    if (setIndex % (numsets/numsets_smplr) == 0 ) // the set is in sampler
    {
        //     // cout << " the set is in sampler " << endl;
        for(UINT32 way=0; way<assoc_smplr; way++)
        {
            if ((sampler[setIndex_smplr][way].tag )== (currLine->tag) )//if sampler has the blocks information
            {
                // cout << " tag is in sampler " << endl ;
                
                updateWayID_smplr = way;
                index_prdctr = hash((sampler[setIndex_smplr][updateWayID_smplr].pc) ) ;
                if ( predictor[index_prdctr] < MAX_CONF )// hit
                {
                    predictor[index_prdctr]++;
                }
                sampler[setIndex_smplr][updateWayID_smplr].pc = pc ;
                found = true;
                
            }//  end if check tag
            
        }//end for
        
        if ( !found)
            // set doesn't contain the block's information, miss in sampler
        {
            // cout << " tag is NOT in sampler" << endl ;
            //    // cout<< setIndex << "|" << numsets << " |"<< nums  //  // coutets_smplr<< endl;
            
            
            
            updateWayID_smplr = get_LRU_sampler(setIndex_smplr);
            
            index_prdctr = hash(pc) ;
            if ( predictor[index_prdctr] > MIN_CONF ) //miss
            {
                predictor[index_prdctr] --; // the block that is evicted is getting closer to death
            }
            sampler[setIndex_smplr][updateWayID_smplr].pc= pc ;
            sampler[setIndex_smplr][updateWayID_smplr].tag = currLine->tag;
            sampler[setIndex_smplr][updateWayID_smplr].isValid = true;
            
        }// end not founds
        
        //update predictor
        
        
        //    repl[setIndex_smplr][updateWayID_smplr].age = predictor[index_prdctr];
        
        update_LRU_sampler(setIndex_smplr, updateWayID_smplr);
        //  update_MRU_sampler(setIndex_smplr, updateWayID_smplr);
        
        
        
    }//end if in sampler
    
    
    if ( predictor[hash(pc)] > YOUTH_FACTOR)
    {
        repl[setIndex][updateWayID].isDead = false;
    } else
    {
        repl[setIndex][updateWayID].isDead = true;
    }
    
    
}

CACHE_REPLACEMENT_STATE::~CACHE_REPLACEMENT_STATE (void) {
}
