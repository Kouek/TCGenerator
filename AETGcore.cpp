#include <stdio.h>
#include <iostream>
#include <unordered_map>
#include <stack>
#include <algorithm>

#include "AETGcore.h"

#include <QDebug>

using namespace std;

// ��comb�������ţ�comb[0:k-1]�����ɵ����
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

// ���ڲ���������еĽṹ������
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
	int movN = floor(1.0f * n * rand() / RAND_MAX), rt; // ����[0:n-1]������

	p = startPtr->next;
	for (int i = 0; i < movN - 1; i++) // ����start movN+1����������һ���±�
		p = p->next;

	if (movN == 0) // ���������star��һ����������һ���±꣬�����±꣬�������±�
	{
		rt = p - randSeq;
		startPtr->next = startPtr->next->next;
	}
	else // �����±꣬�����±�����
	{
		rt = p->next - randSeq;
		p->next = p->next->next;
	}

	return rt;
}

// T-way���ǽṹ������
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

		// Cover��ʱ����
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

static int* seq; // ��ʾK�������������
static int* maxAppear; // ��ʾÿ��ȡֵ�ĳ��ִ���
static RandSeq* randSeq; // ���ڲ�������±�����
static int* realSeq; // ���ڻָ�������Һ���±�����˳��

void genAllCov(unordered_map<Cover, int, CoverHash, CoverEq>& ALL, int T, int K, const int* V, int** val)
{
	int i;
	Cover cov(T);
	for (i = 0; i < K; i++)
		seq[i] = i;

	// �����ܸ���ALL
	// ѭ��1����seq��K����ѡ��T�������ϣ�Comb(T,K)����������� seq[0:T-1]
	//   ѭ��2���� seq[0:T-1] ��Ӧ�������ȡֵ val[seq[i]] (i����[0,T-1]) �и�ѡ��һ����������ϣ�
	//   ���һ��T�����entry����entry����ALL
	do {
		int t; // ��ǰ����������
		stack<int> idxs; // ��¼��ǰ���ڱ����ڼ���������ջ

		// �����ڵ�ĳ���ֱϵȫ����ջ
		// {
		for (t = 0; t < T; t++)
			idxs.push(0);
		// }

		// ��ѭ��2�Ĺ�����Ϊ��ȱ�����
		while (!idxs.empty())
		{
			if (t == T) // ����Ҷ�ӽڵ㣬�������entry��ALL
			{
				cov.clear();
				stack<int> tmpidxs(idxs);
				// ȡ�õ�ǰ����·�����Ӹ��ڵ㵽Ҷ�ڵ㣩����ֵ
				// {
				for (i = T - 1; i >= 0; i--)
				{
					cov.insert(val[seq[i]][tmpidxs.top()]);
					tmpidxs.pop();
				}
				// }
				ALL[Cover(cov)] = 0; // cov����ALL

				if (idxs.top() < V[seq[t - 1]] - 1) // ��ǰҶ�ӽڵ㻹�еܣ�����������
				{
					int tmp = idxs.top();
					// �ƶ����ܽڵ�
					// {
					idxs.pop();
					tmp++;
					idxs.push(tmp);
					// }
				}
				else // ��ǰҶ�ӽڵ��޵ܣ����˵����ڵ�
				{
					idxs.pop();
					t--; // ��߼���
				}
			}
			else // ���ڷ�Ҷ�ڵ�
			{
				if (idxs.top() < V[seq[t - 1]] - 1) // ��ǰ��Ҷ�ڵ��е�
				{
					int tmp = idxs.top();
					// �ƶ����ܽڵ�
					// {
					idxs.pop();
					tmp++;
					idxs.push(tmp);
					// }
					// ���ܽڵ�ĳ���ֱϵȫ����ջ
					// {
					for (i = 0; i <= T - t; i++)
					{
						idxs.push(0);
						t++; // �������
					}
					// }
				}
				else // ��ǰ��Ҷ�ڵ��޵ܣ����˵����ڵ�
				{
					idxs.pop();
					t--; // ��߼���
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
	TC.push_back(nextRow); // ����β��Ϊ��ʱ���������������󸲸ǵ�row

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
	// ���еĸ��Ǽ���Ӧ��״̬
	//   key:���� val:���ǵ�״̬{0:δ����, 1:����}
	unordered_map<Cover, int, CoverHash, CoverEq> ALL;

	int sum, i;
	seq = new int[K]; // ��ʾK�������������
	randSeq = new RandSeq[K]; // ���ڲ�������±�����
	realSeq = new int[K]; // ���ڻָ�������Һ���±�����˳��

	int** val = NULL; // ��ʾK�������������ȡֵ���ܣ�˳�����ɣ�
	// val��sum�ĳ�ʼ��
	// ��������3�������K=3���ҵ�һ����1�����ڶ�����2������������3������ֵ��V={1,2,3}������
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
	maxAppear = new int[sum]; // ��ʾÿ��ȡֵ�ĳ��ִ���

	genAllCov(ALL, T, K, V, val);

	int uncoverNum = ALL.size();
	genFirstRow(ALL, &uncoverNum, TC, T, K, val);
	while (uncoverNum)
		genNextRow(ALL, &uncoverNum, TC, T, K, V, val, M);

	// �ͷ�
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