#include <stdio.h>
#include <iostream>
#include <unordered_map>
#include <stack>
#include <algorithm>

#include "AETGcore.h"

#include <QDebug>

using namespace std;

// 对comb进行重排，comb[0:k-1]即生成的组合
bool next_comb(int* comb, int n, int k)
{
	int i = k - 1;
	const int e = n - k;
	do
		comb[i]++;
	while (comb[i] > e + i && i--);
	if (comb[0] > e)
		return false;
	while (++i < k)
		comb[i] = comb[i - 1] + 1;
	return true;
}

// 用于产生随机序列的结构及函数
struct RandSeq {
	RandSeq* next;
};
void resetRandSeq(RandSeq* randSeq, int n)
{
	for (int i = 0; i < n - 1; i++)
		randSeq[i].next = &randSeq[i + 1];
	randSeq[n - 1].next = &randSeq[0];
}
int next_rand_idx(RandSeq* randSeq, int n, int start)
{
	RandSeq* startPtr = &randSeq[start], * p;
	int movN = floor(1.0f * n * rand() / RAND_MAX), rt; // 产生[0:n-1]的整数

	p = startPtr->next;
	for (int i = 0; i < movN - 1; i++) // 距离start movN+1的数就是下一个下标
		p = p->next;

	if (movN == 0) // 特殊情况，star下一个数就是下一个下标，存下下标，跳过该下标
	{
		rt = p - randSeq;
		startPtr->next = startPtr->next->next;
	}
	else // 存下下标，将该下标跳过
	{
		rt = p->next - randSeq;
		p->next = p->next->next;
	}

	return rt;
}

// T-way覆盖结构及函数
int cmpINT(const void* a, const void* b) {
	return *((int*)a) < *((int*)b) ? 1 : 0;
}
struct Cover
{
	int* dat;
	int T, size;
	Cover(int T) : T(T), size(0) {
		dat = new int[T];
	}
	Cover(const Cover& right) {
		T = right.T;
		size = right.size;
		dat = new int[T];
		memcpy(dat, right.dat, sizeof(int) * T);
	}
	~Cover() {
		delete[] dat;
	}
	int idxOf(int v) const {
		for (int i = 0; i < T; i++)
			if (dat[i] == v)return i;
		return -1;
	}
	void insert(int v) {
		dat[size++] = v;

		// Cover满时排序
		if (size == T)
			qsort(dat, T, sizeof(int), cmpINT);
	}
	void clear() {
		size = 0;
	}
};
struct CoverHash {
	size_t operator()(const Cover& c) const {
		int sum = 0, i;
		for (i = 0; i < c.size; i++)
			sum += c.dat[i];
		return sum;
	}
};
struct CoverEq {
	int operator()(const Cover& a, const Cover& b) const {
		int flag = 1, i;
		for (i = 0; i < a.size; i++)
			if (a.dat[i] != b.dat[i]) {
				flag = 0; break;
			}
		return flag;
	}
};

static int* seq; // 表示K个测试项的序列
static int* maxAppear; // 表示每个取值的出现次数
static RandSeq* randSeq; // 用于产生随机下标序列
static int* realSeq; // 用于恢复随机打乱后的下标序列顺序

