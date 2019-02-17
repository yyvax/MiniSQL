#ifndef RECORDMANAGER_DATA_H_
#define RECORDMANAGER_DATA_H_

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include "DataException.h"
#include "DBException.h"

using namespace std;

enum KeyType{NORMAL, UNIQUE, PRIMARY};
enum DataType{INT, FLOAT, CHAR};
const int LENGTH_OF_INT = 4;
const int LENGTH_OF_FLOAT = 4;
const int LENGTH_OF_CHAR = 1;
enum Op { EQUALS, GREATER_THAN, GREATE_THAN_OR_EQUAL, LESS_THAN, LESS_THAN_OR_EQUAL, NOT_EQUAL };

class Data {//数据定义信息
private:
	unsigned type;
	unsigned length;
public:
	Data(unsigned _type, unsigned _length);
	virtual ~Data();
	int getType() const;
	unsigned getLength() const;
	virtual bool compare(Op op, const Data* data);
	virtual void* getValue() const = 0;

};

class Int : public Data {
private:
	int value;
public:
	Int(int _value);
	~Int();
	virtual bool compare(Op op, const Data* data);
	virtual void* getValue() const;
};

class Float : public Data {
private:
	float value;
public:
	Float(float _value);
	~Float();
	virtual bool compare(Op op, const Data* data);
	virtual void* getValue() const;
};

class Char : public Data {
private:
	char value[256];
public:
	Char(string _value);
	~Char();
	virtual bool compare(Op op, const Data* data);
	virtual void* getValue() const;
};

class Field {//属性定义信息
public:
	string name;//属性名
	int type;//属性数据类型{INT, FLOAT, CHAR};
	int attribute;//属性KeyType{NORMAL, UNIQUE, PRIMARY};
	int length;
	bool hasIndex;//该属性是否含有缩影
	string indexname;

	Field(string _name, int _type, int _attribute = NORMAL, int _length = 4, bool _hasIndex = false, string _indexname = "noindex") :
			name(_name), type(_type), attribute(_attribute), length(_length), hasIndex(_hasIndex), indexname(_indexname) {
		if (type == INT)
			length = LENGTH_OF_INT;
		if (type == FLOAT)
			length = LENGTH_OF_FLOAT;
	}
	virtual ~Field() {
	}
};

class Table {//表定义信息
public:
	friend ostream &operator<<(ostream &os, const Table &t);
	string name;//表名
	int numOfField;//列数
	vector<Field> fields;
	int size;
	int locationOfTable;//table的首位置
	int locationOfData;
	int key;//主键
	Table(string _name, vector<Field> _fields, int _size, int _locationOfTable,
		  int _locationOfData) :
			name(_name), fields(_fields), size(_size), locationOfTable(
			_locationOfTable), locationOfData(_locationOfData) {
		numOfField = fields.size();
		key = getKeyIndex();
	}
	virtual ~Table() {
	}
	int getKeyIndex();//获得主键位置
	int getIndexOf(string name)const;//返回属性在
	bool findField(string _name);
	const Field& getFieldInfo(string _name)const;
	const Field& getFieldInfoAtIndex(size_t index) const;//
	void show();//显示table有关属性的信息

	vector<int> getUniqueIndexs() {
		vector<int> UniqueIndexs;
		for (unsigned i = 0; i < fields.size(); i++)
		{
			const Field &f = fields[i];
			if (f.attribute == KeyType::UNIQUE)
			{
				UniqueIndexs.push_back(i);
			}
		}
		return UniqueIndexs;
	}

	vector<int> getTupleWithIndexs() const{
		vector<int> TupleWithIndexs;
		for (unsigned i = 0; i < fields.size(); i++)
		{
			const Field &f = fields[i];
			if (f.hasIndex == true)
			{
				TupleWithIndexs.push_back(i);
			}
		}
		return TupleWithIndexs;
	}

	bool HasPrim()const {
		for (auto f : fields)
		{
			if (f.attribute == PRIMARY)
				return true;
		}
		return false;
	}
};
#endif /* RECORDMANAGER_DATA_H_ */

