#include <math.h>
#include <string.h>
#include "mystr.h"
#include "IndexManager.h"
#include "datatype.h"

const int DataFile(0), IndexFile(1);
const char LEAF = '!', INTERNAL = '?';
const int NULLBLOCK(-3), ERRORTYPE(-1), NOTFOUNDKEY(-8), NULLKEY(-7);

using namespace std;

IndexManager::IndexManager() :StrLen(10), N(3), LeafLeast(ceil((N - 1) / 2.0)), InnodeLeast(ceil(N / 2.0) - 1) {};

//根据每个结点的最大分叉数设定B+树基本信息
void IndexManager::setN(int n) {
	N = n;
	LeafLeast = (int)ceil((N - 1) / 2.0);
	InnodeLeast = (int)ceil(N / 2.0) - 1;
}

//保存key的长度
void IndexManager::setLength(const index_info& inform)
{
	if (inform.type == INT)
		KeyLength = IntLen;
	else if (inform.type == FLOAT)
		KeyLength = FloatLen;
	else if (inform.type == CHAR)
		KeyLength = inform.length;
}

//找到要查询的key所在的叶节点
int IndexManager::search_leaf(const string& database, const string& table_name, const index_info& inform)
{
	blockInfo *node = get_my_file_block(database, table_name, IndexFile, 0);
	if (node->charNum == 0) return NULLBLOCK;
	string info = node->cBlock;
	int count = StrToI(info.substr(1, ValueLen)), start, blocknum(0);
	if (count == 0)
		return NULLKEY;
	if (inform.type == INT)
		KeyLength = IntLen;
	else if (inform.type == FLOAT)
		KeyLength = FloatLen;
	else if (inform.type == CHAR)
		KeyLength = inform.length;
	else return -1;

	while (info[0] != LEAF) {
		count = StrToI(info.substr(1, ValueLen));
		int end;
		for (int i = 0; i < count; i++) {
			start = (KeyLength + ChildLen)*i + (1 + ValueLen + ChildLen);
			end = start + KeyLength - 1;
			int compare = AnyCmp(inform.value, info.substr(start, KeyLength), inform.type);
			if (compare >= 0) {
				if (end + ChildLen == node->charNum - 1) {
					blocknum = StrToI(info.substr(end + 1, ChildLen));
					break; // hit last pointer;
				}
				else continue;
			}
			else {
				blocknum = StrToI(info.substr(start - ChildLen, ChildLen));
				break;
			}
		}
		node = get_my_file_block(database, table_name, IndexFile, blocknum);
		if (node->charNum == 0)
			return NULLBLOCK;
		info = node->cBlock;
	}
	return node->blockNum;
}

//
int IndexManager::search_one(const string& database, const string& table_name, index_info& inform) {
	int count, start;
	int blocknum = search_leaf(database, table_name, inform);

	if (blocknum < 0)
	{
		inform.offset = 0;
		return blocknum;
	}

	setLength(inform);

	blockInfo *node = get_my_file_block(database, table_name, IndexFile, blocknum);
	if (node->charNum == 0) {
		inform.offset = 0;
		return NULLBLOCK;
	}
	string info = node->cBlock;
	count = StrToI(info.substr(1, ValueLen));
	for (int i = 0; i < count; i++) {
		start = (KeyLength + LeafLen)*i + (1 + ValueLen + LeafLen);
		int compare = AnyCmp(inform.value, info.substr(start, KeyLength), inform.type);
		if (compare == 0) {
			string Linfo = info.substr(start - LeafLen, LeafLen);
			blocknum = StrToI(Linfo.substr(0, ChildLen));
			inform.offset = StrToI(Linfo.substr(ChildLen, ValueLen));
			return blocknum;
		}
	}
	inform.offset = 0;
	return NOTFOUNDKEY;
}

