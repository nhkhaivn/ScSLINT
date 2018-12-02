#include "cLink.h"
#include "StreamIO.h"
#include "Utility.h"
#include <thread>
#include <algorithm>


VECTOR<INT32> Combine(VECTOR<INT32> &A, VECTOR<INT32> &B)
{
	VECTOR<INT32> C(A.size() + B.size());
	C.reserve(A.size() + B.size());
	auto it = set_union(A.begin(), A.end(), B.begin(), B.end(), C.begin());
	C.resize(it - C.begin());
	return C;
}

INT64 Compose(VECTOR<INT32> ids, VECTOR<INT32> map)
{
	INT64 id = 0;
	for (INT32 i : ids)
		id |= (INT64)(1L << map[i]);
	return id;
}

VOID Heuristic(VECTOR<ScoreEntryEx> *ensTrain, SET<INT64> *refLinksTrain, VECTOR<ScoreEntryEx> *ensValidation, SET<INT64> *refLinksValidation,
	Configuration* config, SystemParameters params, STRING strOutputFile, INT32 outID)
{
	if(config->Similarity.size() == 0)
		return;
	INT8 outBuf[1024];
	sprintf(outBuf, "%d", outID);
	ofstream fo(strOutputFile + outBuf);

	auto allConfig = SortSimilarityFunctions(ensTrain, config, refLinksTrain);
	for (auto cfg : allConfig)
		LOGSTR(fo, cfg.ToString() << endl);

	VECTOR<INT32> map, candidate;
	INT32 selected = MapSimilarityFunctions(&allConfig, &map, params.TopSimFunctionCount);
	if (selected == 0)
		return;
	for (INT32 i = 0; i < selected; i++)
		candidate.push_back(i);	
	allConfig.reserve(1 << selected);

	INT32 maxID = candidate[0];
	INT32 loop = 0;
	

	sprintf(outBuf, "[%-2d] loop:%-2d | check:%-4d | next:%-4d | goal:%.5f", outID, loop++, allConfig.size(), candidate.size(), allConfig[maxID].TrainingPerformance.F1);
	LOGSTRSAFE(cout, outBuf << endl);

	//Multiple
	MAP<INT64, INT32> visited;
	while (candidate.size() > 0)
	{
		VECTOR<INT32> next;
		for (INT32 a = 0; a < candidate.size(); a++)
		for (INT32 b = a + 1; b < candidate.size(); b++)
		{
			auto cfgA = allConfig.begin() + candidate[a];
			auto cfgB = allConfig.begin() + candidate[b];

			Configuration cfg(*config);
			cfg.sFOI = Combine(cfgA->sFOI, cfgB->sFOI);
			INT64 id = Compose(cfg.sFOI, map);
			if (!Contains(visited, id))
			{
				FindThreshold(ensTrain, &cfg, refLinksTrain); 
				INT32 nextID = (INT32)allConfig.size();
				if (cfg.TrainingPerformance.F1 >= MAX(cfgA->TrainingPerformance.F1, cfgB->TrainingPerformance.F1))
				{
					next.push_back(nextID);
					if (cfg.TrainingPerformance.F1 >= allConfig.at(maxID).TrainingPerformance.F1)
						maxID = nextID;
				}
				visited[id] = nextID;
				allConfig.push_back(cfg);
				fo << cfg.ToString() << endl << flush;
			}
		}
		candidate = next;
		sprintf(outBuf, "[%-2d] loop:%-2d | check:%-4d | next:%-4d | goal:%.5f", outID, loop++, allConfig.size(), candidate.size(), allConfig[maxID].TrainingPerformance.F1);
		LOGSTRSAFE(cout, outBuf << endl);
	}
	config->FilteringThreshold = allConfig[maxID].FilteringThreshold;
	config->TrainingPerformance = allConfig[maxID].TrainingPerformance;
	config->sFOI.assign(allConfig[maxID].sFOI.begin(), allConfig[maxID].sFOI.end());
	
	
	config->ValidationPerformance = Validate(ensValidation, refLinksValidation, config);
	sprintf(outBuf, "[%-2d] Finished: %s", outID, config->ToString().c_str()); LOGSTRSAFE(cout, outBuf << endl);
	fo << outBuf << endl;
	fo.close();
}
