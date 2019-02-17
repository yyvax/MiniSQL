#pragma once
#include "datatype.h"
#include "string"
const size_t BLOCK_LEN = 4096;// the size of one block，4KB
const size_t MAX_FILE_ACTIVE = 5;// limit the active files in the buffer
const size_t MAX_BLOCK = 40; // the max number of the blocks



using namespace std;



static string DATABASE_NAME;

string getDatabaseName();//返回数据库名字

void mem_init();//初始化内存中的block信息和文件信息

bool useDatabase(string DB_Name);//判断数据库文件是否存在

string get_file_path(string table_name, int file_type);//找到所存table的路径path


string get_directory_path(string databaseName);//找到存数据库文件的路径


void replace(fileInfo *m_fileInfo, blockInfo *m_blockInfo);//用lock能锁定缓冲区某块，不允许替换出去
//实现把指针m_blockInfo 所指向的块连到文件头指针m_fileInfo所指向的块链表的结尾，
//同时将m_blockInfo所指向的块的file指针指向m_fileInfo。

blockInfo *get_file_block(string Table_Name, int fileType, int blockNum);//实现缓冲区的替换算法，找替换块
//根据文件名，文件类型查找该文件是否在内存，
//如果是，根据文件的块号，从内存中查询该块，
//如该块已经在内存，返回该块的指针，
//如果该块没有在内存，判断垃圾链表中是否有空余的块，
//如有从中找到一块（链表的头），将该块的blockNUM设置为参数blockNUM，
//如果没有，判断现在内存中的块数是否已经达到了最大块数限制
//如果没有，为其分配新分配一块，并链接到对应的文件头所指的链表的结尾；
//如果已达到，使用LRU算法，找到一个替换块，按照给定的要求进行初始化，并将其链接到指定文件块链表的结尾。
//如果文件没有在内存，调用get_file_info(string fileName, int m_fileType)来为文件分配一个头指针。
//然后从blockHandle或新分配或使用LRU算法找到一个替换块（方法与上面类似），将该块按要求初始化。



void closeDatabase();//关闭数据库通过关闭存取该数据库的有关文件
 //调用closeFile(filename,fileType,m_flag),逐个关闭文件。



void closeFile(size_t num);//关闭第num个文件, dirty bit记录块状态（是否被修改过）
						   //根据文件名和文件类型，查找内存是否有这个文件，如果没有函数结束
						   //如果找到，文件数减1，对于块链表中的每个块，而且dirty位为1，调用writeBlock（m_file_num，块指针），
						   //将该块的内容写回磁盘，如果dirty位为0，不作处理。然后把真个块链表加到垃圾链表中。释放文件头的struct 。



void writeBlock(size_t m_file_num, blockInfo *block);//将block中的内容写到第m_file_num个文件
//数据块的写出										 //把block指针所指向的块的内容写回磁盘。


fileInfo *get_file_info(string fileName, int m_fileType);//给创建文件节点并附上相关信息
//首先查找该文件是否已经在内存中存在，如果是，返回该文件头；
//如果不存在，判断内存中已有的文件数是否等于最大的文件数，
//如果不是，为其分配一个文件头的struct，并根据要求进行初始化，文件个数加一，
//调用get_tablei_info(DB_Name,fileName,fileinfo->recordAmount,fileinfo->recordLength,fileinfo->freeNum)
//或get_index_info(DB_Name,fileName,fileinfo->recordAmount,fileinfo->recordLength,fileinfo->freeNum)对给文件头进行初始化，并返回该文件头；
//如果不是，选取文件头链表的第一个文件头，作为替换项，首先对选定的文件进行关闭文件操作，使得内存中的文件数目减少一个，进而为可以重新申请一个文件头，
//调用get_tablei_info(DB_Name,fileName,fileinfo->recordAmount,fileinfo->recordLength,fileinfo->freeNum)
//或get_index_info(DB_Name,fileName,fileinfo->recordAmount,fileinfo->recordLength,fileinfo->freeNum)对给文件头进行初始化，返回该文件头。

blockInfo *readBlock(string m_fileName, int m_blockNum, int m_fileType);//数据块的写入
//从磁盘中读取该块的内容到内存，返回该块。

blockInfo *get_new_block(const string& file_name, int fileType, int blockNum);

blockInfo *findBlock();
//函数实现从现已分配的内存单元中获取用于替换的内存块。
//首先判断垃圾链表blockHandle中是否有可用的内存块，如果有从链表的头取一块返回。
//如没有可用块，使用LRU算法找到没有使用的时间最长的块（itime最大），从其现在位置取出，itime置零；并将该块返回。

size_t create_file(string fileName, int fileType);//创建文件



size_t createDatabase(string databaseName);
//创建数据库,先检查是否存在该数据库，若存在则不创建，抛出相应异常



size_t delete_file(string fileName, int fileType);//删除文件
size_t deleteDatabase(string databaseName);//删除数据库



void show(blockInfo *);//输出block的信息
void show(fileInfo *);//输出文件的信息