//寻找下一个叶子的兄弟节点
int IndexManager::findNextLeafSibling(const string& database, const string& table_name, int blocknum) {
	blockInfo *node = get_my_file_block(database, table_name, IndexFile, blocknum);
	if (node->charNum == 0)
		return NULLBLOCK;
	string info = node->cBlock;
	if (info[node->charNum - 1] == '#')
		return 0;
	return StrToI(info.substr(node->charNum - 3, 3));
}
//寻找左边
int IndexManager::findLeftMostSibling(const string& database, const string& table_name) {
	blockInfo *node = get_my_file_block(database, table_name, IndexFile, 0);
	if (node->charNum == 0)
		return NULLBLOCK;
	string left;
	string info = node->cBlock;
	while (info[0] == INTERNAL) {
		left = info.substr(1 + ValueLen, ChildLen);
		node = get_my_file_block(database, table_name, IndexFile, StrToI(left));
		info = node->cBlock;
	}
	return StrToI(left);
}

int IndexManager::findPrevLeafSibling(const string& database, const string& table_name, int blocknum) {
	int left = findLeftMostSibling(database, table_name);
	if (left == blocknum)
		return -1;
	blockInfo* Left = get_my_file_block(database, table_name, IndexFile, left);
	if (Left->charNum == 0)
		return NULLBLOCK;
	string info = Left->cBlock;
	while (info.substr(Left->charNum - 3, 3) != IToStr(blocknum, 3)) {
		left = StrToI(info.substr(Left->charNum - 3, 3));
		Left = get_my_file_block(database, table_name, IndexFile, left);
		if (Left->charNum == 0)
			return NULLBLOCK;
		info = Left->cBlock;
	}
	return left;
}

//寻找parent节点
int IndexManager::findParent(const std::string& database, const std::string& table_name, const index_info& inform, int blocknum) {
	blockInfo *node = get_my_file_block(database, table_name, IndexFile, blocknum);
	if (node->charNum == 0)
		return NULLBLOCK;
	int parent;
	setLength(inform);
	string info = node->cBlock;
	string value;
	if (info[0] == INTERNAL)
		value = info.substr(1 + ValueLen + ChildLen, KeyLength);
	else if (info[0] == LEAF)
		value = info.substr(1 + ValueLen + LeafLen, KeyLength);
	blockInfo *root = get_my_file_block(database, table_name, IndexFile, 0);
	if (root->blockNum == blocknum)
		return -1; // already root
	int start;
	info = root->cBlock;
	while (root->blockNum != blocknum) {
		int count = StrToI(info.substr(1, ValueLen));
		int end;
		int blockNum;
		for (int i = 0; i <count; i++) {
			start = (KeyLength + ChildLen)*i + (1 + ValueLen + ChildLen);
			end = start + KeyLength - 1;
			int compare = AnyCmp(value, info.substr(start, KeyLength), inform.type);
			if (compare >= 0) {
				if (end + ChildLen == root->charNum - 1) {
					blockNum = StrToI(info.substr(end + 1, ChildLen));
					break; // hit last pointer;
				}
				else
					continue;
			}
			else {
				blockNum = StrToI(info.substr(start - ChildLen, ChildLen));
				break;
			}
		}
		parent = root->blockNum;
		root = get_my_file_block(database, table_name, IndexFile, blockNum);
		info = root->cBlock;
	}
	return parent;
}

