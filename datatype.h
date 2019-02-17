#ifndef __DATATYPE_H_
#define __DATATYPE_H_
#include <string>
#include "Data.h"
const int ValueLen(4), ChildLen(3), OffsetLen(4), LeafLen(7), IntLen(5), FloatLen(10);
const int Greater(1), NotLess(2), Less(3), NotGreater(4);

struct blockInfo;


//文件结构体(struct fileInfo)
//包含文件的基本信息;类型（数据（表）文件或索引文件）、文件名、
//记录数、空块号、记录长度、指向下一个文件的指针、文件所指向的第一个块。
struct fileInfo {
    int type;	 // 0-> data file， 1 -> index file
    string fileName;	// the name of the file
    int recordAmount;
    int freeNum;// the block number a file needs
    int recordLength;// the length of the record in the file      or n 个节点
    fileInfo *next;// the pointer points to the next file
    blockInfo *firstBlock;// point to the first block within the file
};



//块信息结构体(struct blockInfo)
//包含块的基本信息;块号、脏位、指向下一个块的指针、块中的字符数
//、存放信息的字符型数组、年龄（用于LRU算法）、锁。
struct blockInfo {
    int blockNum;// the block number of the block,
    bool dirtyBit;// 0 -> flase， 1 -> indicate dirty, write back
    blockInfo *prev;// the pointer points to the pre block
    blockInfo *next;// the pointer points to the next block
    fileInfo *file;// the pointer point to the file, which the block belongs to
    int charNum;// the number of chars not used in the block
    char *cBlock;	// the array space for storing the records in the block in buffer
    int iTime;// it indicate the age of the file in use
    int lock;// prevent the block from replacing
};


struct index_info {
    std::string index_name;     //the name of the index file
    int length;                 //the length of the value
    char type;                  //the type of the value  0---int,1---float,2----char(n)
    long offset;                //the record offset in the table file
    std::string value;          //the value

    index_info(){}
    index_info(const string& name,const Field& f, const char *_value) {
        index_name = name + "_" + f.name;
        length = f.length;
        type = f.type;
        value = _value;
    }
};


#endif
