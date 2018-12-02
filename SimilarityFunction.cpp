#include "Resolution.h"

SimilarityFunc simFuncID[] = { ExactMatching/*0*/, ReverseDifference/*1*/, Levenshtein/*2*/, JaroWinkler/*3*/, LongestCommonSubstring/*4*/,
								 LongestCommonPrefix /*5*/, Jaccard/*6*/, TFIDFCosine/*7*/, ModifiedBM25/*8*/, JaccardBM25 /*9*/ };
STRING simFuncName[] = { "ExactMatching", "ReverseDifference", "Levenshtein", "JaroWinkler", "LongestCommonSubstring", 
								"LongestCommonPrefix", "Jaccard", "TFIDFCosine", "ModifiedBM25" };

typedef FLOAT(*SimilarityFunc)(DataSource*, DataSource*, SPOEntry*, SPOEntry*, INT32 pSrc, INT32 pDes);

INT32 simFuncCount = 10;

VECTOR<SimilarityFunction> GenerateSimilarityFunction(VECTOR<Alignment> *alg)
{
	VECTOR<SimilarityFunction> pFunc;
	SimilarityFunction temp;
	for (INT32 i = 0; i < alg->size(); i++)
	{
		temp.pSrc = alg->at(i).Source;
		temp.pDes = alg->at(i).Target;
		switch (alg->at(i).Type)
		{
		case 0:
			if (alg->at(i).IsLongString == 0) //short STRING, edit distance
			{
				//temp.pFuncID = 2; pFunc.push_back(temp); 
				//temp.pFuncID = 3; pFunc.push_back(temp);
				//temp.pFuncID = 4; pFunc.push_back(temp);
				//temp.pFuncID = 5; pFunc.push_back(temp);
			} 
			//all types of string
			//temp.pFuncID = 6; pFunc.push_back(temp);
			//temp.pFuncID = 7; pFunc.push_back(temp);
			temp.pFuncID = 9; pFunc.push_back(temp);
			break;

		case 1:
		case 2:
			//number
			//temp.pFuncID = 1; pFunc.push_back(temp);
			break;

		case 3:
		case 4:
			//uri and datetime
			//temp.pFuncID = 0; pFunc.push_back(temp);
			break;
		}
	}
	return pFunc;
}

VECTOR<INT32> GenerateSimilarityFunctionMap(VECTOR<Alignment> *alg)
{
	VECTOR<INT32> map;
	for (INT32 i = 0; i < alg->size(); i++)
	{
		switch (alg->at(i).Type)
		{
		case 0:
			if (alg->at(i).IsLongString == 0)
			{ 
				map.push_back(i); //map.push_back(i); map.push_back(i); map.push_back(i);
			} 
			map.push_back(i); map.push_back(i); //Bmap.push_back(i);
			break;
		case 1:
		case 2:
			map.push_back(i);
			break;
		case 3:
		case 4:
			map.push_back(i);
			break;
		}
	}
	return map;
}


FLOAT SimilarityFunction::Calculate(DataSource* dSrc, DataSource* dDes, SPOEntry* entrySrc, SPOEntry* entryDes)
{
	return simFuncID[pFuncID](dSrc, dDes, entrySrc, entryDes, pSrc, pDes);
};