void genAllCov(unordered_map<Cover, int, CoverHash, CoverEq>& ALL, int T, int K, const int* V, int** val)
{
	int i;
	Cover cov(T);
	for (i = 0; i < K; i++)
		seq[i] = i;

	// 生成总覆盖ALL
	// 循环1：从seq的K项中选出T项，进行组合（Comb(T,K)），组成序列 seq[0:T-1]
	//   循环2：从 seq[0:T-1] 对应测试项的取值 val[seq[i]] (i属于[0,T-1]) 中各选出一个，进行组合，
	//   组成一个T项组合entry，将entry加入ALL
	do {
		int t; // 当前遍历的树高
		stack<int> idxs; // 记录当前正在遍历第几棵子树的栈

		// 将根节点的长子直系全部入栈
		// {
		for (t = 0; t < T; t++)
			idxs.push(0);
		// }

		// 将循环2的过程视为深度遍历树
		while (!idxs.empty())
		{
			if (t == T) // 处于叶子节点，可以添加entry入ALL
			{
				cov.clear();
				stack<int> tmpidxs(idxs);
				// 取得当前遍历路径（从根节点到叶节点）所有值
				// {
				for (i = T - 1; i >= 0; i--)
				{
					cov.insert(val[seq[i]][tmpidxs.top()]);
					tmpidxs.pop();
				}
				// }
				ALL[Cover(cov)] = 0; // cov加入ALL

				if (idxs.top() < V[seq[t - 1]] - 1) // 当前叶子节点还有弟，继续遍历弟
				{
					int tmp = idxs.top();
					// 移动到弟节点
					// {
					idxs.pop();
					tmp++;
					idxs.push(tmp);
					// }
				}
				else // 当前叶子节点无弟，回退到父节点
				{
					idxs.pop();
					t--; // 层高减少
				}
			}
			else // 处于非叶节点
			{
				if (idxs.top() < V[seq[t - 1]] - 1) // 当前非叶节点有弟
				{
					int tmp = idxs.top();
					// 移动到弟节点
					// {
					idxs.pop();
					tmp++;
					idxs.push(tmp);
					// }
					// 将弟节点的长子直系全部入栈
					// {
					for (i = 0; i <= T - t; i++)
					{
						idxs.push(0);
						t++; // 层高增加
					}
					// }
				}
				else // 当前非叶节点无弟，回退到父节点
				{
					idxs.pop();
					t--; // 层高减少
				}
			}
		}
	} while (next_comb(seq, K, T));
}

void coverWithRow(unordered_map<Cover, int, CoverHash, CoverEq>& ALL, int* uncoverNum,
	const vector<int>& ROW,
	int T, int K)
{
	int i;
	for (i = 0; i < K; i++)
		seq[i] = i;

	Cover cov(T);
	do {
		cov.clear();
		for (i = 0; i < T; i++)
			cov.insert(ROW[seq[i]]);
		if (ALL[cov] == 0) 
		{
			ALL[cov] = 1; (*uncoverNum)--;
		}
	} while (next_comb(seq, K, T));
}

void genFirstRow(unordered_map<Cover, int, CoverHash, CoverEq>& ALL, int* uncoverNum, 
	vector<vector<int>>& TC,
	int T, int K, int** val)
{
	vector<int> firstRow;
	for (int i = 0; i < K; i++)
		firstRow.push_back(val[i][0]);

	int oriUncoverNum = *uncoverNum;
	coverWithRow(ALL, uncoverNum, firstRow, T, K);

	TC.push_back(firstRow);
	TC[TC.size() - 1].push_back(oriUncoverNum - *uncoverNum);

	qDebug() << *uncoverNum << "left" << oriUncoverNum - *uncoverNum << "minused";
}

void countAppearInUncover(const unordered_map<Cover, int, CoverHash, CoverEq>& ALL,
	int T, int K, const int* V, int** val)
{
	int i, j;
	for (i = 0; i < K; i++)
		for (j = 0; j < V[i]; j++)
			maxAppear[val[i][j]] = 0;
	for (auto itr = ALL.begin(); itr != ALL.end(); itr++)
		if (itr->second == 0)
			for (i = 0; i < T; i++)
				maxAppear[itr->first.dat[i]]++;
}

int genNextElem(unordered_map<Cover, int, CoverHash, CoverEq>& ALL,
	vector<int>& ROW,
	int T, int K, const int* V, int** val, int idx)
{
	if (K < T)
	{
		int coverNum, maxOff = 0, maxCoverNum = 0, flag;
		ROW.push_back(val[idx][maxOff]);
		for (int j = 0; j < V[idx]; j++)
		{
			ROW[ROW.size() - 1] = val[idx][j];
			coverNum = 0;
			for (auto itr = ALL.begin(); itr != ALL.end(); itr++)
			{
				flag = 1;
				if (itr->second == 0)
					for (auto itr2 = ALL.begin(); itr2 != ALL.end(); itr2++)
						if (itr->first.idxOf(val[idx][j]) == -1)
						{
							flag = 0; break;
						}
				if (flag)coverNum++;
			}
			if (coverNum > maxCoverNum)
			{
				maxOff = j;
				maxCoverNum = coverNum;
			}
		}
		ROW[ROW.size() - 1] = val[idx][maxOff];
		return 0;
	}
	else
	{
		int i, coverNum, maxOff = 0, maxCoverNum = 0;
		Cover cov(T);
		ROW.push_back(val[idx][0]);

		for (int j = 0; j < V[idx]; j++)
		{
			for (i = 0; i < K; i++)
				seq[i] = i;
			
			ROW[ROW.size() - 1] = val[idx][j];
			coverNum = 0;
			do {
				cov.clear();
				for (i = 0; i < T; i++)
					cov.insert(ROW[seq[i]]);
				if (ALL[cov] == 0)
					coverNum++;
			} while (next_comb(seq, K, T));

			if (coverNum > maxCoverNum)
			{
				maxCoverNum = coverNum;
				maxOff = j;
			}
		}
		ROW[ROW.size() - 1] = val[idx][maxOff];
		return maxCoverNum;
	}
}

