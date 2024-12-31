#include "include/disk_manager.h"

int DSMgr::OpenFile(std::string filename) {
  currFile = fopen(filename.c_str(), "rb+");
  if(currFile == nullptr) {
    return -1;
  }
  return 0;
};
int DSMgr::CloseFile() {
  return fclose(currFile);
};
bFrame DSMgr::ReadPage(int page_id) {
  bFrame frame;
  Seek(0, page_id * FRAMESIZE);
  fread(frame.field, FRAMESIZE, 1, currFile);
  return frame;
};
int DSMgr::WritePage(int page_id, bFrame frm) {
  Seek(0, page_id * FRAMESIZE);
  return fwrite(frm.field, FRAMESIZE, FRAMESIZE, currFile);
};
int DSMgr::Seek(int offset, int pos) {
  return fseek(currFile, pos + offset, SEEK_SET);
};
FILE * DSMgr::GetFile() {
  return currFile;
};
void DSMgr::IncNumPages() {
  if(numPages > MAXPAGES) {
    throw std::runtime_error("Page number exceeds the limit!!!");
  }
  numPages++;
};
int DSMgr::GetNumPages() {
  return numPages;
};
void DSMgr::SetUse(int index, int use_bit) {
  // As a matter of fact, we only read/write the page for simulation instead pf records, so we can't manipulate the record access
  // So we don't need to maintain the `use_bit`
  // In other words, the `use_bit` only can be set to 1 but never to 0 in our Buffer/Disk Manager Simulation Framework
};
int DSMgr::GetUse(int index) {
  // The same as `SetUse` function before
  return 0;
};