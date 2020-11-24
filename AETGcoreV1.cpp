#include <stdio.h>
#include <iostream>
#include <set>
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
// {
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
// }

// T覆盖项的结构与函数
// {
struct SetInt {
	set<int>* dat;

	SetInt(set<int>* _dat) :dat(_dat) {}

	bool operator < (const SetInt& right) const {
		set<int>::iterator leftItr, rightItr;
		for (leftItr = dat->begin(), rightItr = right.dat->begin();
			leftItr != dat->end() && rightItr != right.dat->end();
			leftItr++, rightItr++)
			if ((*leftItr) < (*rightItr))return true;
			else if ((*leftItr) > (*rightItr))return false;
		if (rightItr != right.dat->end())return true;
		return false;
	}
};
// }

void genAllCovSet(set<SetInt>& ALL, int T, int K, int* V, int** val)
{
	int* seq = new int[K]; // 表示K个测试项的序列
	int i;
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
				set<int>* entry = new set<int>();
				stack<int> tmpidxs(idxs);
				// 取得当前遍历路径（从根节点到叶节点）所有值
				// {
				for (i = T - 1; i >= 0; i--)
				{
					entry->insert(val[seq[i]][tmpidxs.top()]);
					tmpidxs.pop();
				}
				// }
				ALL.insert(SetInt(entry)); // entry加入ALL

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

	delete[] seq;
}

void genAllCovSet(set<SetInt>& ALL, int T, int K, const vector<int>& val)
{
	int* seq = new int[K]; // 表示K个测试项的序列
	int i;
	for (i = 0; i < K; i++)
		seq[i] = i;

	do {
		set<int>* entry = new set<int>();
		for (i = 0; i < T; i++)
			entry->insert(val[seq[i]]);
		ALL.insert(SetInt(entry));
	} while (next_comb(seq, K, T));

	delete[] seq;
}

void deleteCovSet(set<SetInt>& CS)
{
	set<SetInt>::iterator itr;
	for (itr = CS.begin(); itr != CS.end(); itr++)
		delete (*itr).dat;
	CS.clear();
}

void printCovSet(const set<SetInt>& CS)
{
	set<SetInt>::iterator itr;
	set<int>::iterator itr2;
	const set<int>* sp;
	for (itr = CS.begin(); itr != CS.end(); itr++)
	{
		sp = (*itr).dat;
		for (itr2 = sp->begin(); itr2 != sp->end(); itr2++)
			printf("%d ", *itr2);
		printf("\n");
	}
}

int findMaxCov(int* curMinUnCov, int T, int K, int idx, int* V, int** val, const set<SetInt>& UC, vector<int>& curTC)
{
	bool isContain;
	int i, off;
	int maxCov = 0, maxCovOff = 0, curCov;

	set<SetInt>::iterator itr;
	set<int>* sp;

	if (K < T) // curTC的大小小于T，UC逐项查找并计数
		for (off = 0; off < V[idx]; off++)
		{
			curCov = 0;
			curTC[K - 1] = val[idx][off];
			for (itr = UC.begin(); itr != UC.end(); itr++)
			{
				sp = (*itr).dat;
				isContain = true;
				for (i = 0; i < K; i++)
					if (sp->find(curTC[i]) == sp->end())
					{
						isContain = false; break;
					}
				if (isContain)curCov++;
			}
			if (curCov > maxCov)
			{
				maxCovOff = off; maxCov = curCov;
			}
		}
	else
	{
		set<SetInt> C, O;
		*curMinUnCov = UC.size();
		for (off = 0; off < V[idx]; off++)
		{
			curTC[K - 1] = val[idx][off];
			genAllCovSet(C, T, K, curTC);
			set_difference(UC.begin(), UC.end(), C.begin(), C.end(), inserter(O, O.begin()));
			if(O.size() < *curMinUnCov)
			{
				maxCovOff = off; *curMinUnCov = O.size();
			}
			deleteCovSet(C);
			O.clear();
		}
	}

	return maxCovOff;
}

