#include "pagerank.h"
#ifdef MEM_TEST
#include "mem.h"
#endif // MEM_TEST
// 文件操作
#ifdef _WIN32
#include <io.h>
#include <direct.h> 
#else
#include <unistd.h>
#include <sys/stat.h>
#endif

#ifdef TIME_TEST
clock_t start_time, end_time;
double total_time, time_;
#endif // TIME_TEST

Pagerank::Pagerank()
{
	this->beta = 0.85;
	this->epsilon = 1e-9;
	this->maxBlockSize = 100;
	this->maxIterCount = 1000;
#ifdef _WIN32
	if (_access("blocks", 0) == -1)
		_mkdir("blocks");
	else
		system("del blocks /Q");
	if (_access("rold", 0) == -1)
		_mkdir("rold");
	else
		system("del rold /Q");
#else
	if (access("blocks", 0) == -1)
		system("mkdir -p blocks");
	else
		system("rm -rf blocks/*");
	if (access("rold", 0) == -1)
		system("mkdir -p rold");
	else
		system("rm -rf rold/*");
#endif
}

Pagerank::~Pagerank()
{
	idMap.clear();
}

void Pagerank::setBeta(double beta)
{
	this->beta = beta;
}

void Pagerank::setEpsilon(double epsilon)
{
	this->epsilon = epsilon;
}
void Pagerank::setMaxBlockSize(double maxBlockSize)
{
	this->maxBlockSize = maxBlockSize;
}

void Pagerank::setMaxIterCount(double maxIterCount)
{
	this->maxIterCount = maxIterCount;
}

void Pagerank::loadMatrixFromFile(char *filename)
{
	assert(filename);
	ifstream infile(filename);
	if (!infile.is_open())
	{
		perror(filename);
		exit(1);
	}
	cout << "Loading matrix from file " << filename << "......";
#ifdef TIME_TEST
	start_time = clock();
#endif // TIME_TEST
	map<int, set<int>> allEdges;
	int fromNodeID, toNodeID;
	maxNodeID = 0;
	minNodeID = INT_MAX;
	allNodeCount = 0;
	bool idStat[10000] = { 0 };
	while (infile >> fromNodeID >> toNodeID)
	{
		allEdges[fromNodeID].insert(toNodeID);
		idStat[fromNodeID] = true;
		idStat[toNodeID] = true;
		if (max(maxNodeID, fromNodeID) == fromNodeID)
		{
			maxNodeID = fromNodeID;
		}
		if (max(maxNodeID, toNodeID) == toNodeID)
		{
			maxNodeID = toNodeID;
		}
		if (min(minNodeID, fromNodeID) == fromNodeID)
		{
			minNodeID = fromNodeID;
		}
		if (min(minNodeID, toNodeID) == toNodeID)
		{
			minNodeID = toNodeID;
		}
	}
	infile.close();

	// 按照节点ID的大小顺序为每个节点重新编号，建立节点ID和序号之间的映射关系
	for (int i = 0; i < 10000; i++)
	{
		if (idStat[i])
		{
			idMap[i] = allNodeCount;
			allNodeCount++;
		}
	}
#ifdef TIME_TEST
	end_time = clock();
	time_ = (double)(end_time - start_time) / CLOCKS_PER_SEC;
	total_time += time_;
	cout << "done." << endl;
	cout << "Time cost in loading matrix: " << time_ << "s" << endl;
#else
	cout << "done." << endl;
#endif // TIME_TEST

	// 节点信息统计，可选
	ofstream outfile;
#ifdef STAT
	cout << "Generating node statistics file......";
#ifdef TIME_TEST
	start_time = clock();
#endif // TIME_TEST
	outfile.open("node_statistics.txt");
	outfile << "Source node count: " << allEdges.size() << endl;
	outfile << "All node count: " << allNodeCount << endl;
	outfile << "Max node ID:  " << maxNodeID << endl;
	outfile << "Min node ID:  " << minNodeID << endl;
	for (int i = 0; i < 10000; i++)
	{
		outfile << "[ id: " << i << "  ";
		if (!idStat[i])
		{
			outfile << "not used";
		}
		else
		{
			outfile << "used" << "  ";
			if (allEdges.find(i) == allEdges.end())
			{
				outfile << "degree: 0";
			}
			else
			{
				outfile << "degree: " << allEdges[i].size() << "  outNode: ";
				set<int>::iterator iter = allEdges[i].begin();
				while (iter != allEdges[i].end())
				{
					outfile << *iter << ", ";
					iter++;
				}
			}
		}
		outfile << " ]" << endl;
	}
	outfile.close();
#ifdef TIME_TEST
	end_time = clock();
	time_ = (double)(end_time - start_time) / CLOCKS_PER_SEC;
	total_time += time_;
	cout << "done." << endl;
	cout << "Time cost in generating statistics file: " << time_ << "s" << endl;
#else
	cout << "done." << endl;
#endif // TIME_TEST
#endif // STAT

	cout << "Blocking matrix......";
#ifdef TIME_TEST
	start_time = clock();
#endif // TIME_TEST
	blockCount = allNodeCount / maxBlockSize; // 计算分块个数
	special = allNodeCount % maxBlockSize != 0; // 判断能不能整除
	if (special) // 校正
	{
		blockCount++;
	}
	vector<ofstream> makeBlocks(blockCount);
	for (int i = 0; i < blockCount; i++) // 提前打开分块文件，避免I/O过于密集，减少分块时间
	{
		char tempstr[50] = { 0 };
		sprintf(tempstr, "blocks/block%d.txt", i);
		makeBlocks[i].open(tempstr, ios::app);
	}
	map<int, set<int>>::iterator iter1 = allEdges.begin();
	map<int, set<int>> edgesInBlock;
	while (iter1 != allEdges.end())	 // 遍历allEdges的每个条目
	{
		// 提取出allEdges该源节点条目所在的块信息，注意这里使用的节点序号是上面重新编的序号
		set<int>::iterator iter2 = iter1->second.begin();
		while (iter2 != iter1->second.end())
		{
			int temp = idMap[*iter2];
			edgesInBlock[temp / maxBlockSize].insert(temp);
			iter2++;
		}

		// 将分块信息写入到文件
		map<int, set<int>>::iterator iter3 = edgesInBlock.begin();
		while (iter3 != edgesInBlock.end())
		{
			int blockID = iter3->first;
			// 分块文件每一行的格式为：
			// 源节点ID  源节点出度  源节点在这个块的出度数  目的节点1ID  目的节点2ID ...... 
			makeBlocks[blockID] << idMap[iter1->first] << "  " << iter1->second.size() << "  ";
			makeBlocks[blockID] << iter3->second.size() << "  ";
			set<int>::iterator iter4 = iter3->second.begin();
			while (iter4 != iter3->second.end())
			{
				makeBlocks[blockID] << *iter4 << "  ";
				iter4++;
			}
			makeBlocks[blockID] << endl;
			iter3++;
		}
		edgesInBlock.clear();
		iter1++;
	}
	allEdges.clear();
	edgesInBlock.clear();
	for (int i = 0; i < blockCount; i++)
	{
		makeBlocks[i].close();
	}
	makeBlocks.clear();
#ifdef TIME_TEST
	end_time = clock();
	time_ = (double)(end_time - start_time) / CLOCKS_PER_SEC;
	total_time += time_;
	cout << "done." << endl;
	cout << "Time cost in blocking matrix: " << time_ << "s" << endl;
#else
	cout << "done." << endl;
#endif // TIME_TEST
}

