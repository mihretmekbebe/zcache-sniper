#include "simulator.h"
#include "zcache.h"
#include "log.h"
#include <limits.h>
#include <cassert>

// Cache class
// constructors/destructors

#ifdef ZCACHE_CLASS

Cache::Cache(
   String name,
   String cfgname,
   core_id_t core_id,
   UInt32 num_sets,
   UInt32 associativity,
   UInt32 cache_block_size,
   String replacement_policy,
   bool is_zcache,
   cache_t cache_type,
   hash_t hash,
   FaultInjector *fault_injector,
   AddressHomeLookup *ahl)
:
   CacheBase(name, num_sets, associativity, cache_block_size, hash, ahl),
   m_enabled(false),
   m_num_accesses(0),
   m_num_hits(0),
   m_cache_type(cache_type),
   m_fault_injector(fault_injector)
{
   //m_set_info = CacheSet::createCacheSetInfo(name, cfgname, core_id, replacement_policy, m_associativity);
   
   /*
   m_sets = new CacheSet*[m_num_sets];
   for (UInt32 i = 0; i < m_num_sets; i++)
   {
      m_sets[i] = CacheSet::createCacheSet(cfgname, core_id, replacement_policy, m_cache_type, m_associativity, m_blocksize, m_set_info);
   }
   */
   //Replace set by ways

   m_is_zcache = is_zcache;

   //printf("Core id: %d Creating zcache elements: %d\n", core_id, is_zcache);

   if(m_is_zcache){

      printf("Zcache is being created\n\n");
      m_ways = new CacheWay*[m_associativity];
      global_counter = new UInt32*[m_associativity];

      for (UInt32 index = 0; index < m_associativity; index++)
      {
         //m_ways[i] = CacheSet::createCacheSet(cfgname, core_id, replacement_policy, m_cache_type, m_associativity, m_blocksize, m_set_info);

         m_ways[index] = new CacheWay(m_cache_type,m_associativity,m_blocksize,m_num_sets, index);

         global_counter[index] = new UInt32[m_num_sets];
         for(UInt32 j=0;j<m_num_sets;j++)
            global_counter[index][j] = 0;

      }

      num_levels = NUM_ZCACHE_LEVELS;

   }

   else{
      m_set_info = CacheSet::createCacheSetInfo(name, cfgname, core_id, replacement_policy, m_associativity);
      m_sets = new CacheSet*[m_num_sets];
      for (UInt32 i = 0; i < m_num_sets; i++)
      {
         m_sets[i] = CacheSet::createCacheSet(cfgname, core_id, replacement_policy, m_cache_type, m_associativity, m_blocksize, m_set_info);
      }

      #ifdef ENABLE_SET_USAGE_HIST
      m_set_usage_hist = new UInt64[m_num_sets];
      for (UInt32 i = 0; i < m_num_sets; i++)
         m_set_usage_hist[i] = 0;
      #endif


   }
   
   //parent = new replacement_tree(m_associativity,num_levels);




/* 
   #ifdef ENABLE_SET_USAGE_HIST
   m_set_usage_hist = new UInt64[m_num_sets];
   for (UInt32 i = 0; i < m_num_sets; i++)
      m_set_usage_hist[i] = 0;
   #endif
*/

   //Removed as no sets exist for zcache
}

Cache::~Cache()
{
   /*
   #ifdef ENABLE_SET_USAGE_HIST
   printf("ZCache %s set usage:", m_name.c_str());
   for (SInt32 i = 0; i < (SInt32) m_num_sets; i++)
      printf(" %" PRId64, m_set_usage_hist[i]);
   printf("\n");
   delete [] m_set_usage_hist;
   #endif
   */ 

   //Removed as no set exist for zcache

   //if (m_set_info)
      //delete m_set_info;

   /*
   for (SInt32 i = 0; i < (SInt32) m_num_sets; i++)
      delete m_sets[i];
   delete [] m_sets;
   */

   if(m_is_zcache){

      printf("Zcache is being destructed\n");
      for (UInt32 i = 0; i <  m_associativity; i++){
      delete m_ways[i];
      delete [] global_counter[i];
      }
      delete [] m_ways;
      delete [] global_counter;
   }

   else{

      #ifdef ENABLE_SET_USAGE_HIST
      printf("Cache %s set usage:", m_name.c_str());
      for (SInt32 i = 0; i < (SInt32) m_num_sets; i++)
         printf(" %" PRId64, m_set_usage_hist[i]);
      printf("\n");
      delete [] m_set_usage_hist;
      #endif

      if (m_set_info)
         delete m_set_info;

      for (SInt32 i = 0; i < (SInt32) m_num_sets; i++)
         delete m_sets[i];
      delete [] m_sets;

   }
   

}


