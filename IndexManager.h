#ifndef __INDEXMANAGER_H_
#define __INDEXMANAGER_H_

#include<string>
#include<vector>
#include "datatype.h"
#include "BufferManager.h"

blockInfo *get_my_file_block(const std::string& database, const std::string& table_name, int fileType, int blockNum);
inline void deleteBlock(const std::string& database, const std::string& table_name, blockInfo *block) {
    memset(block->cBlock, 0, 4096);
    block->dirtyBit = true;
}
blockInfo *get_my_new_block(const std::string& database, const std::string& table_name, int blockNum);

class IndexManager {

private:
    int N; // number of pointers in each node
    int StrLen;
    int KeyLength;
    int LeafLeast;
    int InnodeLeast;

    void setN(int n);
    void setLength(const index_info&);
    int search_leaf(const std::string& , const std::string& , const index_info& );
    int findNextLeafSibling(const std::string& , const std::string& , int );
    int findLeftMostSibling(const std::string& , const std::string& );
    int findPrevLeafSibling(const std::string& , const std::string& , int );
    int findParent(const std::string& , const std::string& , const index_info& , int );
    void write(blockInfo *const , const std::string& );
    void delete_entry(const std::string& , const std::string& , struct index_info& , int , const std::string& , int );
    void insert_leaf(const std::string& , const std::string& , struct index_info& , int );
    void insert_parent(const std::string& , const std::string& , struct index_info& , int , const std::string& , int);
    void get_index(const std::string& , const std::string& , int, int , int, struct index_info& , std::vector<int>& );
public:
    IndexManager();
    /* 寻找table中是否已经存在某一个key 若已存在则返回大于0 */
    int search_one(const std::string& , const std::string&, struct index_info& );
    /* 将符合查找条件的块号与偏移量存到vector中 */
    void search_many(const std::string& , const std::string& , int& , int& , int , struct index_info&, std::vector<int>& );
    /* 新增一个index */
    void insert_one(const std::string& , const std::string&, struct index_info& );
    /* 删除一个index */
    void delete_one(const std::string& , const std::string& , struct index_info& );
};
#endif
