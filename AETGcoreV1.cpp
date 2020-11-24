#include <stdio.h>
#include <iostream>
#include <set>
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
// }

// T������Ľṹ�뺯��
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
	int* seq = new int[K]; // ��ʾK�������������
	int i;
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
				set<int>* entry = new set<int>();
				stack<int> tmpidxs(idxs);
				// ȡ�õ�ǰ����·�����Ӹ��ڵ㵽Ҷ�ڵ㣩����ֵ
				// {
				for (i = T - 1; i >= 0; i--)
				{
					entry->insert(val[seq[i]][tmpidxs.top()]);
					tmpidxs.pop();
				}
				// }
				ALL.insert(SetInt(entry)); // entry����ALL

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

	delete[] seq;
}

void genAllCovSet(set<SetInt>& ALL, int T, int K, const vector<int>& val)
{
	int* seq = new int[K]; // ��ʾK�������������
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

	if (K < T) // curTC�Ĵ�СС��T��UC������Ҳ�����
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
	vector<int> curTC; // ��ǰ��������
	int i, j;
	
	set<SetInt> UC(ALL); // uncovered setδ���Ǽ��ϣ���ʼΪ���и��ǵļ���ALL
	set<SetInt> curC; // cureent covered set���θ��Ǽ���
	set<SetInt> curO; // ���μ��ϲ�����
	set<SetInt> C; // ά��������genCovSet������ڴ棬�ú��ͷ�

	// ���в�����ĵ�һ��ȡֵ����curC
	// {
	for (i = 0; i < K; i++)
		curTC[i] = val[i][0];
	
	TC.push_back(curTC); // �����һ����������
	
	genAllCovSet(curC, T, K, curTC);
	set_difference(UC.begin(), UC.end(), curC.begin(), curC.end(), inserter(curO, curO.begin()));
	TC[0][K] = UC.size() - curO.size(); // ��¼��һ��ѡ���Ĳ��������ĸ�����
	UC = curO; // UC��curC
	set_union(C.begin(), C.end(), curC.begin(), curC.end(), inserter(C, C.end())); // Cά����genCovSet������ڴ�
	// }

	int idx, off, maxIdx, maxOff;
	int minUnCov, curMinUnCov;
	int* cnt = new int[sum]; // ����
	int* curSeq = new int[K]; // ��ǰ�±��˳��
	int* seq = new int[K]; // �±��˳��

	set<SetInt>::iterator itr;
	set<int>* sp;
	set<int>::iterator itr2;

	RandSeq* randSeq = new RandSeq[K]; // ���ڲ�������±�����

	while (UC.size())
	{
		// �ҵ�UC�г�������ȡֵ��Ӧ���±�
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

		// ������һ����������������UC��ȥ�ò��������ĸ��Ǽ���
		{
			minUnCov = UC.size();
			for (i = 0; i < M; i++)
			{
				// ����M���������������ҳ�����ʵ�
				// {
				resetRandSeq(randSeq, K);
				curTC[0] = val[maxIdx][maxOff]; // �̶�maxIdx����ֵval[maxIdx][maxOff]
				curSeq[0] = maxIdx;
				for (j = 1; j < K; j++)
				{
					idx = next_rand_idx(randSeq, K - j, maxIdx); // �̶�maxIdx�����������һ���±�
					curSeq[j] = idx; // ��¼�±꣬��������ǰ�±�˳��curSeq��
					off = findMaxCov(&curMinUnCov, T,  j + 1, idx, V, val, UC, curTC); // ��val[idx]��V[idx]��ֵ���ҳ�һ����ʹ������curTC���ɵ�j+1������UC�г������
					curTC[j] = val[idx][off]; // ������õ���ֵ����curTC
				}

				if (curMinUnCov < minUnCov)
				{
					memcpy(TC[TC.size() - 1], curTC, sizeof(int) * K); // ��¼��ǰ��������
					memcpy(seq, curSeq, sizeof(int) * K); // ��¼��ǰ�±�˳��
					minUnCov = curMinUnCov;
				}
				// }
			}
			
			// �������˳��seq���е�TC[*N-1]�ָ�Ϊ��0,1,2,...,K˳������
			{
				memcpy(curTC, TC[TC.size() - 1], sizeof(int) * K);
				for (i = 0; i < K; i++)
					TC[TC.size() - 1][seq[i]] = curTC[i];
			}

			// UC��ȥ����ѡ���Ĳ��������ĸ���
			{
				curC.clear(); curO.clear();
				genAllCovSet(curC, T, K, TC[TC.size() - 1]);
				set_difference(UC.begin(), UC.end(), curC.begin(), curC.end(), inserter(curO, curO.begin()));
				TC[TC.size() - 1][K] = UC.size() - curO.size(); // ��¼����ѡ���Ĳ��������ĸ�����
				UC = curO; // UC��curC
				set_union(C.begin(), C.end(), curC.begin(), curC.end(), inserter(C, C.end())); // Cά����genCovSet������ڴ�
				qDebug() << UC.size() << "left " << TC[TC.size() - 1][K] << "minused";
			}
		}
	}

	delete[] randSeq;

	deleteCovSet(C); // �ͷű���������genCovSet����������ڴ�

	delete[] cnt;
	delete[] curSeq;
	delete[] seq;
}

void AETG(vector<vector<int>>& TC, int T, int K, int* V, int M)
{
	set<SetInt> ALL; // ��ʾ����ƥ��ļ���
	int sum, i;

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

	genAllCovSet(ALL, T, K, V, val); // �������и��ǵļ��ϣ�����ALL

	//printCovSet(ALL); // ��ӡALL

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

	deleteCovSet(ALL); // �ͷ�ALLά������genCovSet����������ڴ�

	// �ͷ�val
	{
		for (i = 0; i < K; i++)
			delete[] val[i];
		delete[] val;
	}
}