//To be learnt what this is for
Lock&
Cache::getSetLock(IntPtr addr)
{
   if(m_is_zcache){
      return m_lock;
   }

   else
   {
      IntPtr tag;
      UInt32 set_index;

      splitAddress(addr, tag, set_index);
      assert(set_index < m_num_sets);

      return m_sets[set_index]->getLock();
   }
   
}


//Done for Zcache
bool
Cache::invalidateSingleLine(IntPtr addr)
{

   if(m_is_zcache){
      //printf("Invalidate line enter\n");
      IntPtr tag = addr >> floorLog2(m_blocksize);    //tag for given address
      UInt32 set_index;

      for (int index = m_associativity-1; index >= 0; index--)
      {
         set_index = hash_set_index(tag,index);       //For the given tag and way number, map set_index
         if(m_ways[index]->find(tag,set_index) != NULL)  //If available in the particular index, invalidate
         {
            //printf("Invalidate line end\n");
            return m_ways[index]->invalidate(tag,set_index);
         }
      }

      //printf("Invalidate line end\n");
      return false;

   }
   
   else{

      IntPtr tag;
      UInt32 set_index;

      splitAddress(addr, tag, set_index);
      assert(set_index < m_num_sets);

      return m_sets[set_index]->invalidate(tag);

   }

}

//addr:        address where read/write is to be performed
//access_type: LOAD/STORE
//buff:        
//bytes:       bytes
//now:         time used for fault injection
//update_replacement: Whther or not replacement policy must be applied. i.e considered as an access.         



//Done read and write, replacement logic to be written
CacheBlockInfo*
Cache::accessSingleLine(IntPtr addr, access_t access_type,
      Byte* buff, UInt32 bytes, SubsecondTime now, bool update_replacement)
{
   //assert((buff == NULL) == (bytes == 0));


   /*
   ZCACHE
   */
   if(m_is_zcache){

      //printf("Access line enter\n");
      IntPtr tag;
      UInt32 way_index = UINT_MAX;
      UInt32 line_index = UINT_MAX;
      UInt32 block_offset = addr & (m_blocksize-1);

      //splitAddress(addr, tag, set_index, block_offset);
      //IntPtr tag = addr >> floorLog2(m_blocksize);


      tag = addr >> floorLog2(m_blocksize);

      for (int index = m_associativity-1; index >= 0; index--)
      {
         UInt32 set_index = hash_set_index(tag, index);

         if(m_ways[index]->find(tag,set_index) != NULL)
         {
            way_index = (UInt32)index;
            //return m_ways[index]->find(tag,set_index);
         }
      }

      if (way_index == UINT_MAX)
      {
         //printf("Access line end\n");
         return NULL;
      }

      CacheWay* way = m_ways[way_index];

      line_index = hash_set_index(tag, way_index);

      CacheBlockInfo* cache_block_info = m_ways[way_index]->find(tag, line_index);


      if (access_type == LOAD)
      {
         // NOTE: assumes error occurs in memory. If we want to model bus errors, insert the error into buff instead
         //if (m_fault_injector)
            //m_fault_injector->preRead(addr, set_index * m_associativity + line_index, bytes, (Byte*)m_sets[set_index]->getDataPtr(line_index, block_offset), now);

         //set->read_line(line_index, block_offset, buff, bytes, update_replacement); //where is set defined ?

         way->read_line(line_index, block_offset, buff, bytes);

         if (update_replacement)
            updateReplacementIndex_z(way_index,line_index);



         //Write logic for updating replacement

         //memcpy(&m_blocks[line_index * m_blocksize + offset], (void*) in_buff, bytes);
         //copy bytes number of bytes from buff to 
      }
      else
      {
         way->write_line(line_index, block_offset, buff, bytes);

         if (update_replacement)
            updateReplacementIndex_z(way_index,line_index);


         //Write logic for updating replacement

         // NOTE: assumes error occurs in memory. If we want to model bus errors, insert the error into buff instead
         //if (m_fault_injector)
            //m_fault_injector->postWrite(addr, set_index * m_associativity + line_index, bytes, (Byte*)m_sets[set_index]->getDataPtr(line_index, block_offset), now);
      }

      //printf("Access line end\n");

      return cache_block_info;

   }

   /*
   NORMAL CACHE
   */

   else
   {
      IntPtr tag;
      UInt32 set_index;
      UInt32 line_index = -1;
      UInt32 block_offset;

      splitAddress(addr, tag, set_index, block_offset);

      CacheSet* set = m_sets[set_index];
      CacheBlockInfo* cache_block_info = set->find(tag, &line_index);

      if (cache_block_info == NULL)
         return NULL;

      if (access_type == LOAD)
      {
         // NOTE: assumes error occurs in memory. If we want to model bus errors, insert the error into buff instead
         if (m_fault_injector)
            m_fault_injector->preRead(addr, set_index * m_associativity + line_index, bytes, (Byte*)m_sets[set_index]->getDataPtr(line_index, block_offset), now);

         set->read_line(line_index, block_offset, buff, bytes, update_replacement);
      }
      else
      {
         set->write_line(line_index, block_offset, buff, bytes, update_replacement);

         // NOTE: assumes error occurs in memory. If we want to model bus errors, insert the error into buff instead
         if (m_fault_injector)
            m_fault_injector->postWrite(addr, set_index * m_associativity + line_index, bytes, (Byte*)m_sets[set_index]->getDataPtr(line_index, block_offset), now);
      }

      return cache_block_info;
   }

}

