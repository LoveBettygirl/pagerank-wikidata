#include "pagerank.h"
#ifdef MEM_TEST
extern size_t getPeakRSS();
#endif // MEM_TEST
#ifdef TIME_TEST
extern double total_time;
#endif // TIME_TEST

Pagerank p;
char *srcFile = "WikiData.txt";

void showUsage(char *argv)
{
	cout << "Usage: " << argv << " [-b beta] [-e epsilon] "
		<< "[-t max_iter_count] [-i input_file]" << endl;
	cout << "Default values: " << endl;
	cout << "beta = 0.85" << endl;
	cout << "epsilon = 1e-9" << endl;
	cout << "max iteration count = 1000" << endl;
	cout << "input file path: WikiData.txt" << endl;
	exit(1);
}

void parse_args(int argc, char *argv[])
{
	if (argc == 1)	// 不指定任何命令行参数则采用默认参数设置
		return;
	bool setSrcfile = false;
	for (int i = 1; i < argc; )
	{
		if (strcmp(argv[i], "-b") == 0)	// beta参数值设置
		{
			double temp;
			if (i + 1 < argc && sscanf(argv[i + 1], "%lf", &temp) > 0)
			{
				p.setBeta(temp);
				i += 2;
			}
			else
			{
				showUsage(argv[0]);
			}
		}
		else if (strcmp(argv[i], "-e") == 0) // epsilon参数值设置
		{
			double temp;
			if (i + 1 < argc && sscanf(argv[i + 1], "%lf", &temp) > 0)
			{
				p.setEpsilon(temp);
				i += 2;
			}
			else
			{
				showUsage(argv[0]);
			}
		}
		else if (strcmp(argv[i], "-t") == 0) // 最大迭代次数值设置
		{
			int temp;
			if (i + 1 < argc && sscanf(argv[i + 1], "%d", &temp) > 0)
			{
				p.setMaxIterCount(temp);
				i += 2;
			}
			else
			{
				showUsage(argv[0]);
			}
		}
		else if (argv[i][0] == '-')
		{
			showUsage(argv[0]);
		}
		else // 输入文件设置
		{
			if (setSrcfile)
				showUsage(argv[0]);
			srcFile = argv[i];
			setSrcfile = true;
			i++;
		}
	}
}


int main(int argc, char *argv[])
{
	parse_args(argc, argv);
	p.loadMatrixFromFile(srcFile);
	p.calculate();
	p.writeResultIntoFile();
#ifdef TIME_TEST
	cout << "Total time cost: " << total_time << "s" << endl;
#endif // TIME_TEST
#ifdef MEM_TEST
	double peakSize = (double)getPeakRSS() / 1024 / 1024;
	cout << "Total memory cost: " << peakSize << " MB" << endl;
#endif // MEM_TEST
	return 0;
}