# 存储和缓冲管理器实验说明

## 高级数据库系统 (2024)

### 目录

- [引言](#引言)
- [缓冲和帧](#缓冲和帧)
- [页面格式](#页面格式)
- [文件格式](#文件格式)
- [缓冲技术](#缓冲技术)
- [哈希技术](#哈希技术)
- [文件存储](#文件存储)
- [类设计](#类设计)
- [实验设置](#实验设置)
- [实现计划](#实现计划)

## 引言

在这个项目中，我们将实现一个简单的存储和缓冲管理器。本文档涉及存储和缓冲管理器。将讨论缓冲和帧的大小、缓冲和帧的存储结构、页面格式、页面存储结构、文件格式、缓冲技术、哈希技术、文件存储结构，以及磁盘空间和缓冲模块的接口函数。所选技术来自课堂上与缓冲和存储管理器相关的材料。

## 缓冲和帧

### 缓冲和帧大小

缓冲指的是主内存中的空间。CPU只能访问主内存中的内容。缓冲由一系列帧组成。当请求页面时，它会被加载到内存中的缓冲区。大多数商业数据库管理系统将帧大小设置为与页面大小相同，以防止外部碎片。项目中采用了相同的策略。默认情况下，缓冲区大小将设置为1024。

### 缓冲和帧存储结构

缓冲由称为帧的逻辑分区组成。帧将存储在全局定义的结构中，描述帧的外观。此结构将定义为：

```c
#define FRAMESIZE 4096
struct bFrame {
    Char field [FRAMESIZE ];
};
```

缓冲数组将存储一系列帧，这些帧将存储加载到其中的页面。此数组将如下所示：

```c
#define DEFBUFSIZE 1024
bFrame buf[DEFBUFSIZE]; // 或者使用用户通过输入参数定义的大小
```

这将是为缓冲区分配的空间，缓冲管理器和文件及访问方法将通过它引用所需的页面。

## 页面格式

在这个项目中，我们不需要参考页面的详细结构。唯一重要的信息是页面ID和页面大小。因此，你不需要设计页面格式。

## 文件格式

我们建议使用基于目录的结构来组织数据库文件，如课堂上介绍的。每个文件都有一个基页面，其中包含指向文件中每个页面的指针。基页面中的每个指针按顺序排列。此类文件的数据页面没有指针，只有记录。必须咨询基页面（或目录）才能获取文件中的下一个页面。选择基于目录的文件格式是因为它适合于查找特定页面，这些页面被请求记录。文件的目录基础将允许快速访问正确的页面，而无需搜索长列表的页面以找到正确的页面。

## 缓冲技术

我们选择LRU作为实验室中唯一的替换策略。LRU总是从LRU队列中逐出最近最少使用的页面，该队列用于组织缓冲页面，这些页面按最后引用的时间排序。它总是选择LRU位置的页面作为受害者。LRU最重要的优势是其恒定的运行时间复杂度。此外，LRU以其在参考模式具有高时间局部性的情况下表现出色而闻名，即当前引用的页面在不久的将来被重新引用的可能性很高。

## 哈希技术

对于缓冲中的每个帧，必须保持一个缓冲控制块（BCB）。每个缓冲控制块包含页面ID、帧ID、页面互斥锁、固定计数和脏位。页面ID用作键，进入哈希函数，该函数将页面ID映射到BCB。必须保持两个哈希表：一个将页面ID映射到帧ID和BCB，另一个将帧ID映射到页面ID。我们建议使用简单的静态哈希技术。在静态哈希中，桶的数量是固定的。如果一个桶满了，就会连接一个溢出链，用于额外的数据条目。使用键值，哈希函数将其映射到一个桶。在单个桶内使用顺序搜索。随着时间的推移，桶的数量不会改变。

## 文件存储

在我们的项目中，我们只需要磁盘上的一个物理文件。数据库中的所有数据都将保存在这个单个文件中。该文件将保留在工作目录中，并命名为data.dbf。即使在系统首次运行时，也应该始终找到此文件，此时它将是空的。

## 类设计

### 数据存储管理器类 DSMgr

```c
class DSMgr {
public:
    DSMgr();
    int OpenFile(string filename);
    int CloseFile();
    bFrame ReadPage(int page_id);
    int WritePage(int frame_id, bFrame frm);
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
```

### 缓冲管理器类 BMgr

```c
class BMgr {
public:
    BMgr();
    int FixPage(int page_id, int prot);
    void FixNewPage();
    int UnfixPage(int page_id);
    int NumFreeFrames();
    int SelectVictim();
    int Hash(int page_id);
    void RemoveBCB(BCB * ptr, int page_id);
    void RemoveLRUEle(int frid);
    void SetDirty(int frame_id);
    void UnsetDirty(int frame_id);
    void WriteDirtys();
    void PrintFrame(int frame_id);
private:
    int ftop[DEFBUFSIZE];
    BCB* ptof[DEFBUFSIZE];
};
```

## 缓冲接口函数

这些接口函数将为上面的文件和访问管理器提供接口。所需的功能如下：

- FixPage(int page_id, int prot)：此函数的原型是FixPage(Page_id, protection)，它返回一个frame_id。文件和访问管理器将使用记录ID中的page_id调用此页面。该函数查看页面是否已经在缓冲区中，并在是的情况下返回相应的frame_id。如果页面尚未在缓冲区中，它将选择一个受害者页面（如果需要），并加载请求的页面。
- FixNewPage()：此函数的原型是FixNewPage()，它返回一个page_id和一个frame_id。此函数用于在插入、索引拆分或对象创建时需要新页面。page_id返回以便分配给记录ID和元数据。此函数将找到一个空页面，文件和访问管理器可以使用它来存储一些数据。
- UnfixPage(int page_id)：此函数的原型是UnfixPage(page_id)，它返回一个frame_id。此函数是FixPage或FixNewPage调用的补充。此函数递减帧上的固定计数。如果计数减少到零，则移除页面上的互斥锁，如果选择，帧可以被移除。将page_id转换为frame_id，它可能被解锁，以便它可以被选为受害者页面，如果页面上的计数已减少到零。
- NumFreeFrames()：NumFreeFrames函数查看缓冲区并返回可以使用的空闲缓冲页面的数量。这对于查询处理器的N路排序特别有用。此函数的原型看起来像NumFreeFrames()，返回一个整数，范围从0到BUFFERSIZE-1(1023)。
- SelectVictim()：SelectVictim函数选择一个帧进行替换。如果选定帧的脏位已设置，则需要将页面写入磁盘。
- Hash(int page_id)：哈希函数将page_id作为参数并返回帧ID。

## 数据存储接口函数

当前数据文件将保留在DSManager类中。此文件将命名为data.dbf。

- OpenFile(string filename)：OpenFile函数在需要打开文件进行读写时被调用。此函数的原型是OpenFile(String filename)，返回一个错误代码。该函数打开由filename指定的文件。
- CloseFile()：CloseFile函数在需要关闭数据文件时被调用。此函数的原型是CloseFile()，返回一个错误代码。此函数关闭当前正在使用的文件。此函数应该只在数据库发生变化或程序关闭时调用。
- ReadPage(int page_id)：ReadPage函数由缓冲管理器中的FixPage函数调用。此函数的原型是ReadPage(page_id, bytes)，返回它读取的内容。此函数调用fseek()和fread()从文件中获取数据。
- WritePage(int frame_id, bFrame frm)：WritePage函数在页面从缓冲区取出时被调用。此函数的原型是WritePage(frame_id, frm)，返回写入的字节数。此函数调用fseek()和fwrite()将数据保存到文件中。
- Seek(int offset, int pos)：Seek函数移动文件指针。
- GetFile()：GetFile函数返回当前文件。
- IncNumPages()：IncNumPages函数递增页面计数器。
- GetNumPages()：GetNumPages函数返回页面计数器。
- SetUse(int page_id, int use_bit)：SetUse函数设置pages数组中的位。此数组跟踪正在使用的页面。如果页面中的所有记录都被删除，则该页面实际上不再使用，可以在数据库中再次使用。为了知道页面是否可重用，检查数组中是否有
