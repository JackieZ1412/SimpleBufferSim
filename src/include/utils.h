#ifndef UTILS_H
#define UTILS_H

#include <cassert>

#define FRAMESIZE 4096
#define DEFBUFSIZE 1024
#define MAXPAGES 50000

struct bFrame {
  char field[FRAMESIZE];
};

struct BCB {
  struct Metadata {
    // count use for clock algorithm
    int cnt;
    // Pointer use for LRU algorithm
    BCB *prev;
    BCB *next;
  };
  BCB() : page_id(-1),frame_id(-1),latch(0),count(0),dirty(0),next(nullptr) {
    data.cnt = 0;
    data.prev = nullptr;
    data.next = nullptr;
  }
  Metadata data;
  int page_id;
  int frame_id;
  int latch;
  int count;
  int dirty;
  BCB * next;
};

#endif