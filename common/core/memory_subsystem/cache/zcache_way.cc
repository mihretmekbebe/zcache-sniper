#include "zcache.h"
#include "zcache_way.h"
#include "cache_base.h"
#include "log.h"
#include "simulator.h"
#include "config.h"
#include "config.hpp"

CacheWay::CacheWay(CacheBase::cache_t cache_type,
      UInt32 associativity, UInt32 blocksize, UInt32 num_sets, UInt32 index):
      m_associativity(associativity), m_blocksize(blocksize), m_num_sets(num_sets), m_way_id(index)
{

    //printf("Constructor for way entered\n");

   m_cache_block_info_array = new CacheBlockInfo*[m_num_sets];
   for (UInt32 i = 0; i < m_num_sets; i++)
   {
      m_cache_block_info_array[i] = CacheBlockInfo::create(cache_type);
   }

   if (Sim()->getFaultinjectionManager())
   {
      m_blocks = new char[m_num_sets * m_blocksize];
      memset(m_blocks, 0x00, m_num_sets * m_blocksize);
   } else {
      m_blocks = NULL;
   }

   //printf("Constructor for way done\n");
}

CacheWay::~CacheWay()
{

   //printf("Destructor for way entered\n");
   for (UInt32 i = 0; i < m_num_sets; i++)
      delete m_cache_block_info_array[i];
   delete [] m_cache_block_info_array;
   delete [] m_blocks;
   //printf("Destructor for way done\n");
}

void
CacheWay::read_line(UInt32 line_index, UInt32 offset, Byte *out_buff, UInt32 bytes)
{

   //printf("Read enter\n");
   assert(offset + bytes <= m_blocksize);
   //assert((out_buff == NULL) == (bytes == 0));

   if (out_buff != NULL && m_blocks != NULL)
      memcpy((void*) out_buff, &m_blocks[line_index * m_blocksize + offset], bytes);

   //printf("Read end\n");
   //Replacement policy shifted to zcache
}

void
CacheWay::write_line(UInt32 line_index, UInt32 offset, Byte *in_buff, UInt32 bytes)
{

   //printf("Write enter\n");
   assert(offset + bytes <= m_blocksize);
   //assert((in_buff == NULL) == (bytes == 0));

   if (in_buff != NULL && m_blocks != NULL)
      memcpy(&m_blocks[line_index * m_blocksize + offset], (void*) in_buff, bytes);

   //printf("Write end\n");

   //Replacement policy shifted to zcache

}

//Done for zcache
CacheBlockInfo*
CacheWay::find(IntPtr tag, UInt32 index)
{
   //printf("Find enter\n");
   if (m_cache_block_info_array[index]->getTag() == tag)
      {
         //printf("Find end\n");
         return (m_cache_block_info_array[index]);
      }

   //printf("Find end\n");
   return NULL;
}

//Done for zcache
bool
CacheWay::invalidate(IntPtr& tag, UInt32 index)
{
   //printf("Invalidate enter\n");
   if (m_cache_block_info_array[index]->getTag() == tag)
      {
         m_cache_block_info_array[index]->invalidate();
         //printf("Invalidate end\n");
         return true;
      }

   //printf("Invalidate end\n");
   return false;

}

void
CacheWay::insert(CacheBlockInfo* cache_block_info, Byte* fill_buff, bool* eviction, CacheBlockInfo* evict_block_info, Byte* evict_buff, UInt32 way_index,CacheCntlr *cntlr)
{
   // This replacement strategy does not take into account the fact that
   // cache blocks can be voluntarily flushed or invalidated due to another write request
   
   //printf("Insert enter\n");
   assert(way_index < m_num_sets);

   assert(eviction != NULL);

   if (m_cache_block_info_array[way_index]->isValid())
   {
      *eviction = true;
      // FIXME: This is a hack. I dont know if this is the best way to do
      evict_block_info->clone(m_cache_block_info_array[way_index]);
      if (evict_buff != NULL && m_blocks != NULL)
         memcpy((void*) evict_buff, &m_blocks[way_index * m_blocksize], m_blocksize);
   }
   else
   {
      *eviction = false;
   }

   // FIXME: This is a hack. I dont know if this is the best way to do
   m_cache_block_info_array[way_index]->clone(cache_block_info);

   //printf("Way %d: index: %d tag 0x%lX inserted\n", m_way_id ,way_index, m_cache_block_info_array[way_index]->getTag() );

   if (fill_buff != NULL && m_blocks != NULL)
      memcpy(&m_blocks[way_index * m_blocksize], (void*) fill_buff, m_blocksize);

   //printf("Insert end\n");
}

char*
CacheWay::getDataPtr(UInt32 line_index, UInt32 offset)
{
   return &m_blocks[line_index * m_blocksize + offset];
}

