#include "include/buffer_manager.h"

// TODO: Concurrent Lock For BCB
void BMgr::hit(int page_id) {
  BCB *bptr = GetBCB(page_id);
  if(evict_type == 0) {
    // LRU Type
    ListRemove(bptr);
    ListPushFront(lru_head, bptr);
  } else {
    if(bptr->data.cnt < 8) {
      bptr->data.cnt ++;
    }
  }
};

BCB* BMgr::evict() {
  BCB *victim = nullptr;
  bool found = false;
  if(evict_type == 0) {
    // LRU evict
    while(!found) {
      victim = lru_head->data.next;
      assert(victim != lru_head);
      ListRemove(victim);
      if(victim->count == 0) {
        found = true;
        break;
      } else {
        ListPushFront(lru_head, victim);
        victim = nullptr;
      }
    }
  } else {
    while(!found) {
      if(ftop[victim_offset]->data.cnt == 0 && ftop[victim_offset]->count == 0) {
        victim = ftop[victim_offset];
        found = true;
      } else if(ftop[victim_offset]->data.cnt > 0) {
        ftop[victim_offset]->data.cnt--;
      }
      victim_offset++;
      if(victim_offset == DEFBUFSIZE) {
        victim_offset = 0;
      }
    }
  }
  assert(victim != nullptr);
  return victim;
};

int BMgr::FixPage(int page_id, bool &found) {
  int frame_id = -1;
  BCB* info = GetBCB(page_id);
  if(info != nullptr) {
    // Buffer hit
    assert(info->page_id == page_id);
    found = true;
    frame_id = info->frame_id;
    hit(info->page_id);
    info->count += 1;
  } else {
    // Buffer miss
    found = false;
    frame_id = FixNewPage(page_id);
    memcpy(&buf[frame_id], (disk_manager.ReadPage(page_id)).field, sizeof(buf[frame_id]));
  }
  return frame_id;
  
};

int BMgr::FixNewPage(int page_id) {
  assert(page_id < 50000 && page_id >= 0);
  int frame_id = -1;
  BCB *bptr = new BCB();
  bptr->page_id = page_id;
  // Cache is full, need eviction
  if(curr_size.load() >= cache_size) {
    // need evict
    // Evict deal with the situation that the item is hold
    if(in_debug) {
      std::cout << "Need eviction" << std::endl;
    }
    BCB *victim_ptr = evict();
    if(victim_ptr->dirty) {
      disk_manager.WritePage(victim_ptr->page_id,buf[victim_ptr->frame_id]);
    }
    frame_id = victim_ptr->frame_id;
    RemoveBCB(victim_ptr, victim_ptr->page_id);
  }
  if(frame_id == -1) {
    bool found = false;
    // No eviction, select a candidate from freelist

    // while(num_free == 0) {
    //   ;
    // }
    for(int i = 0;i < DEFBUFSIZE; i++) {
      if(ftop[i] == nullptr){
        frame_id = i;
        found = true;
        num_free--;
        break;
      }
    }
    assert(found == true);
  }
  if(in_debug) {
    std::cout << "Found frame id: " << frame_id << std::endl;
  }
  assert(frame_id != -1);
  ftop[frame_id] = bptr;
  bptr->frame_id = frame_id;
  bptr->count += 1;
  // Set hashtable next pointer to BCB
  int offset = Hash(page_id);
  if(in_debug) {
    std::cout << "offset in ptof is: " << offset << std::endl;
  }
  BCB *head = ptof[offset];
  if(head == nullptr) {
    ptof[offset] = bptr;
  } else {
    while(head->next != nullptr) {
      head = head->next;
    }
    head->next = bptr;
  }
  if(evict_type == 0) {
    // Admit as LRU Type
    ListPushFront(lru_head, bptr);
  } else {
    // Admit as CLOCK Type
    bptr->data.cnt += 1;
  }
  curr_size++;

  return frame_id;
  
};

int BMgr::UnfixPage(int frame_id) {
  BCB *info = ftop[frame_id];
  if(info == nullptr) {
    throw std::runtime_error("The page wanna unfix is not exist!!!");
  }
  info->count -= 1;
  return info->frame_id;
};

int BMgr::NumFreeFrames() {
  return num_free;
};

int BMgr::Hash(int page_id) {
  return page_id % DEFBUFSIZE;
};

BCB* BMgr::GetBCB(int page_id) {
  BCB *bptr = ptof[Hash(page_id)];
  if(bptr == nullptr) {
    return nullptr;
  }
  assert(bptr != nullptr);
  while(bptr->next != nullptr && bptr->page_id != page_id) {
    bptr = bptr->next;
  }
  if(bptr == nullptr || bptr->page_id != page_id) {
    return nullptr;
  } else {
    return bptr;
  }
};

void BMgr::RemoveBCB(BCB * ptr, int page_id) {
  BCB *head_ptr = ptof[Hash(ptr->page_id)];
  ftop[ptr->frame_id] = nullptr;
  num_free++;
  if(head_ptr == ptr) {
    ptof[Hash(ptr->page_id)] = ptr->next;
  } else {
    while(head_ptr->next != ptr) {
      head_ptr = head_ptr->next;
    }
    head_ptr->next = ptr->next;
  }
  delete ptr;
  curr_size--;
};

void BMgr::SetDirty(int frame_id) {
  assert(frame_id != -1);
  BCB *ptr  = ftop[frame_id];
  if(in_debug) {
    std::cout << "para frame id: " << frame_id << " and ftop page id is: " << ftop[frame_id] << std::endl;
    // std::cout << "para frame id: " << frame_id << " and bcb frame id: " << ptr->frame_id << " and bcb page id: " << ptr->page_id << std::endl;
  }
  assert(ptr != lru_head);
  assert(ptr->frame_id != -1);
  assert(ptr->frame_id == frame_id);
  ptr->dirty = true;
};

void BMgr::UnsetDirty(int frame_id) {
  BCB *ptr = ftop[frame_id];
  assert(ptr->frame_id == frame_id);
  ptr->dirty = false;
};

void BMgr::WriteDirtys() {
  for(int i = 0;i < DEFBUFSIZE; i++) {
    BCB *ptr = ptof[i];
    if(ptr != nullptr) {
      if(ptr->dirty == true) {
        disk_manager.WritePage(ptr->page_id, buf[i]);
        ptr->dirty = false;
      }
    }
  }
};

void BMgr::PrintFrame(int frame_id) {
  std::cout << "frame id: " << frame_id << " with value: " << buf[frame_id].field << std::endl;
};

void BMgr::ListPushFront(BCB* head, BCB* node) {
  node->data.next = head;
  node->data.prev = head->data.prev;
  node->data.prev->data.next = node;
  head->data.prev = node;
};

void BMgr::ListRemove(BCB* node) {
  node->data.next->data.prev = node->data.prev;
  node->data.prev->data.next = node->data.next;
  node->data.next = nullptr;
  node->data.prev = nullptr;
};