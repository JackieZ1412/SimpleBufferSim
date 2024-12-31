#ifndef BUFFER_MANAGER_H
#define BUFFER_MANAGER_H

#include "utils.h"
#include "disk_manager.h"
#include <cassert>
#include <atomic>

class BMgr {
  public:
    BMgr(int size, int type) : cache_size(size), curr_size(0), evict_type(type) {
      victim_offset = 0;
      lru_head = new BCB();
      lru_head->data.prev = lru_head;
      lru_head->data.next = lru_head;
      for(int i = 0; i < DEFBUFSIZE; i++) {
        ftop[i] = nullptr;
        ptof[i] = nullptr;
      }
    }
    // Interface functions
    int FixPage(int page_id, bool &found);
    int FixNewPage(int page_id);
    int UnfixPage(int page_id);
    int NumFreeFrames();
    // Internal Functions
    int Hash(int page_id);
    BCB* GetBCB(int page_id);
    void RemoveBCB(BCB * ptr, int page_id);
    // void RemoveLRUEle(int frid);
    void SetDirty(int frame_id);
    void UnsetDirty(int frame_id);
    void WriteDirtys();
    void PrintFrame(int frame_id);

    void hit(int page_id);
    BCB* evict();

    void ListPushFront(BCB* head, BCB* node);
    void ListRemove(BCB* node);

    DSMgr disk_manager;
  private:
    // Hash Table
    BCB* ftop[DEFBUFSIZE];
    BCB* ptof[DEFBUFSIZE];
    bFrame buf[DEFBUFSIZE];
    int num_free;
    const int cache_size;
    std::atomic<int> curr_size;

    // `evict_type` 1:stand for clock 0: stand for lru
    const int evict_type;

    // Use for clock algorithm
    int victim_offset;

    // Use for LRU algorithm
    BCB *lru_head;
    
    // debug
    bool in_debug = false;
};

#endif