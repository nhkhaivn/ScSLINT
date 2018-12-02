#include "cLink.h"
#include "StreamIO.h"
#include "Utility.h"
#include <thread>
#include <algorithm>
#include <stdlib.h>

FLOAT Entropy(INT32 pos, INT32 neg)
{
	if (pos == 0 || neg == 0)
		return 0;

	FLOAT all = pos + neg;
	FLOAT p = pos / all;
	FLOAT n = neg / all;
	return -(p * LOG(p, 2) + n * LOG(n, 2));
}

FLOAT InformationGainDiscrete(VECTOR<FLOAT> score[2])
{
	FLOAT min = 0xFFFF;
	FLOAT max = 0;

	for (INT32 i = 0; i < 2; i++)
	for (auto sim : score[i])
	{
		min = MIN(min, sim);
		max = MAX(max, sim);
	}
	FLOAT epsilon = (max - min) / 10;
	if (fabs(max - min)<0.00001)
		epsilon = 1;

	FLOAT maxInfoGain = 0;
	for (FLOAT split = min; split <= max; split += epsilon)
	{
		INT32 confusion[2][2] = { 0 };
		for (INT32 i = 0; i < 2; i++)
		for (auto sim : score[i])
			confusion[i][sim >= split]++;

		FLOAT all = score[0].size() + score[1].size();
		FLOAT ae = (confusion[0][0] + confusion[1][0]) / all * Entropy(confusion[0][0], confusion[1][0])
			+ (confusion[0][1] + confusion[1][1]) / all * Entropy(confusion[0][1], confusion[1][1]);
		FLOAT infoGain = Entropy(score[0].size(), score[1].size()) - ae;
		maxInfoGain = MAX(maxInfoGain, infoGain);
	}
	return maxInfoGain;
}


INT32 GreaterThanPIF(pair<INT32, FLOAT> a, pair<INT32, FLOAT> b)
{
	return a.second > b.second;
}
VECTOR<INT32> SelectBalanceDataset(VECTOR<ScoreEntryEx> *ens, Configuration *def, SET<INT64> *refLinks)
{
	MAP<INT32, pair<INT32, FLOAT>> src;

	VECTOR<INT32> idx;
	src.reserve(ens->size());
	idx.reserve(refLinks->size() * 2);

	INT32 i = 0;
	for (auto en = ens->begin(); en != ens->end(); en++, i++)
	{
		INT64 entry = ((INT64)en->Target << 32) | en->Source;
		if (refLinks->find(entry) != refLinks->end()) //positive
			idx.push_back(i);
		else
		{
			//negative --> find max score
			FLOAT score = def->Aggregate.Aggregate(&*en, &def->sFOI, &def->SimilarityThreshold);
			if (score > 0)
			{
				INT32 exist = src.find(en->Source) != src.end();
				if (!exist || (exist && score > src[en->Source].second))
					src[en->Source] = pair<INT32, FLOAT>(i, score);
			}
		}
	}

	for (auto entry = src.begin(); entry != src.end(); entry++)
		idx.push_back(entry->second.first);
	return idx;
}

VOID InfoGain(VECTOR<ScoreEntryEx> *ensTrain, SET<INT64> *refLinksTrain, VECTOR<ScoreEntryEx> *ensValidation, SET<INT64> *refLinksValidation,
	Configuration* config, SystemParameters params, STRING strOutputFile, INT32 outID)
{
	if (config->Similarity.size() == 0)
		return;

	//VECTOR<INT32> ids = SelectBalanceDataset(ensTrain, config, refLinksTrain);	
	//VECTOR<ScoreEntryEx> ens; ens.reserve(ids.size());
	//for (auto id : ids)
	//	ens.push_back(ensTrain->at(id));	
	auto ens = *ensTrain;

	INT8 outBuf[1024];
	sprintf(outBuf, "%d", outID);
	ofstream fo(strOutputFile + outBuf);

	INT32 funcSize = (INT32)config->Similarity.size();
	VECTOR<pair<INT32, FLOAT>> gainList;
	
	for (INT32 k = 0; k < funcSize; k++)
	{	
		VECTOR<FLOAT> score[2];
		score[0].reserve(ens.size()); //negative
		score[1].reserve(refLinksTrain->size()); //positive
		config->sFOI.push_back(k);
		
		for (auto en = ens.begin(); en != ens.end(); en++)
		{
			INT64 ref = ((INT64)en->Target << 32) | en->Source;			
			FLOAT sim = config->Aggregate.Aggregate(&*en, &config->sFOI, &config->SimilarityThreshold);
			if (sim > 0)
				score[RefContains(refLinksTrain, ref)].push_back(sim);
		}

		FLOAT gain = InformationGainDiscrete(score);
		gainList.push_back(pair<INT32, FLOAT>(k, gain));
		sprintf(outBuf, "[%-2d] funcID:%-2d | gain:%.5f", outID, k, gain); 
		fo << outBuf;
		LOGSTRSAFE(cout, outBuf << endl);
		config->sFOI.clear();
	}
	
	sort(gainList.begin(), gainList.end(), GreaterThanPIF);
	INT32 topSim = MIN(params.TopSimFunctionCount, gainList.size());
	for (INT32 i = 0; i < topSim; i++)
		config->sFOI.push_back(gainList[i].first);
	FindThreshold(&ens, config, refLinksTrain);

	config->ValidationPerformance = Validate(ensValidation, refLinksValidation, config);
	sprintf(outBuf, "[%-2d] Finished: %s", outID, config->ToString().c_str());  LOGSTRSAFE(cout, outBuf << endl);
	fo << outBuf << endl;
	fo.close();
}