void Pagerank::calculate()
{
	cout << "Calculating pagerank......" << endl;
#ifdef TIME_TEST
	start_time = clock();
#endif // TIME_TEST
	/* 初始化变量 */
	realIterCount = 0; // 迭代次数
	double dvalue = 0.0; // convergence
	const char *curr_rold = "rold/rold%d.dat"; // 每次迭代的rold都从单独的文件中存取，由句柄rold和rnew进行操作
	const char *currBlock = "blocks/block%d.txt";  // 分块文件
	char tempstr[50] = { 0 }; // 字符串缓冲区
	vector<double> rnew_;  // 计算过程中，单独的分块对应的rnew
	double init = 1.0 / (double)allNodeCount;
	fstream rold; // rold对应的句柄
	fstream rnew; // 本次迭代的rnew对应的句柄
	ifstream readBlock;	 // 读取分块矩阵的句柄

	// 初始化向量rold和rnew
	sprintf(tempstr, curr_rold, realIterCount);
	rold.open(tempstr, ios::in | ios::out | ios::binary | ios::trunc);
	sprintf(tempstr, curr_rold, realIterCount + 1);
	rnew.open(tempstr, ios::in | ios::out | ios::binary | ios::trunc);
	rold.seekp(0, ios::beg);
	rnew.seekp(0, ios::beg);
	rold.seekg(0, ios::beg);
	rnew.seekg(0, ios::beg);
	for (int i = 0; i < allNodeCount; i++)
	{
		rold.write((char*)&init, sizeof(double));
	}

	while (realIterCount < maxIterCount)
	{
		cout << "Iteration: " << realIterCount << endl;
		double leaked = 0.0; // 用于计算leaked pagerank
		double temprank = 0.0;
#ifdef DEBUG
		cout << "Calculating blocks......" << endl;
#endif // DEBUG
		for (int i = 0; i < blockCount; i++) // 分块计算
		{
			sprintf(tempstr, currBlock, i);
			readBlock.open(tempstr);
			int src = 0, degree = 0, size = 0;
			int rnew_size = special && i == blockCount - 1 ? allNodeCount%maxBlockSize : maxBlockSize;
			rnew_.resize(rnew_size, 0.0); // 当前块的rnew初始化为0
			while (readBlock >> src >> degree >> size)
			{
				int dst = 0;
				rold.seekg(src*sizeof(double), ios::beg);
				rold.read((char*)&temprank, sizeof(double)); // 从文件中读入rold值
				while (size)
				{
					readBlock >> dst;
					rnew_[dst % maxBlockSize] += beta*temprank / (double)degree;
					size--;
				}
			}
			for (int j = 0; j < rnew_.size(); j++)
			{
				leaked += rnew_[j];
				rnew.write((char*)&rnew_[j], sizeof(double));
			}
			rnew_.clear();
			readBlock.close();
		}

		// 所有分块计算完之后将leaked pagerank加回来
#ifdef DEBUG
		cout << "Calculating leaked pagerank......" << endl;
#endif // DEBUG
		leaked = (1.0 - leaked) / (double)allNodeCount;
		rnew.seekg(0, ios::beg);
		rnew.seekp(0, ios::beg);
		for (int j = 0; j < allNodeCount; j++)
		{
			double curr = 0;
			rnew.seekg(j*sizeof(double), ios::beg);
			rnew.read((char*)&curr, sizeof(double));
			curr += leaked;
			rnew.seekp(j*sizeof(double), ios::beg);
			rnew.write((char*)&curr, sizeof(double));
		}

		// 计算convergence
#ifdef DEBUG
		cout << "Calculating convergence......" << endl;
#endif // DEBUG
		rnew.seekg(0, ios::beg);
		rold.seekg(0, ios::beg);
		dvalue = 0.0;
		while (true)
		{
			double old, new_;
			rnew.read((char*)&new_, sizeof(double));
			rold.read((char*)&old, sizeof(double));
			if (rnew.eof() || rold.eof())
				break;
			dvalue += fabs(new_ - old);
		}
		rnew.close();
		rold.close();
		realIterCount++;

		// 判断是否收敛
		cout << "Convergence: " << dvalue << endl;
		if (dvalue <= epsilon)
			break;

		// 重置rold和rnew
		sprintf(tempstr, curr_rold, realIterCount);
		rold.open(tempstr, ios::in | ios::binary);
		sprintf(tempstr, curr_rold, realIterCount + 1);
		rnew.open(tempstr, ios::in | ios::out | ios::binary | ios::trunc);
		rnew.seekp(0, ios::beg);
		rnew.seekg(0, ios::beg);
		rold.seekg(0, ios::beg);
	}
	if (rold.is_open())
		rold.close();
	if (rnew.is_open())
		rnew.close();
#ifdef TIME_TEST
	end_time = clock();
	time_ = (double)(end_time - start_time) / CLOCKS_PER_SEC;
	total_time += time_;
	cout << "Calculate pagerank complete." << endl;
	cout << "Time cost in calculate pagerank: " << time_ << "s" << endl;
#else
	cout << "Calculate pagerank complete." << endl;
#endif // TIME_TEST
	cout << "Real iteration count: " << realIterCount << endl;
#ifdef MEM_TEST
	double currentSize = (double)getCurrentRSS() / 1024 / 1024;
	cout << "Calculate memory cost: " << currentSize << " MB" << endl;
#endif // MEM_TEST
}

