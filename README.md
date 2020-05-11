# pagerank-wikidata
## 作业要求原文

Dataset: `WikiData.txt`

The format of the lines in the file is as follow:  `FromNodeID   ToNodeID`

In this project, you need to report the `Top 100 NodeID` with their PageRank scores. You can choose different parameters, such as the teleport parameter, to compare different results. One result you must report is that when setting the teleport parameter to `0.85`.

In addition to the basic PageRank algorithm, you need to implement the Block-Stripe Update algorithm.

## 开发环境

- 语言：C++
- Windows 可执行文件生成：Visual Studio 2015,  Windows 10 64 位专业版
- 程序运行截图中的系统环境：CentOS 7（阿里云主机）

## 已经实现的功能

- [x] 考虑了 dead ends 和 spider trap 节点的 PageRank 基础算法
- [x] 稀疏矩阵存储优化
- [x] 稀疏矩阵分块计算（Block-Stripe Update algorithm）
- [x] 各网页节点的最终 PageRank 值按照此格式输出：`[NodeID]   [Score]`

## 目录结构

```
pagerank-wikidata
│  LICENSE
│  README.md
│  executable.pdf    # 可执行文件说明
│  report.pdf    # 报告
│  WikiData.txt    # 原始数据集
│
├─BlockPagerank   # 有稀疏矩阵优化和分块计算的PageRank程序
│  │  main.cpp
│  │  Makefile
│  │  mem.h
│  │  pagerank.cpp
│  │  pagerank.h
│  │
│  ├─tempfiles  # 结果文件和中间文件
│  │  │  node_statistics.txt
│  │  │  result_all.txt
│  │  │  result_top100.txt
│  │  │
│  │  ├─blocks   # 存储分块文件
│  │  │
│  │  └─rold   # 存储rold文件
│  │
│  └─Windows executable file    # Windows可执行文件
│          pagerank.exe
│
└─BasicPagerank   # 只有稀疏矩阵优化无分块计算的PageRank程序
    │  main.cpp
    │  Makefile
    │  mem.h
    │  pagerank.cpp
    │  pagerank.h
    │
    ├─tempfiles  # 结果文件和中间文件
    │      matrix.txt
    │      node_statistics.txt
    │      result_all.txt
    │      result_top100.txt
    │      rold.dat
    │
    └─Windows executable file    # Windows可执行文件
            pagerank.exe
```

## 致谢

具体如何分块的思路有部分参考了一些大佬的思路（因为自己想的思路确实没有这么好），感谢大佬！