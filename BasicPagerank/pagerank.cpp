#include "pagerank.h"
#ifdef MEM_TEST
#include "mem.h"
#endif // MEM_TEST

#ifdef TIME_TEST
clock_t start_time, end_time;
double total_time, time_;
#endif // TIME_TEST

Pagerank::Pagerank()
{
	this->beta = 0.85;
	this->epsilon = 1e-9;
	this->maxIterCount = 1000;
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

	ofstream outfile;
	map<int, set<int>>::iterator iter1 = allEdges.begin();
	outfile.open("matrix.txt");
	while (iter1 != allEdges.end())	 // 遍历allEdges的每个条目
	{
		// 将矩阵信息写入到文件中，注意这里使用的节点序号是上面重新编的序号
		outfile << idMap[iter1->first] << "  " << iter1->second.size() << "  ";
		set<int>::iterator iter2 = iter1->second.begin();
		while (iter2 != iter1->second.end())
		{
			outfile << idMap[*iter2] << "  ";
			iter2++;
		}
		outfile << endl;
		iter1++;
	}
	outfile.close();
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
	allEdges.clear();
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
	vector<double> rnew;  // 计算过程中对应的rnew
	double init = 1.0 / (double)allNodeCount;
	fstream rold; // rold对应的句柄
	ifstream readMatrix; // 读取稀疏矩阵的句柄

	// 初始化向量rold和rnew
	rold.open("rold.dat", ios::in | ios::out | ios::binary | ios::trunc);
	rold.seekg(0, ios::beg);
	rold.seekp(0, ios::beg);
	for (int i = 0; i < allNodeCount; i++)
	{
		rold.write((char*)&init, sizeof(double));
	}

	while (realIterCount < maxIterCount)
	{
		cout << "Iteration: " << realIterCount << endl;
		double leaked = 0.0; // 用于计算leaked pagerank
		double temprank = 0.0;
		rnew.resize(allNodeCount, 0.0);
		readMatrix.open("matrix.txt");
		int src = 0, degree = 0;
		while (readMatrix >> src >> degree)
		{
			int dst = 0, size = degree;
			rold.seekg(src*sizeof(double), ios::beg);
			rold.read((char*)&temprank, sizeof(double));
			while (size)
			{
				readMatrix >> dst;
				rnew[dst] += beta*temprank / (double)degree;
				size--;
			}
		}
		for (int j = 0; j < rnew.size(); j++)
		{
			leaked += rnew[j];
		}
		readMatrix.close();

		// 计算完之后将leaked pagerank加回来
#ifdef DEBUG
		cout << "Calculating leaked pagerank......" << endl;
#endif // DEBUG
		leaked = (1.0 - leaked) / (double)allNodeCount;
		for (int j = 0; j < rnew.size(); j++)
		{
			rnew[j] += leaked;
		}

		// 计算convergence
#ifdef DEBUG
		cout << "Calculating convergence......" << endl;
#endif // DEBUG
		rold.seekg(0, ios::beg);
		dvalue = 0.0;
		int i = 0;
		while (true)
		{
			double old;
			rold.read((char*)&old, sizeof(double));
			if (rold.eof())
				break;
			dvalue += fabs(rnew[i] - old);
			i++;
		}

		// 更新rold
		rold.close();
		rold.open("rold.dat", ios::in | ios::out | ios::binary | ios::trunc);
		rold.seekp(0, ios::beg);
		rold.seekg(0, ios::beg);
		for (int j = 0; j < rnew.size(); j++)
		{
			rold.write((char*)&rnew[j], sizeof(double));
		}

		realIterCount++;
		rnew.clear();

		// 判断是否收敛
		cout << "Convergence: " << dvalue << endl;
		if (dvalue <= epsilon)
			break;
	}
	rold.close();
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
	vector<pair<double, int>> result(allNodeCount, pair<double, int>(0.0, 0));
	ifstream readRank("rold.dat", ios::in | ios::binary);	// 从rold导入Pagerank值
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
		if (count<100)
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