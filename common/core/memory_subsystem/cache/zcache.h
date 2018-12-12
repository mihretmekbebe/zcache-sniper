#ifndef ZCACHE_H
#define ZCACHE_H

#include "cache.h"

#ifdef ZCACHE_CLASS

#include "cache_base.h"
#include "cache_set.h"
#include "cache_block_info.h"
#include "utils.h"
#include "hash_map_set.h"
#include "cache_perf_model.h"
#include "shmem_perf_model.h"
#include "log.h"
#include "core.h"
#include "fault_injection.h"
#include "zcache_way.h"
#include "zcache_tree.h"



// Define to enable the set usage histogram
//#define ENABLE_SET_USAGE_HIST

#define NUM_ZCACHE_LEVELS 3

class Cache : public CacheBase
{
   private:
      bool m_enabled;

      // Cache counters
      UInt64 m_num_accesses;
      UInt64 m_num_hits;

      bool m_is_zcache;

      // Generic Cache Info
      cache_t m_cache_type;
      CacheWay** m_ways;

      CacheSet** m_sets;
      CacheSetInfo* m_set_info;
      
      //CacheSetInfo* m_set_info;

      FaultInjector *m_fault_injector;

      #ifdef ENABLE_SET_USAGE_HIST
      UInt64* m_set_usage_hist;
      #endif

   public:

      // constructors/destructors
      Cache(String name,
            String cfgname,
            core_id_t core_id,
            UInt32 num_sets,
            UInt32 associativity, UInt32 cache_block_size,
            String replacement_policy,
            bool is_zcache,
            cache_t cache_type,
            hash_t hash = CacheBase::HASH_MASK,
            FaultInjector *fault_injector = NULL,
            AddressHomeLookup *ahl = NULL);
      ~Cache();

      Lock& getSetLock(IntPtr addr);

      bool invalidateSingleLine(IntPtr addr);
      CacheBlockInfo* accessSingleLine(IntPtr addr,
            access_t access_type, Byte* buff, UInt32 bytes, SubsecondTime now, bool update_replacement);
      void insertSingleLine(IntPtr addr, Byte* fill_buff,
            bool* eviction, IntPtr* evict_addr,
            CacheBlockInfo* evict_block_info, Byte* evict_buff, SubsecondTime now, CacheCntlr *cntlr = NULL);
      CacheBlockInfo* peekSingleLine(IntPtr addr);

      //CacheBlockInfo* peekBlock(UInt32 set_index, UInt32 way) const { return m_sets[set_index]->peekBlock(way); }
      CacheBlockInfo* peekBlock(UInt32 way_id, UInt32 way_index) const { return m_ways[way_id]->peekBlock(way_index); }

      // Update Cache Counters
      void updateCounters(bool cache_hit);
      void updateHits(Core::mem_op_t mem_op_type, UInt64 hits);

      void enable() { m_enabled = true; }
      void disable() { m_enabled = false; }


      private:

      UInt32 hash_set_index(IntPtr tag, UInt32 index);



      protected:

      //Zcache start
      UInt32** global_counter;
      Lock m_lock;
      
      replacement_tree getReplacementIndex_z(replacement_tree*, bool*, UInt32*, UInt32*);
      void updateReplacementIndex_z(UInt32,UInt32);
      
      private:

      //replacement tree

      UInt32 num_levels;

      void CreateTree(replacement_tree *parent , UInt32 cur_level);

      replacement_tree traverse_tree(replacement_tree *parent, bool *eviction, UInt32 *way_id, UInt32 *way_index);

      void delete_tree(replacement_tree *parent);

      void print_cache(void);

      //Zcache end
};

template <class T>
UInt32 moduloHashFn(T key, UInt32 hash_fn_param, UInt32 num_buckets)
{
   return (key >> hash_fn_param) % num_buckets;
}

#endif

#endif /* CACHE_H */
