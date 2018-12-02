#include "Resolution.h"
#include "StreamIO.h"
#include "Utility.h"
#include <algorithm>

INT32 ComparePredicateType(PredicateInfo A, PredicateInfo B) { return A.Type < B.Type; }
INT32 ReverseComparePredicate(PredicateInfo A, PredicateInfo B) { return A.Diversity > B.Diversity; }
INT32 ReverseCompareAlignment(Alignment A, Alignment B){ return A.Coverage > B.Coverage; }
INT32 ReverseCompareAlignmentPair(pair<Alignment, VECTOR<INT32>> A, pair<Alignment, VECTOR<INT32>> B){ return A.first.Coverage > B.first.Coverage; }

INT32 EndWiths(STRING S, STRING T)
{
	INT32 i = T.length() - 1;
	INT32 j = S.length() - 1;
	while (i >= 0 && j >= 0 && T[i--] == S[j--]);
	return T.length() > 0 && i == -1;
}

VOID Align(DataSource *dSrc, DataSource *dDes, SystemParameters params, VECTOR<Alignment> *alg, VECTOR<VECTOR<INT32>> *sharedTokens)
{
	//Select from Source
	
	auto srcInfo = dSrc->ReadPredicateInfo();
	auto desInfo = dDes->ReadPredicateInfo();	
	

	//Divides srcInfo INT32o types
	VECTOR<VECTOR<PredicateInfo>> pSrc(5), pDes(5);
	for (auto p : srcInfo)
	{
		if (!EndWiths(p.Name, "sameAs") && !EndWiths(p.Name, "exactMatch"))
		if (p.Coverage >= params.Alpha)
			pSrc[p.Type].push_back(p);
	}
	for (auto p : desInfo)
		pDes[p.Type].push_back(p);

	//Select from Src
	for (INT32 i = 0; i < pSrc.size(); i++) 
	{
		sort(pSrc[i].begin(), pSrc[i].end(), ReverseComparePredicate); //Sort by Diversity
		pSrc[i].resize(MIN(params.Kcover, pSrc[i].size()));
	}

	
	dSrc->LoadPO();
	dDes->LoadPO();

	

	VECTOR<Alignment> accAlign;
	VECTOR<VECTOR<INT32>> accShared;
	//Alignment
	for (INT32 i = 0; i < 5; i++)
	{
		if (i == 0) //String
		{
			MAP<INT32, VECTOR<INT32>> rawAlign;
			MAP<INT32, INT32> desOccurence; //cache, to control memory overflow
			INT64 count = 0;
			for (auto p : pSrc[i])
			for (auto q : pDes[i])
			if (q.ParentCount >= p.ParentCount * params.KoverRAW && !((p.MeanLength >= 100) ^ (q.MeanLength >= 100)))
			{
				rawAlign[p.ID].push_back(q.ID);	
				desOccurence[q.ID]++;
				count++;
			}
			if (count == 0)
				continue;
			Loop(count);
			MAP<INT32, VECTOR<INT32>> cache;
			for (auto r : rawAlign)
			{
				VECTOR <pair<Alignment, VECTOR<INT32>>> selAlign;

				auto po = dSrc->ReadPOEntry(r.first);
				SET<INT32> poSrc(po.begin(), po.end());
				for (auto a : r.second)
				{
					if (!Contains(cache, a))
						cache[a] = dDes->ReadPOEntry(a);
					auto poDes = cache[a];
					if (--desOccurence[a] == 0)
						cache.erase(a);

					VECTOR<INT32> shr;
					shr.reserve(MIN(poDes.size(), poSrc.size()));
					for (auto obj : poDes)
					if (Contains(poSrc, obj))
						shr.push_back(obj);
					if (shr.size() > 0)
					{
						FLOAT cov = (FLOAT)shr.size() / poSrc.size();
						if (cov >= params.KoverRAW)
							selAlign.push_back(make_pair(Alignment(r.first, a, i, srcInfo[r.first].MeanLength >= 100, cov), shr));
					}

					Loop();
				}
				sort(selAlign.begin(), selAlign.end(), ReverseCompareAlignmentPair);
				INT32 top = MIN(params.Kover, selAlign.size());
				for (INT32 i = 0; i < top; i++)
				{
					accAlign.push_back(selAlign[i].first);
					accShared.push_back(selAlign[i].second); //Tokens
				}
			}
		}
		else 
		{
			MAP<INT32, VECTOR<INT32>> rawAlign;
			MAP<INT32, INT32> desOccurence;
			INT64 count = 0;
			for (auto p : pSrc[i])
			for (auto q : pDes[i])
			if (q.ParentCount >= p.ParentCount * params.KoverRAW)
			{
				rawAlign[p.ID].push_back(q.ID);
				desOccurence[q.ID]++;
				count++;
			}
			if (count == 0)
				continue;
			Loop(count);
			MAP<INT32, VECTOR<STRING>> cache;
			for (auto r : rawAlign)
			{
				VECTOR <Alignment> selAlign;
				auto po = dSrc->ReadPOEntryRaw(r.first);
				SET<STRING> poSrc(po.begin(), po.end());
				for (auto a : r.second)
				{
					if (!Contains(cache, a))
						cache[a] = dDes->ReadPOEntryRaw(a);
					auto poDes = cache[a];
					if (--desOccurence[a] == 0)
						cache.erase(a);
					INT32 shr = 0;
					for (auto obj : poDes)
					if (Contains(poSrc, obj))
						shr++;
					if (shr > 0)
					{
						FLOAT cov = (FLOAT)shr / poSrc.size();
						if (cov >= params.KoverRAW)
							selAlign.push_back(Alignment(r.first, a, i, 0, cov));
					}
					Loop();
				}

				sort(selAlign.begin(), selAlign.end(), ReverseCompareAlignment);
				INT32 top = MIN(params.Kover, selAlign.size());
				for (INT32 i = 0; i < top; i++)
					accAlign.push_back(selAlign[i]);
			}
		}
	}

	//Remove duplicates
	for (INT32 i = 0; i < accAlign.size(); i++)
	{
		INT32 exist = 0;
		for (auto j = alg->begin(); j != alg->end(); j++)
		if (accAlign[i].Type == j->Type && accAlign[i].Coverage == j->Coverage
			&& (accAlign[i].Source == j->Source || accAlign[i].Target == j->Target))
		{
			exist = 1;
			break;
		}
		if (!exist)
		{
			alg->push_back(accAlign[i]);
			if (accAlign[i].Type == 0)
				sharedTokens->push_back(accShared[i]);
		}
	}	

	for (auto a : *alg)
		LOGALL(srcInfo[a.Source].Name << "\t" << desInfo[a.Target].Name << endl);
}

