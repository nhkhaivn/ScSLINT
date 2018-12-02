#include "cLink.h"
#include "StreamIO.h"
#include "Utility.h"
#include <thread>
#include <stdlib.h>

VOID AcceptAll(VECTOR<ScoreEntryEx> *ensTrain, SET<INT64> *refLinksTrain, VECTOR<ScoreEntryEx> *ensValidation, SET<INT64> *refLinksValidation, 
	Configuration* config, SystemParameters params, STRING strOutputFile, INT32 outID)
{
	ofstream fo(strOutputFile);
	config->sFOI.resize(config->Similarity.size());
	config->SimilarityThreshold.resize(config->Similarity.size());
	for (INT32 i = 0; i < config->Similarity.size(); i++)
		config->sFOI[i] = i;
	FindThreshold(ensTrain, config, refLinksTrain);
	INT8 outBuf[1024];
	sprintf(outBuf, "[%-2d] goal:%.5f\n", outID, config->TrainingPerformance.F1); LOGSTRSAFE(cout, outBuf);
	fo << outBuf;
	
	config->ValidationPerformance = Validate(ensValidation, refLinksValidation, config);
	sprintf(outBuf, "[%-2d] Finished: %s", outID, config->ToString().c_str());  LOGSTRSAFE(cout, outBuf << endl);
	fo << outBuf << endl;
	fo.close();
}
