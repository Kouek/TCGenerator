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
	void insert(int v) {
		dat[size++] = v;

		// Cover满T维时排序
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
static int oriSize = 0;

// 用于寻找下一下标对应取值中，K<T部分的计算
//   因为配对深度2<= T <=10，所以只需2,3,..,9深度，对应数组大小8
//   key:覆盖 val:覆盖的出现次数
static unordered_map<Cover, int, CoverHash, CoverEq> CoversetOfLessT[8];

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
				ALL[Cover(cov)] = 0; // 将cov加入ALL，使用深度拷贝防止dat被delete，初始状态为0

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

	// 将ALL中被ROW覆盖的状态设为1
	Cover cov(T);
	do {
		cov.clear();
		for (i = 0; i < T; i++)
			cov.insert(ROW[seq[i]]);
		if (ALL.find(cov) != ALL.end())
			if (ALL[cov] == 0)
			{
				ALL[cov] = 1; (*uncoverNum)--;
			}
	} while (next_comb(seq, K, T)); // 在K位中取出T位进行组合
}

void genFirstRow(unordered_map<Cover, int, CoverHash, CoverEq>& ALL, int* uncoverNum, 
	vector<vector<int>>& TC,
	int T, int K, int** val)
{
	// 所有测试项的第一个取值构成第一行用例
	vector<int> firstRow;
	for (int i = 0; i < K; i++)
		firstRow.push_back(val[i][0]);

	int oriUncoverNum = *uncoverNum;
	coverWithRow(ALL, uncoverNum, firstRow, T, K); // 让firstRow覆盖ALL
	TC.push_back(firstRow);
	TC[TC.size() - 1].push_back(oriUncoverNum - *uncoverNum); // 记载当前行的覆盖数

	qDebug() << *uncoverNum << "left" << oriUncoverNum - *uncoverNum << "minused"; // Debug
}

void countAppearInUncover(const unordered_map<Cover, int, CoverHash, CoverEq>& ALL,
	int T, int K, const int* V, int** val)
{
	int i, j;
	for (i = 0; i < K; i++)
		for (j = 0; j < V[i]; j++)
			maxAppear[val[i][j]] = 0;
	for (auto itr = ALL.begin(); itr != ALL.end(); ++itr)
		if (itr->second == 0)
			for (i = 0; i < T; i++)
				maxAppear[(itr->first).dat[i]]++;
}