//
void IndexManager::search_many(const std::string& database, const std::string& table_name, int& start, int& end, int type, struct index_info& inform, vector<int>& container)
{
	int next;
	setLength(inform);
	int blocknum = search_leaf(database, table_name, inform);
	blockInfo *leaf = get_my_file_block(database, table_name, IndexFile, blocknum);
	string info = leaf->cBlock;
	int compare;
	/*如果是大于或者大于等于*/
	if (type == Greater || type == NotLess) {
		/*如果没有下一个兄弟节点*/
		if (info[info.size() - 1] == '#')
			compare = AnyCmp(inform.value, info.substr(info.size() - KeyLength - 1, KeyLength), inform.type);
		else
			compare = AnyCmp(inform.value, info.substr(info.size() - KeyLength - ChildLen, KeyLength), inform.type);
	}
	/*如果是小于或者小于等于*/
	else if (type == Less || NotGreater)
		compare = AnyCmp(inform.value, info.substr(1 + ValueLen + LeafLen, KeyLength), inform.type);
	string Linfo;
	if (type == Greater) {
		if (info[info.size() - 1] == '#')
		{ // last node
			if (compare >= 0) {
				start = 0;
				return;
			}
			else
				start = leaf->blockNum;
		}
		else {
			if (compare >= 0)
				start = findNextLeafSibling(database, table_name, leaf->blockNum);
			else
				start = leaf->blockNum;
		}
		while ((next = findNextLeafSibling(database, table_name, blocknum)) != 0)
			blocknum = next;
		end = blocknum;
		get_index(database, table_name, start, end, type, inform, container);
	}
	else if (type == NotLess) {
		if (info[info.size() - 1] == '#') { // last node
			if (compare > 0) {
				start = 0;
				return;
			}
			else start = leaf->blockNum;
		}
		else if (compare > 0)
				start = findNextLeafSibling(database, table_name, leaf->blockNum);

		while ((next = findNextLeafSibling(database, table_name, blocknum)) != 0)
			blocknum = next;
		end = blocknum;
		get_index(database, table_name, start, end, type, inform, container);
	}
	else if (type == Less) {
		start = findLeftMostSibling(database, table_name);
		if (leaf->blockNum == start) {
			if (compare <= 0) {
				start = 0;
				return;
			}
			else {
				end = leaf->blockNum;
				get_index(database, table_name, start, end, type, inform, container);
			}
		}
		else {
			if (compare <= 0) {
				end = findPrevLeafSibling(database, table_name, leaf->blockNum);
				get_index(database, table_name, start, end, type, inform, container);
			}
			else {
				end = leaf->blockNum;
				get_index(database, table_name, start, end, type, inform, container);
			}
		}
	}
	else if (type == NotGreater) {
		start = findLeftMostSibling(database, table_name);
		if (leaf->blockNum == start) {
			if (compare < 0) {
				start = 0;
				return;
			}
			else {
				end = leaf->blockNum;
				get_index(database, table_name, start, end, type, inform, container);
			}
		}
		else {
			if (compare < 0) {
				end = findPrevLeafSibling(database, table_name, leaf->blockNum);
				get_index(database, table_name, start, end, type, inform, container);
			}
			else {
				end = leaf->blockNum;
				get_index(database, table_name, start, end, type, inform, container);
			}
		}
	}
}

/*将符合查找条件的元素的块号和偏移量存到container中*/
void IndexManager::get_index(const std::string& database, const std::string& table_name, int begin, int end, int type, struct index_info& inform, vector<int>& container)
{
	int  count, start, compare;
	setLength(inform);

	int between = begin, prev = -1;
	//bool flag = false;
	while (prev != end) {
		blockInfo *leaf = get_my_file_block(database, table_name, IndexFile, between);
		string info = leaf->cBlock;
		count = StrToI(info.substr(1, ValueLen));
		for (int i = 0; i < count; i++) {
			start = (KeyLength + LeafLen)*i + (1 + ValueLen + LeafLen);
			compare = AnyCmp(inform.value, info.substr(start, KeyLength), inform.type);
			switch (type) {
			case Greater:
				if (compare < 0)
						container.push_back(StrToI(info.substr(start - OffsetLen, OffsetLen)));
				break;
			case NotLess:
				if (compare <= 0)
						container.push_back(StrToI(info.substr(start - OffsetLen, OffsetLen)));
				break;
			case Less:
				if (compare > 0)
					container.push_back(StrToI(info.substr(start - OffsetLen, OffsetLen)));
				break;
			case NotGreater:
				if (compare >= 0)
					container.push_back(StrToI(info.substr(start - OffsetLen, OffsetLen)));
				break;
			}
		}
		prev = between;
		between = StrToI(info.substr(leaf->charNum - 3, 3));
	}
}

//删除一个数据
void IndexManager::delete_one(const string& database, const string& table_name, struct index_info& inform) {
	int L = search_leaf(database, table_name, inform);
	delete_entry(database, table_name, inform, L, inform.value, L);
}