//TBD
void
Cache::insertSingleLine(IntPtr addr, Byte* fill_buff,
      bool* eviction, IntPtr* evict_addr,
      CacheBlockInfo* evict_block_info, Byte* evict_buff,
      SubsecondTime now, CacheCntlr *cntlr)
{
   //IntPtr tag;
   //UInt32 set_index;
   //splitAddress(addr, tag, set_index);


   if(m_is_zcache){

         //printf("Insert single line enter\n");
         IntPtr tag;

         tag = addr >> floorLog2(m_blocksize);

         CacheBlockInfo* cache_block_info = CacheBlockInfo::create(m_cache_type);
         cache_block_info->setTag(tag);


         //Logic for insertion and replacement

         
         replacement_tree *parent = new replacement_tree(m_associativity,num_levels);

         parent->level = UINT_MAX;
         parent->block.clone(cache_block_info);
         parent->parent_node = NULL;


         UInt32 way_id,way_index;
         bool temp_eviction = true;

         

         replacement_tree block = getReplacementIndex_z(parent, &temp_eviction, &way_id, &way_index);

         

         Byte evict_temp_buff[m_blocksize];
         Byte *fill_temp_buff = fill_buff;
         


         if(temp_eviction == false)
         {
            m_ways[way_id]->insert(cache_block_info,fill_temp_buff,eviction,evict_block_info,evict_temp_buff,way_index,cntlr);
         }

         else
         {

            for(UInt32 i=0;i <= block.level;i++){

            UInt32 way = block.state[i].way_id;
            UInt32 b_way_index = block.state[i].way_index;

            if(i==block.level)
               m_ways[way]->insert(cache_block_info,fill_temp_buff,&temp_eviction,evict_block_info,evict_temp_buff,b_way_index,cntlr);
            else
               m_ways[way]->insert(cache_block_info,fill_temp_buff,eviction,evict_block_info,evict_buff,b_way_index,cntlr);


            cache_block_info->clone(evict_block_info);
            fill_temp_buff = evict_temp_buff;

            //CacheWay::write_line(UInt32 line_index, UInt32 offset, Byte *in_buff, UInt32 bytes, bool update_replacement);
            }
         }

         

         

         //delete parent;

         
         


         //Naive replace start

       /*           
         UInt32 way = (tag+153)%m_associativity;
         UInt32 b_way_index = hash_set_index(tag, way);

         m_ways[way]->insert(cache_block_info,fill_buff,eviction,evict_block_info,evict_buff,b_way_index,cntlr);
*/
         //Naive replace end


         *evict_addr = tagToAddress(evict_block_info->getTag());

         delete cache_block_info;

         //print_cache();

         //printf("Insert single line end\n");

   }

   else
   {


         IntPtr tag;
         UInt32 set_index;
         splitAddress(addr, tag, set_index);

         CacheBlockInfo* cache_block_info = CacheBlockInfo::create(m_cache_type);
         cache_block_info->setTag(tag);

         m_sets[set_index]->insert(cache_block_info, fill_buff,
               eviction, evict_block_info, evict_buff, cntlr);
         *evict_addr = tagToAddress(evict_block_info->getTag());

         if (m_fault_injector) {
            // NOTE: no callback is generated for read of evicted data
            UInt32 line_index = -1;
            __attribute__((unused)) CacheBlockInfo* res = m_sets[set_index]->find(tag, &line_index);
            LOG_ASSERT_ERROR(res != NULL, "Inserted line no longer there?");

            m_fault_injector->postWrite(addr, set_index * m_associativity + line_index, m_sets[set_index]->getBlockSize(), (Byte*)m_sets[set_index]->getDataPtr(line_index, 0), now);
         }

         #ifdef ENABLE_SET_USAGE_HIST
         ++m_set_usage_hist[set_index];
         #endif

         delete cache_block_info;
   }


}




