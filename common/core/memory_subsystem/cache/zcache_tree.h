#ifndef CACHE_SET_ZCACHE_TREE_H
#define CACHE_SET_ZCACHE_TREE_H

#include "zcache.h"
#include <limits.h>


#define MAX_ASSOCIATIVITY 10

struct replacement_state{
  UInt32 way_id;
  UInt32 way_index;
};

class replacement_tree {

public:

CacheBlockInfo block;

UInt32 way_id;
UInt32 way_index;
UInt32 level;
UInt32 lru_counter;

replacement_state *state;

replacement_tree** children;
replacement_tree* parent_node;

public:

replacement_tree(UInt32 associativity,UInt32 num_levels)
{

    //printf("Replacement tree constructor enter\n");
    parent_node = NULL;
    children = new replacement_tree*[associativity];
    for(UInt32 i=0;i<associativity;i++)
    	children[i] = NULL;
    state = new replacement_state[num_levels];
    for(UInt32 i=0;i<num_levels;i++)
    	state[i] = {UINT_MAX,UINT_MAX};

    way_id = UINT_MAX;
    way_index = UINT_MAX;
    level = 0;

    //printf("Replacement tree constructor end\n");

};

~replacement_tree()
{

  //printf("Replacement tree destructor enter\n");

	delete [] children;
	delete [] state;

  //printf("Replacement tree destructor end\n");
};

};



/*
class ZCacheTree : public ZCache
{
   public:

      void CreateTree();

      void CreateTree(replacement_tree *parent , int cur_level);

      void traverse_tree(replacement_tree *parent, replacement_tree *repl_block);

      void delete_tree(replacement_tree *parent);


      ZCacheTree(UInt32,UInt32);
      ~ZCacheTree();


   protected:

   	  
   	  int associativity;
   	  int num_levels;
   	  
      //UInt32** global_counter; from Zcache
      //CacheWay m_ways[][]; from Zcache
};
*/

#endif /* CACHE_SET_MRU_H */