// n:需要检查的节点, K: 关键key, nod: n的儿子节点，要被删除的节点
void IndexManager::delete_entry(const string& database, const string& table_name, struct index_info& inform, int n, const string& K, int nod)
{
	setLength(inform);

	setN(4088 / (KeyLength + LeafLen));
	blockInfo *Node = get_my_file_block(database, table_name, IndexFile, n);
	string info = Node->cBlock;
	string originN = info;
	int num = StrToI(info.substr(1, ValueLen));
	string tmpN = Node->cBlock;
	tmpN.replace(1, ValueLen, IToStr(num - 1, ValueLen));
	int start;
	for (int i = 0; i < num; i++) {
		int compare;
		if (tmpN[0] == INTERNAL)
			start = (KeyLength + ChildLen)*i + ValueLen + ChildLen + 1;
		else if (tmpN[0] == LEAF)
			start = (KeyLength + LeafLen)*i + ValueLen + LeafLen + 1;
		compare = AnyCmp(K, info.substr(start, KeyLength), inform.type);
		if (compare == 0) {
			if (tmpN[0] == INTERNAL) {
				if (tmpN.substr(start - ChildLen, ChildLen) == IToStr(nod, ChildLen)) { // parent
					tmpN.replace(start - ChildLen, KeyLength + ChildLen, "");
					break;
				}
				else if (tmpN.substr(start + KeyLength, ChildLen) == IToStr(nod, ChildLen)) { // parent
					tmpN.replace(start, KeyLength + ChildLen, "");
					break;
				}
			}
			else if (tmpN[0] == LEAF) { // leaf
				tmpN.replace(start - LeafLen, KeyLength + LeafLen, "");
				break;
			}
		}
	}
	int parent = findParent(database, table_name, inform, Node->blockNum);
	write(Node, tmpN);
	/*删除掉节点根节点只有一个孩子*/
	if (Node->blockNum == 0 && originN.substr(1, ValueLen) == "0001") {
		/*如果已经是叶节点*/
		if (tmpN[0] == LEAF)
			return;
		// 将叶节点的儿子作为根节点
		blockInfo *child = get_my_file_block(database, table_name, IndexFile, StrToI(originN.substr(1+ValueLen, ChildLen)));
		if (child->blockNum == nod)
			child = get_my_file_block(database, table_name, IndexFile, StrToI(originN.substr(13,ChildLen)));
		Node->blockNum = child->blockNum;
		child->blockNum = 0;
		//writeRootBlock(database, table_name, child);
		deleteBlock(database, table_name, Node);
		return;
	}
	// 如果Node节点是root且至少有两个孩子
	if (Node->blockNum == 0) {
		return;
	}
	/*Node 不是root节点*/
	else
	{
		info = Node->cBlock;
		tmpN = info;
		/*如果Node是叶节点*/
		if (info[0] == LEAF) {
			/*如果Node节点的key的个数小于LeafLeast*/
			if (StrToI(info.substr(1, ValueLen)) < LeafLeast) {
				int n1 = findPrevLeafSibling(database, table_name, n);
				blockInfo *N1 = NULL;
				int parent1;
				if (n1 >= 0) {
					N1 = get_my_file_block(database, table_name, IndexFile, n1);
					parent1 = findParent(database, table_name, inform, N1->blockNum);
				}
				else parent1 = -1;
				/*如果Node节点和它的上一个节点是同一个父母*/
				if (parent == parent1) {
					string K1 = originN.substr(1 + ValueLen + LeafLen, KeyLength);
					string tmpN1 = N1->cBlock;
					/*如果上一个节点可以容纳两个节点的个数的总和，则合并*/
					if (StrToI(info.substr(1, ValueLen)) + StrToI(tmpN1.substr(1, ValueLen)) <= N - 1) { // merge
						tmpN1.replace(1, ValueLen, IToStr(StrToI(info.substr(1, ValueLen)) + StrToI(tmpN1.substr(1, ValueLen)), ValueLen));
						tmpN1.replace(tmpN1.size() - ChildLen, ChildLen, "");
						tmpN1 += info.substr(5, strlen(Node->cBlock) - 5);
						write(N1, tmpN1);
						delete_entry(database, table_name, inform, parent, K1, n);
						deleteBlock(database, table_name, Node);
					}
					/*否则将上一个节点的最后一个元素移动到此节点的第一个元素位置*/
					else {
						int numN1 = StrToI(tmpN1.substr(1, 4));
						string last = tmpN1.substr(5 + (KeyLength + LeafLen)*(numN1 - 1), LeafLen + KeyLength);
						tmpN1.replace(1, 4, IToStr(StrToI(tmpN1.substr(1, 4)) - 1, 4));
						tmpN1.replace(5 + (KeyLength + LeafLen)*(numN1 - 1), LeafLen + KeyLength, "");
						write(N1, tmpN1);
						tmpN.replace(1, 4, IToStr(StrToI(tmpN.substr(1, 4)) + 1, 4));
						tmpN.insert(5, last);
						write(Node, tmpN);
						string Km = last.substr(LeafLen, KeyLength);
						blockInfo* Parent = get_my_file_block(database, table_name, IndexFile, parent);
						string tmpP = Parent->cBlock;
						num = StrToI(tmpP.substr(1, 4));
						for (int i = 0; i < num; i++) {
							start = (3 + KeyLength)*i + 8;
							if (tmpP.substr(start, KeyLength) == K1) {
								tmpP.replace(start, K1.size(), Km);
								write(Parent, tmpP);
								break;
							}
						}
					}
				}
				/*否则此节点和它的下一个节点是同一个父母*/
				else {
					n1 = findNextLeafSibling(database, table_name, n);
					blockInfo *N1 = get_my_file_block(database, table_name, IndexFile, n1);
					string tmpN1 = N1->cBlock;
					string K1 = tmpN1.substr(1 + ValueLen + LeafLen, KeyLength);
					/*如果下一个节点的空间可以容纳两个节点的元素的总和，则直接合并*/
					if (StrToI(tmpN.substr(1, 4)) + StrToI(tmpN1.substr(1, 4)) <= N - 1) { // merge
						tmpN1.replace(1, 4, IToStr(StrToI(info.substr(1, 4)) + StrToI(tmpN1.substr(1, 4)), 4));
						tmpN.replace(tmpN.size() - 3, 3, "");
						tmpN1.insert(5, tmpN.substr(5, tmpN.size() - 5));
						write(N1, tmpN1);
						delete_entry(database, table_name, inform, parent, K1, n);
						deleteBlock(database, table_name, Node);
					}
					/*否则将下一个节点的第一个元素移动到本节点的最后一个元素*/
					else {
						string first = tmpN1.substr(1 + ValueLen, LeafLen + KeyLength);
						tmpN1.replace(1, 4, IToStr(StrToI(tmpN1.substr(1, 4)) - 1, 4));
						tmpN1.replace(1 + ValueLen, KeyLength + LeafLen, "");
						write(N1, tmpN1);
						tmpN.replace(1, 4, IToStr(StrToI(tmpN.substr(1, 4)) + 1, 4));
						tmpN.insert(tmpN.size() - 3, first);
						write(Node, tmpN);
						string N1K1 = tmpN1.substr(1 + ValueLen + LeafLen, KeyLength);
						blockInfo* Parent = get_my_file_block(database, table_name, IndexFile, parent);
						string tmpP = Parent->cBlock;
						num = StrToI(tmpP.substr(1, 4));
						for (int i = 0; i < num; i++) {
							start = (3 + KeyLength)*i + 8;
							if (tmpP.substr(start, KeyLength) == K1) {
								tmpP.replace(start, KeyLength, N1K1);
								write(Parent, tmpP);
								break;
							}
						}
					}
				}
			}

		}
		/*如果是内部节点*/
		else if (info[0] == INTERNAL) {
			/*如果key的个数小于节点个数的最小值*/
			if (StrToI(info.substr(1, ValueLen)) < InnodeLeast) { // merge
				blockInfo *Parent = get_my_file_block(database, table_name, IndexFile, parent);
				string tmpP = Parent->cBlock;
				int num = StrToI(tmpP.substr(1, ValueLen));
				int start, end, n1, pre = 1; // bool previous
				string K1;
				for (int i = 0; i <= num; i++) {
					start = (3 + KeyLength)*i + 5;
					end = start + 3;
					/*找到Node节点的父指针*/
					if (tmpP.substr(start, 3) == IToStr(Node->blockNum, 3)) {
						if (end != 8) { // 如果不是第一个儿子，则有前一个兄弟节点
							n1 = StrToI(tmpP.substr(start - KeyLength - 3, 3));
							K1 = tmpP.substr(start - KeyLength, KeyLength);
							pre = 1;
						}
						else { // 如果没有前一个兄弟节点，找到下一个节点
							n1 = StrToI(tmpP.substr(end + KeyLength, 3));
							K1 = tmpP.substr(end, KeyLength);
							pre = 0;
						}
						break;
					}

				}
				blockInfo *N1 = get_my_file_block(database, table_name, IndexFile, n1);
				string tmpN1 = N1->cBlock;
				//如果找到的是前一个兄弟节点
				if (pre == 1) {
					/*如果两个节点的key可以在一个节点中放的下，则合并*/
					if (StrToI(tmpN.substr(1, ValueLen)) + StrToI(tmpN1.substr(1, ValueLen)) < N - 1) {
						tmpN1.replace(1, 4, IToStr(StrToI(tmpN.substr(1, 4)) + StrToI(tmpN1.substr(1, 4)) + 1, 4));
						string tail = tmpN.substr(5, Node->charNum - 5);
						string head = tmpN1.substr(0, 5);
						tmpN1 = head + K1 + tail;
						write(N1, tmpN1);
						delete_entry(database, table_name, inform, parent, K1, n);
						deleteBlock(database, table_name, Node);
					}
					/*否则重新分配节点*/
					else {
						string N1Pm = tmpN1.substr(N1->charNum - 3, 3); // last pointer in N1
						string N1K = tmpN1.substr(N1->charNum - 3 - KeyLength, KeyLength);
						tmpN1.replace(1, 4, IToStr(StrToI(tmpN1.substr(1, 4)) - 1, 4));
						tmpN1 = tmpN1.substr(0, N1->charNum - 3 - KeyLength);
						write(N1, tmpN1);
						tmpN.replace(1, 4, IToStr(StrToI(tmpN.substr(1, 4)) + 1, 4));
						string head = tmpN.substr(0, 5);
						string tail = tmpN.substr(5, Node->charNum - 5);
						tmpN = head + N1Pm + K1 + tail;
						write(Node, tmpN);
						tmpP.replace(start - KeyLength, KeyLength, N1K);
						write(Parent, tmpP);
					}
				}
				/*如果是下一个兄弟节点*/
				else if (pre == 0) {
					/*如果两个节点的key可以在一个节点中放的下，则合并*/
					if (StrToI(tmpN.substr(1, 4)) + StrToI(tmpN1.substr(1, 4)) < N - 1) {
						tmpN1.replace(1, 4, IToStr(StrToI(tmpN.substr(1, 4)) + StrToI(tmpN1.substr(1, 4)) + 1, 4));
						string head = tmpN1.substr(0, 5);
						string tail = tmpN1.substr(5, N1->charNum - 5);
						string part = tmpN.substr(5, tmpN.size() - 5);
						tmpN1 = head + part + K1 + tail;
						write(N1, tmpN1);
						delete_entry(database, table_name, inform, parent, K1, n);
						deleteBlock(database, table_name, Node);
					}
					/*否则重新分配节点*/
					else {
						string N1P1 = tmpN1.substr(5, 3);
						string N1K1 = tmpN1.substr(8, KeyLength);
						tmpN1.substr(1, 4) = IToStr(StrToI(tmpN1.substr(1, 4)) - 1, 4);
						string headN1 = tmpN1.substr(0, 5);
						string tailN1 = tmpN1.substr(8 + KeyLength, N1->charNum - 8 - KeyLength); tmpN1 = headN1 + tailN1; write(N1, tmpN1); tmpN.replace(1, 4, IToStr(StrToI(tmpN.substr(1, 4)) + 1, 4)); tmpN = tmpN + K1 + N1P1; write(Node, tmpN); tmpP.replace(end, KeyLength, N1K1); write(Parent, tmpP);
					}
				}
			}
		}
	}
}

