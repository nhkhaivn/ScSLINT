#include "cLink.h"
#include "StreamIO.h"
#include "Utility.h"
#include <thread>

VOID Exhaustive(VECTOR<ScoreEntryEx> *ensTrain, SET<INT64> *refLinksTrain, VECTOR<ScoreEntryEx> *ensValidation, SET<INT64> *refLinksValidation,
	Configuration* config, SystemParameters params, STRING strOutputFile, INT32 outID)
{
	if (config->Similarity.size() == 0)
		return;

	auto allConfig = SortSimilarityFunctions(ensTrain, config, refLinksTrain);
	INT8 outBuf[1024];
	sprintf(outBuf, "%d", outID);
	ofstream fo(strOutputFile + outBuf);

	for (auto cfg : allConfig)
		fo<< cfg.ToString() << endl;
	VECTOR<INT32> map;
	INT32 count = MapSimilarityFunctions(&allConfig, &map, params.TopSimFunctionCount);
	if (count == 0)
		return;
	map.clear();
	for (INT32 i = 0; i < count; i++)
		map.push_back(allConfig[i].sFOI[0]);
	
	allConfig.reserve(1 << count);
	INT32 maxID = 0;
	
	for (INT64 mask = 1; mask < allConfig.capacity(); mask++)
	{
		//decompose mask into array of SimFuns
		Configuration cfg(*config);
		for (INT64 k = 0; k < count; k++)
		if ((1 << k) & mask)
			cfg.sFOI.push_back(map[k]);

		//compute PRF
		if (cfg.sFOI.size() > 1)
		{
			FindThreshold(ensTrain, &cfg, refLinksTrain);
			if (cfg.TrainingPerformance.F1 > allConfig[maxID].TrainingPerformance.F1)
				maxID = allConfig.size();
			allConfig.push_back(cfg);
			fo << cfg.ToString() << endl;
		}
		sprintf(outBuf, "[%-2d] loop:%-2d | goal:%.5f\n", outID, allConfig.size(), allConfig[maxID].TrainingPerformance.F1); LOGALLSAFE(outBuf);
	}
	
	config->FilteringThreshold = allConfig[maxID].FilteringThreshold;
	config->TrainingPerformance = allConfig[maxID].TrainingPerformance;
	config->sFOI.assign(allConfig[maxID].sFOI.begin(), allConfig[maxID].sFOI.end());
	

	config->ValidationPerformance = Validate(ensValidation, refLinksValidation, config);
	sprintf(outBuf, "[%-2d] | Finished: %s", outID, config->ToString().c_str()); LOGALLSAFE(outBuf);
	fo << outBuf;
	fo.close();
}