//Irrelelevant for zcache

/*
CacheWay*
CacheWay::createCacheWay(String cfgname, core_id_t core_id,
      String replacement_policy,
      CacheBase::cache_t cache_type,
      UInt32 associativity, UInt32 blocksize, CacheSetInfo* set_info, UInt32 num_sets)
{
   CacheBase::ReplacementPolicy policy = parsePolicyType(replacement_policy);
   switch(policy)
   {
      case CacheBase::ROUND_ROBIN:
         return new CacheSetRoundRobin(cache_type, associativity, blocksize);

      case CacheBase::LRU:
      case CacheBase::LRU_QBS:
         return new CacheSetLRU(cache_type, associativity, blocksize, dynamic_cast<CacheSetInfoLRU*>(set_info), getNumQBSAttempts(policy, cfgname, core_id));

      case CacheBase::NRU:
         return new CacheSetNRU(cache_type, associativity, blocksize);

      case CacheBase::MRU:
         return new CacheSetMRU(cache_type, associativity, blocksize);

      case CacheBase::NMRU:
         return new CacheSetNMRU(cache_type, associativity, blocksize);

      case CacheBase::PLRU:
         return new CacheSetPLRU(cache_type, associativity, blocksize);

      case CacheBase::SRRIP:
      case CacheBase::SRRIP_QBS:
         return new CacheSetSRRIP(cfgname, core_id, cache_type, associativity, blocksize, dynamic_cast<CacheSetInfoLRU*>(set_info), getNumQBSAttempts(policy, cfgname, core_id));

      case CacheBase::RANDOM:
         return new CacheSetRandom(cache_type, associativity, blocksize);

      case CacheBase::ZCACHE:
         return new CacheSetZcache(cache_type, associativity, blocksize);

      default:
         LOG_PRINT_ERROR("Unrecognized Cache Replacement Policy: %i",
               policy);
         break;
   }

   return (CacheSet*) NULL;
}
*/

//No set for Zcache

/*
CacheSetInfo*
CacheWay::createCacheSetInfo(String name, String cfgname, core_id_t core_id, String replacement_policy, UInt32 associativity)
{
   CacheBase::ReplacementPolicy policy = parsePolicyType(replacement_policy);
   switch(policy)
   {
      case CacheBase::LRU:
      case CacheBase::LRU_QBS:
      case CacheBase::SRRIP:
      case CacheBase::SRRIP_QBS:
         return new CacheSetInfoLRU(name, cfgname, core_id, associativity, getNumQBSAttempts(policy, cfgname, core_id));
      default:
         return NULL;
   }
}
*/

UInt8
CacheWay::getNumQBSAttempts(CacheBase::ReplacementPolicy policy, String cfgname, core_id_t core_id)
{
   switch(policy)
   {
      case CacheBase::LRU_QBS:
      case CacheBase::SRRIP_QBS:
         return Sim()->getCfg()->getIntArray(cfgname + "/qbs/attempts", core_id);
      default:
         return 1;
   }
}


//Irrelevant for zcache way
/*
CacheBase::ReplacementPolicy
CacheSet::parsePolicyType(String policy)
{
   if (policy == "round_robin")
      return CacheBase::ROUND_ROBIN;
   if (policy == "lru")
      return CacheBase::LRU;
   if (policy == "lru_qbs")
      return CacheBase::LRU_QBS;
   if (policy == "nru")
      return CacheBase::NRU;
   if (policy == "mru")
      return CacheBase::MRU;
   if (policy == "nmru")
      return CacheBase::NMRU;
   if (policy == "plru")
      return CacheBase::PLRU;
   if (policy == "srrip")
      return CacheBase::SRRIP;
   if (policy == "srrip_qbs")
      return CacheBase::SRRIP_QBS;
   if (policy == "random")
      return CacheBase::RANDOM;
   if (policy == "zcache")
      return CacheBase::ZCACHE;

   LOG_PRINT_ERROR("Unknown replacement policy %s", policy.c_str());
}
*/

bool CacheWay::isValidReplacement(UInt32 index)
{

   printf("Check valid replacement enter\n");
   if (m_cache_block_info_array[index]->getCState() == CacheState::SHARED_UPGRADING)
   {
      printf("Check valid replacement done\n");
      return false;
   }
   else
   {
      printf("Check valid replacement done\n");
      return true;
   }
}


void CacheWay::print_way(){

   for(UInt32 i = 0; i < m_num_sets;i++)
   {
      if(m_cache_block_info_array[i]->isValid())
         printf("%d:0x%lX\n", i , m_cache_block_info_array[i]->getTag() );
      //else
         //printf("%d:-",i);
   }
   printf("\n");
}