//
void IndexManager::insert_one(const std::string& database, const std::string& table_name, struct index_info& inform) {
	blockInfo *root = get_my_file_block(database, table_name, IndexFile, 0);
	int  count, start, compare, Node;
	setLength(inform);
	setN(4088 / (KeyLength + LeafLen));
	if (root->charNum == 0) {
		string tmp ="!0001";
		tmp += IToStr(0, ChildLen);
		tmp += IToStr(inform.offset, OffsetLen);
		tmp += SpanStrToLen(inform.value, KeyLength);
		tmp += "#";
		write(root, tmp);
		return;
	}

	//count = StrToI(info.substr(1, ValueLen));
	Node = search_leaf(database, table_name, inform);
	blockInfo *node = get_my_file_block(database, table_name, IndexFile, Node);
	string info = node->cBlock;
	if (StrToI(info.substr(1, ValueLen)) < N - 1)
		insert_leaf(database, table_name, inform, Node);
	else {  // divide T - L1
		blockInfo *L1 = get_my_new_block(database, table_name, -1);
		int l1 = L1->blockNum;
		blockInfo *T = get_my_new_block(database, table_name, l1);////////
		int t = T->blockNum; // tmp backup of leaf node;///////
		strcpy(T->cBlock, info.c_str());//////
		string pn;
		if (info[node->charNum - 1] == '#')
			pn = info.substr(node->charNum - 1, 1);
		else
			pn = info.substr(node->charNum - ChildLen, ChildLen);

		insert_leaf(database, table_name, inform, t);/////

		string tmpT = T->cBlock;
		int half = ceil((StrToI(info.substr(1, ValueLen)) + 1) / 2.0);
		string tmpL = LEAF + IToStr(half, ValueLen);
		tmpL += tmpT.substr(1 + ValueLen, (LeafLen + KeyLength)*half);
		tmpL += IToStr(l1, ChildLen);
		write(node, tmpL);
		int n = StrToI(tmpT.substr(1, ValueLen)); // total value num
		string tmpL1;
		tmpL1 = LEAF + IToStr(n - half, ValueLen);
		tmpL1 += tmpT.substr(1 + ValueLen + (LeafLen + KeyLength)*half, (n - half)*(KeyLength + LeafLen));
		tmpL1 += pn;
		write(L1, tmpL1);
		string K1 = tmpL1.substr(1 + ValueLen + LeafLen, KeyLength); // smallest value in L1;
		insert_parent(database, table_name, inform, Node, K1, l1);
		deleteBlock(database, table_name, T);
	}
}

