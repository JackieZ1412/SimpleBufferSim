#ifndef DISK_MANAGER_H
#define DISK_MANAGER_H

#include <cstring>
#include <string>
#include <iostream>
#include <stdexcept>
#include "utils.h"

class DSMgr {
  public:
    DSMgr() {}
    int OpenFile(std::string filename);
    int CloseFile();
    bFrame ReadPage(int page_id);
    int WritePage(int page_id, bFrame frm);
    int Seek(int offset, int pos);
    FILE * GetFile();
    void IncNumPages();
    int GetNumPages();
    void SetUse(int index, int use_bit);
    int GetUse(int index);
  private:
    FILE *currFile;
    int numPages;
    int pages[MAXPAGES];
};


#endif