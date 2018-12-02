#include "cLink.h"
#include "StreamIO.h"
#include "Utility.h"
#include <thread>

VOID Naive(VECTOR<ScoreEntryEx> *ensTrain, SET<INT64> *refLinksTrain, VECTOR<ScoreEntryEx> *ensValidation, SET<INT64> *refLinksValidation,
	Configuration* config, SystemParameters params, STRING strOutputFile, INT32 outID)
{
	if (config->Similarity.size() == 0)
		return;

	auto allConfig = SortSimilarityFunctions(ensTrain, config, refLinksTrain);
	INT8 outBuf[1024];
	sprintf(outBuf, "%d", outID);
	ofstream fo(strOutputFile + outBuf);

	for (auto cfg : allConfig)
		fo << cfg.ToString() << endl;
	VECTOR<INT32> map;
	INT32 count = MapSimilarityFunctions(&allConfig, &map, params.TopSimFunctionCount);
	if (count == 0)
		return;
	map.clear();
	for (INT32 i = 0; i < count; i++)
		config->sFOI.push_back(allConfig[i].sFOI[0]);
	
	FindThreshold(ensTrain, config, refLinksTrain);
	
	sprintf(outBuf, "[%-2d] goal:%.5f\n", outID, config->TrainingPerformance.F1); LOGSTRSAFE(cout, outBuf);
	config->FilteringThreshold = config->FilteringThreshold;
	config->TrainingPerformance = config->TrainingPerformance;
	config->sFOI.assign(config->sFOI.begin(), config->sFOI.end());

	config->ValidationPerformance = Validate(ensValidation, refLinksValidation, config);
	sprintf(outBuf, "[%-2d] Finished: %s", outID, config->ToString().c_str());  LOGSTRSAFE(cout, outBuf << endl);
	fo << outBuf << endl;
	fo.close();
}

