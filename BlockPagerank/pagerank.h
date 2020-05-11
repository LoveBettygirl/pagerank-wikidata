#pragma once
#include <map>
#include <set>
#include <ctime>
#include <vector>
#include <string>
#include <cstdio>
#include <cassert>
#include <climits>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <algorithm>
using namespace std;

//#define DEBUG
#define STAT
#define MEM_TEST
#define TIME_TEST

class Pagerank
{
private:
	/* Pagerank参数 */
	double beta; // teleport parameter 
	double epsilon;	 // 收敛条件系数
	int maxBlockSize; // 一个分块最多有多少个目的节点
	int maxIterCount; // 如果源数据导致难以迭代至收敛，最大的迭代次数

	/* 临时使用变量 */
	int maxNodeID, minNodeID;  // 出现的最大和最小的节点ID
	int allNodeCount;  // 统计出现的节点ID总数
	int realIterCount; // 实际迭代次数
	map<int, int> idMap; // 储存实际节点ID与节点在程序中的编号的映射
	int blockCount; // 实际分块个数
	bool special; // 判断allNodeCount能不能整除maxBlockSize
public:
	Pagerank();
	~Pagerank();
	void setBeta(double beta);
	void setEpsilon(double epsilon);
	void setMaxBlockSize(double maxBlockSize);
	void setMaxIterCount(double maxIterCount);
	void loadMatrixFromFile(char *filename); // 从文件中读取矩阵信息
	void calculate(); // 计算Pagerank值
	void writeResultIntoFile();	// 将计算结果写入到文件
};