// insert, do not concern about the division
void IndexManager::insert_leaf(const string& database, const string& table_name, struct index_info& inform, int Node) {
	blockInfo *leaf = get_my_file_block(database, table_name, IndexFile, Node);
	string info = leaf->cBlock;
	int count = StrToI(info.substr(1, ValueLen));
	int start, compare;
	setLength(inform);

	string tmp = leaf->cBlock;
	string bnum = IToStr(0, ChildLen); // change!!!
	string offset = IToStr(inform.offset, OffsetLen);
	string insert = bnum + offset + SpanStrToLen(inform.value, KeyLength);
	tmp.replace(1, ValueLen, IToStr(StrToI(info.substr(1, ValueLen)) + 1, ValueLen));
	for (int i = 0; i < count; i++) {
		start = (KeyLength + LeafLen)*i + 1 + ValueLen + LeafLen;
		compare = AnyCmp(inform.value, info.substr(start, KeyLength), inform.type);

		if (compare < 0) {
			tmp.insert(start - LeafLen, insert);
			write(leaf, tmp);
			return;
		}
	}
	// insert in the last
	if (tmp[tmp.size() - 1] == '#')
		tmp.insert(tmp.size() - 1, insert);
	else
		tmp.insert(tmp.size() - ChildLen, insert);
	write(leaf, tmp);
	return;
}