VOID Save(STRING strFile, VECTOR<Alignment> &alg, VECTOR<VECTOR<INT32>> &sharedTokens)
{
	INT8 *buf = new INT8[sizeof(Alignment) * alg.size() + 4];
	ofstream fo(strFile, ios::out | ios::binary);
	INT32 size = alg.size();
	fo.write((INT8*)&size, sizeof(INT32));
	for (INT32 i = 0; i < alg.size(); i++)
	{
		fo.write((INT8*)&alg[i].Source, sizeof(INT32));
		fo.write((INT8*)&alg[i].Target, sizeof(INT32));
		fo.write((INT8*)&alg[i].Type, sizeof(INT32));
		fo.write((INT8*)&alg[i].IsLongString, sizeof(INT32));
		fo.write((INT8*)&alg[i].Coverage, sizeof(FLOAT));
	}
	
	size = sharedTokens.size();
	fo.write((INT8*)&size, sizeof(INT32));
	for (auto v : sharedTokens)
	{
		size = v.size();
		fo.write((INT8*)&size, sizeof(INT32));
		fo.write((INT8*)&v[0], sizeof(INT32)* size);
	}
	fo.close();
}

VECTOR<Alignment> LoadAlignment(STRING strFile)
{
	INT8* ptr, *buf;
	ptr = buf = ReadAllBytes(strFile);
	INT32 count = ReadINT32(ptr);
	//VECTOR<Alignment> alg;
	//alg.resize(count);
	//for (INT32 i = 0; i < count; i++)
	//{
	//	alg[i].Source = ReadINT32(ptr);
	//	alg[i].Target = ReadINT32(ptr);
	//	alg[i].Type = ReadINT32(ptr);
	//	alg[i].IsLongString = ReadINT32(ptr);
	//	alg[i].Coverage = ReadFLOAT(ptr);
	//}

	VECTOR<Alignment> alg((Alignment*)ptr, (Alignment*)ptr + count);
	delete[] buf;
	return alg;
}

VOID LoadStringAlignment(STRING strFile, VECTOR<Alignment> &alg, VECTOR<VECTOR<INT32>> &sharedTokens)
{
	ifstream fi(strFile, ios::binary);
	INT32 count = ReadINT32(&fi);	
	alg.resize(count);
	fi.read((INT8*)&alg[0], sizeof(Alignment)* count);	

	count = ReadINT32(&fi);
	alg.resize(count);
	sharedTokens.resize(count);
	int s = 0; 

	for (INT32 i = 0; i < sharedTokens.size(); i++)
	{
		count = ReadINT32(&fi);
		s += count;
		sharedTokens[i].resize(count);
		fi.read((INT8*)&sharedTokens[i][0], sizeof(INT32)* count);
	}
	cout << s << endl;
	fi.close();	
}
