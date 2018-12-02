#include "Resolution.h"
#include "Utility.h"
#include <algorithm>
#include <iostream>  

INT32 CompareBlockEntry_(INT64 X, INT64 Y){ return X < Y; };
VOID SortAndRemoveDuplicate_(VECTOR<INT64> &cans);

VECTOR<INT64> merge(VECTOR<VECTOR<INT64>>& maps, INT32 mask)
{
	VECTOR<INT64> all; 
	for (INT32 j = 0; j < maps.size(); j++)
		if ((1 << j) & mask)
		{
			
			INT32 k = all.size();
			all.resize(maps[j].size() + k);
			//cout << j <<"\t" << maps[j].size() <<endl;
			copy(maps[j].begin(), maps[j].end(), all.begin() + k);
		}

	SortAndRemoveDuplicate_(all);
	all.shrink_to_fit();
	return all;
}

FLOAT recall(VECTOR<INT64>* cans, SET<INT64>* refLinks)
{
	INT32 rec = 0;
	for (auto x = cans->begin(); x != cans->end(); x++)
		rec += RefContains(refLinks, *x);
	return (FLOAT)rec / refLinks->size();
}


VOID LearnBlock(DataSource* dSrc, DataSource* dDes, VECTOR<Alignment> *alg, VECTOR<VECTOR<INT32>> *sharedTokens, SET<INT32> *srcIns, SET<INT64>* refLinks, SystemParameters params, STRING strOutputFile)
{
	MAP<INT32, MAP<INT32, VECTOR<INT32>>> cacheSrc;
	MAP<INT32, MAP<INT32, VECTOR<INT32>>> cacheDes;
	INT64 tokenCount = 0;
	for (INT32 i = 0; i < sharedTokens->size(); i++) //note: sharedTokens->size() is smaller than alg->size()
	{
		INT32 src = alg->at(i).Source; //predicate src
		INT32 des = alg->at(i).Target; //predicate des
		if (!Contains(cacheSrc, src))
			cacheSrc[src] = dSrc->ReadPOSEntry(src);
		if (!Contains(cacheDes, des))
			cacheDes[des] = dDes->ReadPOSEntry(des);
		tokenCount += sharedTokens->at(i).size();
	}

	INT64 t1 = TickCount();
	
	VECTOR<VECTOR<INT64>> maps(sharedTokens->size());
	for (INT32 i = 0; i < sharedTokens->size(); i++)
	{
		VECTOR<INT64> cans;
		cans.reserve(0x80000000); 

		for (auto token : sharedTokens->at(i))
		{
			auto vs = cacheSrc[alg->at(i).Source][token];
			auto vd = cacheDes[alg->at(i).Target][token];
			for (auto s : vs)
			if (srcIns == 0 || (srcIns != 0 && RefContains(srcIns, s)))
			for (auto d : vd)
				cans.push_back(((INT64)d << 32) | s);
		}

		SortAndRemoveDuplicate_(cans);
		maps[i].assign(cans.begin(), cans.end());
	}


	INT32 maskAll = (1 << maps.size()) - 1;
	VECTOR<INT64> all = merge(maps, maskAll);
	FLOAT recAll = recall(&all, refLinks);
	cout << maskAll << "\t" << recAll << "\t" << all.size() << endl;
	INT32 mask = maskAll;
	FLOAT recExp = recAll * params.drop;
	do
	{
		INT32 rem = -1;
		INT32 siz = 0;
		FLOAT rec = 0;

		for (INT32 i = 0; i < maps.size(); i++)
		{
			INT32 maskTemp = mask ^ ((0 ^ mask) & (1 << i));
			if (maskTemp != mask)
			{
				auto canTemp = merge(maps, maskTemp);
				auto recTemp = recall(&canTemp, refLinks);
				if (recTemp >= recExp)
				{

					if (siz < canTemp.size())
					{
						siz = canTemp.size();
						rem = i;
						rec = recTemp;
					}
				}
			}
		}
		//remove the most expensive 
		if (rem != -1)
			mask = mask ^ ((0 ^ mask) & (1 << rem));
		else
			break;
	}
	while(true);
	t1 = TickCount() - t1;

	LOGALL("LearnBlock," << strOutputFile <<","<< maskAll << "," << all.size() << "," << recAll << "," << t1 << ",");

	//decompose and save
	VECTOR<Alignment> nAlg;
	VECTOR<VECTOR<INT32>> nShared;
	for (INT32 j = 0; j < maps.size(); j++)
	if ((1 << j) & mask)
	{
		nAlg.push_back(alg->at(j));
		nShared.push_back(sharedTokens->at(j));

		LOGALL(j << "_");
	}

	int nT = 0;	
	int nC = 0;
	VECTOR<INT64> cans;
	cans.reserve(0x80000000);
	for (INT32 j = 0; j < maps.size(); j++)
		if ((1 << j) & mask)
		{
			auto m = sharedTokens->at(j);
			nT +=m.size();
			for (auto t : m)
			{
				auto vs = cacheSrc[alg->at(j).Source][t];
				auto vd = cacheDes[alg->at(j).Target][t];
				nC += vs.size()*vd.size();
				for (auto s : vs)
					for (auto d : vd)
						cans.push_back(((INT64)d << 32) | s);
			}
		}
	SortAndRemoveDuplicate_(cans);
	LOGALL("LearnBlock," << strOutputFile << "," << maskAll << "," << all.size() << "," << recAll << "," << t1 << "," << nT << "," << nC << "," << cans.size() << ",");


	Save(strOutputFile, nAlg, nShared);
	

	LOGALL(endl);
}


VOID SortAndRemoveDuplicate_(VECTOR<INT64> &cans)
{
	sort(cans.begin(), cans.end(), CompareBlockEntry_);
	INT32 size = 1;
	for (INT32 i = 1; i < cans.size(); i++)
	if (cans[size - 1] != cans[i])
		cans[size++] = cans[i];
	cans.resize(size);
}