void Pagerank::writeResultIntoFile()
{
	ofstream outfile1("result_all.txt");
	ofstream outfile2("result_top100.txt");
	cout << "Writing result into file result_all.txt and result_top100.txt......";
#ifdef TIME_TEST
	start_time = clock();
#endif // TIME_TEST
	char tempstr[50] = { 0 };
	vector<pair<double, int>> result(allNodeCount, pair<double, int>(0.0, 0));
	sprintf(tempstr, "rold/rold%d.dat", realIterCount);
	ifstream readRank(tempstr, ios::in | ios::binary);	// 从最后一个rold导入Pagerank值
	double temp;
	readRank.seekg(0, ios::beg);
	int i = 0;
	while (true)
	{
		readRank.read((char*)&temp, sizeof(double));
		if (readRank.eof())
			break;
		result[i].first = temp;
		// 根据重新编的号查找实际节点ID
		map<int, int>::iterator idMap_inv = find_if(idMap.begin(), idMap.end(),
			[i](const map<int, int>::value_type item)
		{
			return item.second == i;
		});
		if (idMap_inv != idMap.end())
		{
			result[i].second = (*idMap_inv).first;
		}
		i++;
	}
	readRank.close();

	sort(result.rbegin(), result.rend()); // 对结果由大到小进行排序，sort默认对first进行排序

	// 输出结果
	// 格式为：[NodeID]   [Score]
	double sum = 0.0;
	int count = 0;
	for (int i = 0; i < result.size(); i++)
	{
		outfile1 << "[" << result[i].second << "]   [" << result[i].first << "]" << endl;
		if (count < 100)
			outfile2 << "[" << result[i].second << "]   [" << result[i].first << "]" << endl;
		sum += result[i].first;
		count++;
	}
	outfile1.close();
	outfile2.close();
	result.clear();
#ifdef TIME_TEST
	end_time = clock();
	time_ = (double)(end_time - start_time) / CLOCKS_PER_SEC;
	total_time += time_;
	cout << "done." << endl;
	cout << "The sum of pagerank: " << sum << endl;
	cout << "Time cost in writing result: " << time_ << "s" << endl;
#else
	cout << "done." << endl;
	cout << "The sum of pagerank: " << sum << endl;
#endif // TIME_TEST
}