void IndexManager::insert_parent(const std::string& database, const std::string& table_name, struct index_info& inform, int Node, const string& K1, int N1) {
	blockInfo *root = get_my_file_block(database, table_name, IndexFile, 0);
	setLength(inform);

	if (root->blockNum == Node) { // Node == 0
		blockInfo *R = get_my_new_block(database, table_name, -1);
		int r = R->blockNum;
		string info = "?0001" + IToStr(r, ChildLen) + K1 + IToStr(N1, ChildLen);
		R->blockNum = Node;
		root->blockNum = r;
		write(R, info);
		//writeRootBlock(database, table_name, R);
		return;
	}
	blockInfo *N0 = get_my_file_block(database, table_name, IndexFile, Node);
	int p = findParent(database, table_name, inform, N0->blockNum);
	blockInfo *P = get_my_file_block(database, table_name, IndexFile, p);
	string info = P->cBlock;
	if (StrToI(info.substr(1, 4)) < N - 1) {
		string tmp = P->cBlock;
		int count = StrToI(tmp.substr(1, ValueLen));
		int start;
		tmp.replace(1, ValueLen, IToStr(count + 1, ValueLen));
		for (int i = 0; i <= count; i++) {
			start = (KeyLength + ChildLen)*i + 1 + ValueLen;
			if (tmp.substr(start, ChildLen) == IToStr(Node, ChildLen)) {
				tmp.insert(start + ChildLen, K1 + IToStr(N1, ChildLen));
				write(P, tmp);
				return;
			}
		}
	}
	else { // divide P;
		blockInfo *P1 = get_my_new_block(database, table_name, -1);
		int p1 = P1->blockNum;
		blockInfo *T = get_my_new_block(database, table_name, p1);
		T->cBlock = P->cBlock;
		string tmpT = T->cBlock;
		int count = StrToI(tmpT.substr(1, ValueLen));
		int start;
		tmpT.replace(1, ValueLen, IToStr(count + 1, ValueLen));
		for (int i = 0; i < count; i++) {
			start = (KeyLength + ChildLen)*i + 1 + ValueLen;
			if (tmpT.substr(start, ChildLen) == IToStr(Node, ChildLen)) {
				tmpT.insert(start + ChildLen, K1 + IToStr(N1, ChildLen));
				break;
			}
		}
		int half = ceil(count / 2.0);
		string tmpP = INTERNAL + IToStr(half, ValueLen);
		tmpP += tmpT.substr(5, (KeyLength + ChildLen)*half + ChildLen);
		write(P, tmpP);
		// remaining half, move K11 up to parent
		string K11 = tmpT.substr(5 + (KeyLength + ChildLen)*half + ChildLen, KeyLength);
		string tmpP1 = INTERNAL + IToStr(count - half, ValueLen);
		tmpP1 += tmpT.substr(5 + (KeyLength + ChildLen)*(half + 1), (KeyLength + ChildLen)*(count - half) + ChildLen);
		write(P1, tmpP1);
		insert_parent(database, table_name, inform, p, K11, p1);
		deleteBlock(database, table_name, T);
	}
}


void IndexManager::write(blockInfo *const node, const string& s) {
	memset(node->cBlock, 0, 4096);

	strcpy(node->cBlock, s.c_str());
	node->charNum = strlen(node->cBlock);
	node->dirtyBit = true;
}


blockInfo *get_my_file_block(const std::string& database, const std::string& table_name, int fileType, int blockNum) {
	blockInfo *node = readBlock(table_name, blockNum, fileType);
	if (node == NULL) {
		node = get_new_block(table_name, fileType, blockNum);
		memset(node->cBlock, 0, 4096);
		node->charNum = 0;
		return node;
	}
	int i = -1;
	while (node->cBlock[++i] != '\0');
	node->charNum = i;
	return node;
}

blockInfo *get_my_new_block(const std::string& database, const std::string& table_name, int blockNum) {
	blockInfo *node;
	while (node = readBlock(table_name, ++blockNum, IndexFile)) {
		if (node->cBlock[0] == '\0')
			break;
	}
	if (node == NULL)
		node = get_new_block(table_name, IndexFile, blockNum);
	node->charNum = 0;
	memset(node->cBlock, 0, 4096);
	return node;
}