// Single line cache access at addr
//Done for zcache
CacheBlockInfo*
Cache::peekSingleLine(IntPtr addr)
{

   if(m_is_zcache)
   {

      //printf("Peekline enter\n");
      IntPtr tag = addr >> floorLog2(m_blocksize);

      for (int index = m_associativity-1; index >= 0; index--)
      {
         UInt32 set_index = hash_set_index(tag, index);
         if(m_ways[index]->find(tag,set_index) != NULL)
         {
            //printf("Peekline end\n");
            return m_ways[index]->find(tag,set_index);
         }
      }


      //printf("Peekline end null\n");
      return NULL;
   }

   else
   {
      IntPtr tag;
      UInt32 set_index;
      splitAddress(addr, tag, set_index);

      return m_sets[set_index]->find(tag);

   }
   

}

//Hash function for Zcache
//Change to H3 hash function ?

UInt32
Cache::hash_set_index(IntPtr tag, UInt32 index)
{

   //printf("Hash set index is fine\n");

   return (tag + 153153*index) % m_num_sets;

/*
   int shift_amount = 1;
   int i =32;

   while((i>0) && ((windowSize%i)!=0) ){
      shift_amount = shift_amount*2;
      i = i/2;
   }

   shift_amount = shift_amount%32;
*/

}


//Done for Zcache
void
Cache::updateCounters(bool cache_hit)
{

   if (m_enabled)
   {
      m_num_accesses ++;
      if (cache_hit)
         m_num_hits ++;
   }
}

//Done for Zcache
void
Cache::updateHits(Core::mem_op_t mem_op_type, UInt64 hits)
{
   if (m_enabled)
   {
      m_num_accesses += hits;
      m_num_hits += hits;
   }
}



//ZCache tree