void AETGcore(vector<vector<int>>& TC, const set<SetInt>& ALL, int T, int K, int* V, int M, int** val, int sum)
{
	vector<int> curTC; // 当前测试用例
	int i, j;
	
	set<SetInt> UC(ALL); // uncovered set未覆盖集合，初始为所有覆盖的集合ALL
	set<SetInt> curC; // cureent covered set本次覆盖集合
	set<SetInt> curO; // 本次集合差的输出
	set<SetInt> C; // 维护本函数genCovSet申请的内存，用后释放

	// 所有测试项的第一个取值构成curC
	// {
	for (i = 0; i < K; i++)
		curTC[i] = val[i][0];
	
	TC.push_back(curTC); // 保存第一个测试用例
	
	genAllCovSet(curC, T, K, curTC);
	set_difference(UC.begin(), UC.end(), curC.begin(), curC.end(), inserter(curO, curO.begin()));
	TC[0][K] = UC.size() - curO.size(); // 记录第一次选出的测试用例的覆盖数
	UC = curO; // UC减curC
	set_union(C.begin(), C.end(), curC.begin(), curC.end(), inserter(C, C.end())); // C维护由genCovSet申请的内存
	// }

	int idx, off, maxIdx, maxOff;
	int minUnCov, curMinUnCov;
	int* cnt = new int[sum]; // 计数
	int* curSeq = new int[K]; // 当前下标的顺序
	int* seq = new int[K]; // 下标的顺序

	set<SetInt>::iterator itr;
	set<int>* sp;
	set<int>::iterator itr2;

	RandSeq* randSeq = new RandSeq[K]; // 用于产生随机下标序列

	while (UC.size())
	{
		// 找到UC中出现最多的取值对应的下标
		{
			memset(cnt, 0, sizeof(int) * sum);
			for (itr = UC.begin(); itr != UC.end(); itr++)
			{
				sp = (*itr).dat;
				for (itr2 = sp->begin(); itr2 != sp->end(); itr2++)
					cnt[*itr2]++;
			}
			maxIdx = maxOff = 0;
			for (idx = 0; idx < K; idx++)
				for (off = 0; off < V[idx]; off++)
					if (cnt[val[idx][off]] > cnt[val[maxIdx][maxOff]])
					{
						maxIdx = idx;
						maxOff = off;
					}
		}

		// 产生下一条测试用例，并让UC减去该测试用例的覆盖集合
		{
			minUnCov = UC.size();
			for (i = 0; i < M; i++)
			{
				// 产生M个测试用例，并找出最合适的
				// {
				resetRandSeq(randSeq, K);
				curTC[0] = val[maxIdx][maxOff]; // 固定maxIdx，赋值val[maxIdx][maxOff]
				curSeq[0] = maxIdx;
				for (j = 1; j < K; j++)
				{
					idx = next_rand_idx(randSeq, K - j, maxIdx); // 固定maxIdx，随机产生下一个下标
					curSeq[j] = idx; // 记录下标，保留到当前下标顺序curSeq中
					off = findMaxCov(&curMinUnCov, T,  j + 1, idx, V, val, UC, curTC); // 在val[idx]的V[idx]个值中找出一个，使得其与curTC构成的j+1覆盖在UC中出现最多
					curTC[j] = val[idx][off]; // 将上面得到的值加入curTC
				}

				if (curMinUnCov < minUnCov)
				{
					memcpy(TC[TC.size() - 1], curTC, sizeof(int) * K); // 记录当前测试用例
					memcpy(seq, curSeq, sizeof(int) * K); // 记录当前下标顺序
					minUnCov = curMinUnCov;
				}
				// }
			}
			
			// 将按随机顺序seq排列的TC[*N-1]恢复为按0,1,2,...,K顺序排列
			{
				memcpy(curTC, TC[TC.size() - 1], sizeof(int) * K);
				for (i = 0; i < K; i++)
					TC[TC.size() - 1][seq[i]] = curTC[i];
			}

			// UC减去最终选出的测试用例的覆盖
			{
				curC.clear(); curO.clear();
				genAllCovSet(curC, T, K, TC[TC.size() - 1]);
				set_difference(UC.begin(), UC.end(), curC.begin(), curC.end(), inserter(curO, curO.begin()));
				TC[TC.size() - 1][K] = UC.size() - curO.size(); // 记录最终选出的测试用例的覆盖数
				UC = curO; // UC减curC
				set_union(C.begin(), C.end(), curC.begin(), curC.end(), inserter(C, C.end())); // C维护由genCovSet申请的内存
				qDebug() << UC.size() << "left " << TC[TC.size() - 1][K] << "minused";
			}
		}
	}

	delete[] randSeq;

	deleteCovSet(C); // 释放本函数中由genCovSet申请的所有内存

	delete[] cnt;
	delete[] curSeq;
	delete[] seq;
}

void AETG(vector<vector<int>>& TC, int T, int K, int* V, int M)
{
	set<SetInt> ALL; // 表示所有匹配的集合
	int sum, i;

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

	genAllCovSet(ALL, T, K, V, val); // 产生所有覆盖的集合，存入ALL

	//printCovSet(ALL); // 打印ALL

	AETGcore(TC, ALL, T, K, V, M, val, sum);

	// for debug
	// {
	/*vector<int*>::iterator itr;
	printf("result:\n");
	for (itr = TC.begin(); itr != TC.end(); itr++)
	{
		for (i = 0; i < K; i++)
			printf("%d\t", (*itr)[i]);
		printf("\n");
	}*/
	// }

	deleteCovSet(ALL); // 释放ALL维护的由genCovSet申请的所有内存

	// 释放val
	{
		for (i = 0; i < K; i++)
			delete[] val[i];
		delete[] val;
	}
}