int genNextElem(unordered_map<Cover, int, CoverHash, CoverEq>& ALL,
	vector<int>& ROW,
	int T, int K, const int* V, int** val, int idx)
{
	if (K < T)
	{
		int i, maxOff = 0, maxCoverNum = 0, findFag = 0;
		Cover cov(T);
		ROW.push_back(val[idx][0]);

		for (int j = 0; j < V[idx]; j++)
		{
			ROW[ROW.size() - 1] = val[idx][j];
			cov.clear();
			for (i = 0; i < K; i++)
				cov.insert(ROW[i]);
			if (CoversetOfLessT[K - 2].find(cov) != CoversetOfLessT[K - 2].end())
				if (CoversetOfLessT[K - 2][cov] > maxCoverNum)
				{
					maxOff = j; findFag = 1;
				}
		}

		if (!findFag)
			maxOff = floor(1.0f * V[idx] * rand() / RAND_MAX);

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
				if (ALL.find(cov) != ALL.end())
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

void countCovAppearInUncover(const unordered_map<Cover, int, CoverHash, CoverEq>& ALL,
	int T, int K)
{
	Cover cov(T);
	CoversetOfLessT[K - 2].clear();
	for (auto itr = ALL.begin(); itr != ALL.end(); ++itr)
		if (itr->second == 0)
		{
			do {
				cov.clear();
				for (int i = 0; i < T; i++)
					cov.insert(itr->first.dat[i]);
				if (CoversetOfLessT[K - 2].find(cov) == CoversetOfLessT[K - 2].end())
					CoversetOfLessT[K - 2][Cover(cov)] = 1;
				else
					CoversetOfLessT[K - 2][cov]++;
			} while (next_comb(seq, T, K));
		}
}

void genNextRow(unordered_map<Cover, int, CoverHash, CoverEq>& ALL, int* uncoverNum,
	vector<vector<int>>& TC,
	int T, int K, const int* V, int** val, int M)
{
	int maxIdx = 0, maxOff = 0, maxV = 0, start;
	vector<int> nextRow(K + 1); // 最后一位用于表示每行用例的覆盖数
	
	// 选取第一个取值
	int i, j;
	countAppearInUncover(ALL, T, K, V, val);
	for (i = 0; i < K; i++)
		for (j = 0; j < V[i]; j++)
			if (maxAppear[val[i][j]] > maxAppear[val[maxIdx][maxOff]])
			{
				maxIdx = i; maxOff = j;
			}

	// 从当前ALLL中未覆盖的T覆盖中选i个，统计未覆盖的i覆盖数量，存入CoversetOfLessT[i-2]
	for (i = 2; i < T; i++)
		countCovAppearInUncover(ALL, T, i);

	// 尝试M次，选出M个结果候选中覆盖最多的用例
	TC.push_back(nextRow); // 将队尾作为临时储存变量，储存最优用例
	int k, coverNum = 0;
	maxV = 0;
	realSeq[0] = maxIdx; // 用于恢复原有下标顺序
	for (int m = 0; m < M; m++)
	{
		nextRow.clear();
		nextRow.push_back(val[maxIdx][maxOff]); // 加入第一个取值
		resetRandSeq(randSeq, K);
		for (k = 1; k < K; k++)
		{
			i = next_rand_idx(randSeq, K - k, maxIdx); // 随机得到下一个待处理的下标
			realSeq[k] = i; // 用于恢复原有下标顺序
			coverNum = genNextElem(ALL, nextRow, T, k + 1, V, val, i); // 选出下一个下标对应的取值，返回覆盖数

			if (ALL.size() != oriSize)
				int x = 3;
		}

		// 用于获取最优用例
		if (coverNum > maxV)
		{
			maxV = coverNum;
			for (i = 0; i < K; i++)
				TC[TC.size() - 1][realSeq[i]] = nextRow[i]; // 恢复nextRow为原有下标并加入TC
		}
	}
	
	int oriUncoverNum = *uncoverNum;
	coverWithRow(ALL, uncoverNum, TC[TC.size() - 1], T, K); // 让新产生的行覆盖ALL
	TC[TC.size() - 1][K] = oriUncoverNum - *uncoverNum; // 记载当前行的覆盖数

	qDebug() << *uncoverNum << "left" << oriUncoverNum - *uncoverNum << "minused"; // Debug
}

void AETG(vector<vector<int>>& TC, int T, int K, int* V, int M)
{
	// 所有的覆盖及对应的状态
	//   key:覆盖 val:覆盖的状态{0:未覆盖, 1:覆盖}
	unordered_map<Cover, int, CoverHash, CoverEq> ALL;

	unsigned int sum;
	int i, j;
	seq = new int[K]; // 表示K个测试项的序列
	randSeq = new RandSeq[K]; // 用于产生随机下标序列
	realSeq = new int[K]; // 用于恢复随机打乱后的下标序列顺序

	int** val = NULL; // 表示K个测试项各个的取值可能（顺序生成）
	// val、sum的初始化
	// 例：若有3个测试项（K=3）且第一项有1个，第二项有2个，第三项有3个待测值（V={1,2,3}），则
	// seq={0,1,2}
	// val={{0},{1,2},{3,4,5}}
	{
		val = new int* [K];
		for (i = 0, sum = 0; i < K; i++)
		{
			val[i] = new int[V[i]];
			for (j = 0; j < V[i]; j++)
				val[i][j] = sum++;
		}
	}
	maxAppear = new int[sum]; // 表示每个取值的出现次数

	genAllCov(ALL, T, K, V, val); // 第一步，计算所有从K个正交测试项各自的取值中取出T个，得到的覆盖集合ALL
	oriSize = ALL.size();
	int uncoverNum = ALL.size();
	genFirstRow(ALL, &uncoverNum, TC, T, K, val); // 第二步，产生第一行用例
	while (uncoverNum > 0)
		genNextRow(ALL, &uncoverNum, TC, T, K, V, val, M); // 第三步，按AETG算法产生剩下的用例

	// 释放内存
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