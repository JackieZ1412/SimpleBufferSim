#include "include/buffer_manager.h"
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <fstream>
#include <filesystem>
#include <unordered_map>
#include <chrono>

std::unordered_map<std::string, int> evict_type_map = {
  {"LRU", 0},
  {"CLOCK", 1}
};

int main(int argc, char *argv[]) {
  // the args in order: 1. evict_type 2. database file path 3. data file path 4. (optional) access_number, default is the whole data file length
  if(argc < 4) {
    std::cerr << "Illegal number of parameters!" << std::endl;
    std::cerr << "The args in order: 1. evict_type 2. database file path 3. data file path 4. (optional) access_number, default is the whole data file length" << std::endl;
    std::cerr << "Other parameters are ignored" << std::endl;
    return 1;
  }
  // default evict type LRU
  int evict_type = 0;
  std::string evict_method = "LRU";
  std::string file_path = "./db_file/data.dbf";
  std::string data_path = "./data/data-5w-50w-zipf.txt";
  int total_access = -1;

  evict_method = argv[1];
  if(evict_type_map.find(evict_method) == evict_type_map.end()) {
    std::cerr << "Illegal evict type!" << std::endl;
    return 1;
  } else {
    evict_type = evict_type_map[evict_method];
  }

  file_path = argv[2];
  data_path = argv[3];
  if(argc == 5) {
    total_access = std::stoi(argv[4]);
  }
  std::cout << "Initialize database file" << std::endl;
  std::ofstream outFile(file_path, std::ios::out | std::ios::trunc);
  if(!outFile.is_open()) {
    std::cerr << "Can't open database file" << std::endl;
    return 1;
  }
  for(int i = 0;i < DEFBUFSIZE * FRAMESIZE; i++) {
    outFile << 0;
  }
  outFile.close();
  std::cout << "Finish initializing database file" << std::endl;
  BMgr buffer_manager(DEFBUFSIZE, evict_type);

  int hit = 0;
  int miss = 0;
  int read = 0;
  int write = 0;
  int access = 0;

  buffer_manager.disk_manager.OpenFile(file_path.data());
  FILE *data_file = fopen(data_path.data(), "r");
  int op;
  int page_id;
  int frame_id;


  auto timeBegin = std::chrono::system_clock::now();
  while(fscanf(data_file, "%d,%d", &op, &page_id) != EOF) {
    access++;
    // std::cout << "The access number: " << access << " the op is: " << op << " the page id is: " << page_id << std::endl;
    page_id--;
    bool found = false;
    frame_id = buffer_manager.FixPage(page_id,found);
    if(op == 1) {
      buffer_manager.SetDirty(frame_id);
    }
    buffer_manager.UnfixPage(frame_id);
    if(found) {
      hit++;
    } else {
      miss++;
    }
    if(total_access != -1 && access == total_access) {
      break;
    }
  }
  buffer_manager.WriteDirtys();
  std::cout << "Running time is: " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - timeBegin).count() << std::endl;
  buffer_manager.disk_manager.CloseFile();
  if(evict_type == 0) {
    std::cout << "Statistics: Access number: " << access << " with Hit number: " << hit << " and the hit ratio is: " << 100.0 * ((double)hit / (double)access) << " using LRU algorithm." << std::endl;
  } else if(evict_type == 1) {
    std::cout << "Statistics: Access number: " << access << " with Hit number: " << hit << " and the hit ratio is: " << 100.0 * ((double)hit / (double)access) << " using CLOCK algorithm." << std::endl;
  }

  return 0;


}