void
Cache::CreateTree(replacement_tree *parent , UInt32 cur_level = 0){


   //printf("Creating replacement tree level of ass %d at level %d\n\n", m_associativity, cur_level);

   if(cur_level == num_levels){
      //printf("Num levels reached, exiting\n");
      return;
   }

   for(UInt32 way=0; way < m_associativity; way++){

      printf("CT: wayid: %d parent->way_id : %d\n", way, parent->way_id);

      if ((way != parent->way_id) || (cur_level == 0) ){

            UInt32 child_way_index = hash_set_index(parent->block.getTag(), way );
            //printf("Child Way index: %d \n",child_way_index );

            //m_ways[way]->print_way();

            if(m_ways[way]->isBlockValid(child_way_index)){

            printf("Inserting Level: %d way: %d index: %d tag:0x%lX\n" , cur_level, way,child_way_index, m_ways[way]->peekBlock(child_way_index)->getTag() );
            parent->children[way] = new replacement_tree(m_associativity,num_levels);
            parent->children[way]->way_id = way;
            parent->children[way]->way_index = child_way_index;

            parent->children[way]->level = cur_level;
            parent->children[way]->lru_counter = global_counter[way][child_way_index];

            parent->children[way]->block.clone(peekBlock(way,child_way_index));
            //parent->children[way]->state[cur_level] = {way,child_way_index};
            parent->children[way]->parent_node = parent;


            
            replacement_tree *node = parent->children[way];

            for(int i=cur_level;i>=0;i--)
            {
               parent->children[way]->state[i] = {node->way_id,node->way_index};
               node = node->parent_node;
            }
            


            CreateTree(parent->children[way] , cur_level + 1);
         }

      }

   }

   //printf("Created replacement tree level %d\n\n", cur_level);
}


replacement_tree
Cache::traverse_tree(replacement_tree *parent, bool *eviction, UInt32 *way_id, UInt32 *way_index)
{


   //printf("Traversing replacement tree\n");

   //printf("Traversing replacement tree level\n\n");


   replacement_tree* cur_block = parent;

   assert(cur_block != NULL);

   *eviction = true;

   replacement_tree replacement_candidate(m_associativity,num_levels);// //*parent
   replacement_candidate.lru_counter = 0;


   

   for(UInt32 way=0; way < m_associativity; way++)
   {
      if(cur_block->children[way] != NULL){

         UInt32 child_way_index = parent->children[way]->way_index;

         bool is_valid = m_ways[way]->isBlockValid(child_way_index);
         assert(is_valid == 1);

         printf("Traversing Level: %d way: %d index: %d tag:0x%lX\n\n" , cur_block->children[way]->level, way , child_way_index, m_ways[way]->peekBlock(child_way_index)->getTag());

         replacement_tree temp_replacement = traverse_tree(cur_block->children[way],eviction, way_id, way_index);


         //replacement policy

         if(*eviction == false)
            return replacement_candidate;
         
         if(temp_replacement.lru_counter > replacement_candidate.lru_counter)
            replacement_candidate = temp_replacement;
      
         
      }

      else if(cur_block->level != num_levels - 1)
      {
         *eviction = false;
         *way_id = way;
         *way_index = hash_set_index(parent->block.getTag(), way );

         printf("No replacement.. Insert in level %d way: %d index: %d\n",cur_block->level,*way_id,*way_index);
         return replacement_candidate;

      }

   }

   return replacement_candidate;
   //printf("Completed traversing replacement tree\n");

}



void
Cache::delete_tree(replacement_tree* parent)
{

   //printf("Delete tree enter\n");
   replacement_tree* cur_block = parent;

   for(UInt32 way=0; way < m_associativity; way++)
   {
      if(cur_block->children[way] != NULL){
         delete_tree(cur_block->children[way]);
         cur_block->children[way] = NULL;
      }
   }

   delete cur_block;

   //printf("Delete tree end\n");

}





//Replacement logic



replacement_tree 
Cache::getReplacementIndex_z(replacement_tree* parent, bool *eviction, UInt32 *way_id, UInt32 *way_index)
{
   //Definition written in appropriate program for replacement

   //printf("Get replacement index enter\n");

   CreateTree(parent);

   replacement_tree block = traverse_tree(parent,eviction, way_id, way_index);

   delete_tree(parent);

   //printf("Get replacement index exit\n");

   return block;
}


void 
Cache::updateReplacementIndex_z(UInt32 accessed_way, UInt32 accessed_index)
{
   //Definition written in appropriate program for replacement
   global_counter[accessed_way][accessed_index]++;
}




void
Cache::print_cache(){

   printf("Start print\n...\n...\n");
   for(int i= 0; i < (int)m_associativity; i++){
      printf("Way:%d\n",i);
      m_ways[i]->print_way();
   }
   printf("End print\n\n\n");
}

#endif