void genNextRow(unordered_map<Cover, int, CoverHash, CoverEq>& ALL, int* uncoverNum,
	vector<vector<int>>& TC,
	int T, int K, const int* V, int** val, int M)
{
	int maxIdx = 0, maxOff = 0, maxV = 0, start;
	vector<int> nextRow(K + 1);
	
	int i, j;
	countAppearInUncover(ALL, T, K, V, val);
	for (i = 0; i < K; i++)
		for (j = 0; j < V[i]; j++)
			if (maxAppear[val[i][j]] > maxAppear[val[maxIdx][maxOff]])
			{
				maxIdx = i; maxOff = j;
			}
	TC.push_back(nextRow); // 将队尾作为临时储存变量，储存最大覆盖的row

	int k, coverNum = 0;
	maxV = 0;
	for (int m = 0; m < M; m++)
	{
		nextRow.clear();
		nextRow.push_back(val[maxIdx][maxOff]);
		resetRandSeq(randSeq, K);
		for (k = 1; k < K; k++)
		{
			i = next_rand_idx(randSeq, K - k, maxIdx);
			realSeq[k] = i;
			coverNum = genNextElem(ALL, nextRow, T, k + 1, V, val, i);
		}

		if (coverNum > maxV)
		{
			maxV = coverNum;
			TC[TC.size() - 1] = nextRow;
		}
	}
	
	int oriUncoverNum = *uncoverNum;
	coverWithRow(ALL, uncoverNum, TC[TC.size() - 1], T, K);
	TC[TC.size() - 1].push_back(oriUncoverNum - *uncoverNum);

	qDebug() << *uncoverNum << "left" << oriUncoverNum - *uncoverNum << "minused";
}

void AETG(vector<vector<int>>& TC, int T, int K, int* V, int M)
{
	// 所有的覆盖及对应的状态
	//   key:覆盖 val:覆盖的状态{0:未覆盖, 1:覆盖}
	unordered_map<Cover, int, CoverHash, CoverEq> ALL;

	int sum, i;
	seq = new int[K]; // 表示K个测试项的序列
	randSeq = new RandSeq[K]; // 用于产生随机下标序列
	realSeq = new int[K]; // 用于恢复随机打乱后的下标序列顺序

	int** val = NULL; // 表示K个测试项各个的取值可能（顺序生成）
	// val、sum的初始化
	// 例：若有3个测试项（K=3）且第一项有1个，第二项有2个，第三项有3个待测值（V={1,2,3}），则
	// seq={0,1,2}
	// val={{0},{1,2},{3,4,5}}
	{
		int j;
		val = new int* [K];
		for (i = 0, sum = 0; i < K; i++)
		{
			val[i] = new int[V[i]];
			for (j = 0; j < V[i]; j++)
				val[i][j] = sum++;
		}
	}
	maxAppear = new int[sum]; // 表示每个取值的出现次数

	genAllCov(ALL, T, K, V, val);

	int uncoverNum = ALL.size();
	genFirstRow(ALL, &uncoverNum, TC, T, K, val);
	while (uncoverNum)
		genNextRow(ALL, &uncoverNum, TC, T, K, V, val, M);

	// 释放
	{
		delete[] seq;
		delete[] maxAppear;
		delete[] randSeq;
		delete[] realSeq;
		for (i = 0; i < K; i++)
			delete[] val[i];
		delete[] val;
	}
}