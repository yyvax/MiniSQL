#pragma once
#include "datatype.h"
#include "string"
const size_t BLOCK_LEN = 4096;// the size of one block��4KB
const size_t MAX_FILE_ACTIVE = 5;// limit the active files in the buffer
const size_t MAX_BLOCK = 40; // the max number of the blocks



using namespace std;



static string DATABASE_NAME;

string getDatabaseName();//�������ݿ�����

void mem_init();//��ʼ���ڴ��е�block��Ϣ���ļ���Ϣ

bool useDatabase(string DB_Name);//�ж����ݿ��ļ��Ƿ����

string get_file_path(string table_name, int file_type);//�ҵ�����table��·��path


string get_directory_path(string databaseName);//�ҵ������ݿ��ļ���·��


void replace(fileInfo *m_fileInfo, blockInfo *m_blockInfo);//��lock������������ĳ�飬�������滻��ȥ
//ʵ�ְ�ָ��m_blockInfo ��ָ��Ŀ������ļ�ͷָ��m_fileInfo��ָ��Ŀ�����Ľ�β��
//ͬʱ��m_blockInfo��ָ��Ŀ��fileָ��ָ��m_fileInfo��

blockInfo *get_file_block(string Table_Name, int fileType, int blockNum);//ʵ�ֻ��������滻�㷨�����滻��
//�����ļ������ļ����Ͳ��Ҹ��ļ��Ƿ����ڴ棬
//����ǣ������ļ��Ŀ�ţ����ڴ��в�ѯ�ÿ飬
//��ÿ��Ѿ����ڴ棬���ظÿ��ָ�룬
//����ÿ�û�����ڴ棬�ж������������Ƿ��п���Ŀ飬
//���д����ҵ�һ�飨�����ͷ�������ÿ��blockNUM����Ϊ����blockNUM��
//���û�У��ж������ڴ��еĿ����Ƿ��Ѿ��ﵽ������������
//���û�У�Ϊ������·���һ�飬�����ӵ���Ӧ���ļ�ͷ��ָ������Ľ�β��
//����Ѵﵽ��ʹ��LRU�㷨���ҵ�һ���滻�飬���ո�����Ҫ����г�ʼ�������������ӵ�ָ���ļ�������Ľ�β��
//����ļ�û�����ڴ棬����get_file_info(string fileName, int m_fileType)��Ϊ�ļ�����һ��ͷָ�롣
//Ȼ���blockHandle���·����ʹ��LRU�㷨�ҵ�һ���滻�飨�������������ƣ������ÿ鰴Ҫ���ʼ����



void closeDatabase();//�ر����ݿ�ͨ���رմ�ȡ�����ݿ���й��ļ�
 //����closeFile(filename,fileType,m_flag),����ر��ļ���



void closeFile(size_t num);//�رյ�num���ļ�, dirty bit��¼��״̬���Ƿ��޸Ĺ���
						   //�����ļ������ļ����ͣ������ڴ��Ƿ�������ļ������û�к�������
						   //����ҵ����ļ�����1�����ڿ������е�ÿ���飬����dirtyλΪ1������writeBlock��m_file_num����ָ�룩��
						   //���ÿ������д�ش��̣����dirtyλΪ0����������Ȼ������������ӵ����������С��ͷ��ļ�ͷ��struct ��



void writeBlock(size_t m_file_num, blockInfo *block);//��block�е�����д����m_file_num���ļ�
//���ݿ��д��										 //��blockָ����ָ��Ŀ������д�ش��̡�


fileInfo *get_file_info(string fileName, int m_fileType);//�������ļ��ڵ㲢���������Ϣ
//���Ȳ��Ҹ��ļ��Ƿ��Ѿ����ڴ��д��ڣ�����ǣ����ظ��ļ�ͷ��
//��������ڣ��ж��ڴ������е��ļ����Ƿ���������ļ�����
//������ǣ�Ϊ�����һ���ļ�ͷ��struct��������Ҫ����г�ʼ�����ļ�������һ��
//����get_tablei_info(DB_Name,fileName,fileinfo->recordAmount,fileinfo->recordLength,fileinfo->freeNum)
//��get_index_info(DB_Name,fileName,fileinfo->recordAmount,fileinfo->recordLength,fileinfo->freeNum)�Ը��ļ�ͷ���г�ʼ���������ظ��ļ�ͷ��
//������ǣ�ѡȡ�ļ�ͷ����ĵ�һ���ļ�ͷ����Ϊ�滻����ȶ�ѡ�����ļ����йر��ļ�������ʹ���ڴ��е��ļ���Ŀ����һ��������Ϊ������������һ���ļ�ͷ��
//����get_tablei_info(DB_Name,fileName,fileinfo->recordAmount,fileinfo->recordLength,fileinfo->freeNum)
//��get_index_info(DB_Name,fileName,fileinfo->recordAmount,fileinfo->recordLength,fileinfo->freeNum)�Ը��ļ�ͷ���г�ʼ�������ظ��ļ�ͷ��

blockInfo *readBlock(string m_fileName, int m_blockNum, int m_fileType);//���ݿ��д��
//�Ӵ����ж�ȡ�ÿ�����ݵ��ڴ棬���ظÿ顣

blockInfo *get_new_block(const string& file_name, int fileType, int blockNum);

blockInfo *findBlock();
//����ʵ�ִ����ѷ�����ڴ浥Ԫ�л�ȡ�����滻���ڴ�顣
//�����ж���������blockHandle���Ƿ��п��õ��ڴ�飬����д������ͷȡһ�鷵�ء�
//��û�п��ÿ飬ʹ��LRU�㷨�ҵ�û��ʹ�õ�ʱ����Ŀ飨itime��󣩣���������λ��ȡ����itime���㣻�����ÿ鷵�ء�

size_t create_file(string fileName, int fileType);//�����ļ�



size_t createDatabase(string databaseName);
//�������ݿ�,�ȼ���Ƿ���ڸ����ݿ⣬�������򲻴������׳���Ӧ�쳣



size_t delete_file(string fileName, int fileType);//ɾ���ļ�
size_t deleteDatabase(string databaseName);//ɾ�����ݿ�



void show(blockInfo *);//���block����Ϣ
void show(fileInfo *);//����ļ�����Ϣ