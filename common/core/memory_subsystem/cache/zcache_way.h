#ifndef CACHE_WAY_H
#define CACHE_WAY_H

#include "fixed_types.h"
#include "cache_block_info.h"
#include "cache_base.h"
#include "lock.h"
#include "random.h"
#include "log.h"

#include <cstring>



// Everything related to cache sets
class CacheWay
{
   public:

      //static CacheSet* createCacheSet(String cfgname, core_id_t core_id, String replacement_policy, CacheBase::cache_t cache_type, UInt32 associativity, UInt32 blocksize, CacheSetInfo* set_info = NULL);
      //static CacheSetInfo* createCacheSetInfo(String name, String cfgname, core_id_t core_id, String replacement_policy, UInt32 associativity);
      //static CacheBase::ReplacementPolicy parsePolicyType(String policy);
      static UInt8 getNumQBSAttempts(CacheBase::ReplacementPolicy, String cfgname, core_id_t core_id);

   protected:
      CacheBlockInfo** m_cache_block_info_array;
      char* m_blocks;
      UInt32 m_associativity;
      UInt32 m_blocksize;
      UInt32 m_num_sets;
      UInt32 m_way_id;

   public:

      CacheWay(CacheBase::cache_t cache_type,
            UInt32 associativity, UInt32 blocksize, UInt32 num_sets, UInt32 index);
      virtual ~CacheWay();

      UInt32 getBlockSize() { return m_blocksize; }
      UInt32 getAssociativity() { return m_associativity; }

      void read_line(UInt32 line_index, UInt32 offset, Byte *out_buff, UInt32 bytes);
      void write_line(UInt32 line_index, UInt32 offset, Byte *in_buff, UInt32 bytes);
      CacheBlockInfo* find(IntPtr tag, UInt32 line_index );
      bool invalidate(IntPtr& tag, UInt32 line_index );
      void insert(CacheBlockInfo* cache_block_info, Byte* fill_buff, bool* eviction, CacheBlockInfo* evict_block_info, Byte* evict_buff, UInt32 way_index,CacheCntlr *cntlr = NULL);

      CacheBlockInfo* peekBlock(UInt32 way) const { return m_cache_block_info_array[way]; }

      char* getDataPtr(UInt32 line_index, UInt32 offset = 0);
      UInt32 getBlockSize(void) const { return m_blocksize; }

      //virtual UInt32 getReplacementIndex(CacheCntlr *cntlr) = 0;
      //virtual void updateReplacementIndex(UInt32) = 0;

      bool isValidReplacement(UInt32 index);
      bool isBlockValid(UInt32 index) const {return m_cache_block_info_array[index]->isValid();};

      void print_way(void);

};

#endif /* CACHE_SET_H */
