#include "Learning.h"
#include "Utility.h"
#include "ScSLINT.h"
#include <algorithm>
#include <thread>


VOID GenerateRepositorySplit(SystemParameters params, INT32 nRepeat)
{
	RepositorySpliter spliter;
	DataSource ds(params.SourceRepositoryPath);
	spliter.BeginSplit(LoadReference(params.LabelFile), ds.SPOEntryCount, params.TrainingSplit, params.ValidationSplit, params.KFold);
	INT8 outBuf[1024];
	while (nRepeat--)
	{
		spliter.Split();
		sprintf(outBuf, "%s_%02d_%.02f_%.02f_%03d", params.Jobname.c_str(), params.KFold, params.TrainingSplit, params.ValidationSplit, nRepeat);
		spliter.Save(params.OutputDirectory + outBuf);
	}
}

VOID GenerateBlock(SystemParameters params, INT32 setID) //input: src, trg, align, split, setID
{
	
	VECTOR<Alignment> alg;
	VECTOR<VECTOR<INT32>> sharedTokens;
	LoadStringAlignment(params.AlignmentFile, alg, sharedTokens);

	alg = LoadAlignment(params.AlignmentFile);
	
	DataSource::LoadTokenID(params.SourceRepositoryPath, params.TargetRepositoryPath);
	
	DataSource dSrc(params.SourceRepositoryPath);
	DataSource dDes(params.TargetRepositoryPath);
	dSrc.LoadPOS(params.Ktok); dDes.LoadPOS(params.Ktok);
	RepositorySpliter split(params.SplitFile);
	auto srcIns = split.GetInstanceList(setID);
	INT64 t2 = TickCount();
	Block(&dSrc, &dDes, &alg, &sharedTokens, &srcIns, params, params.OutputDirectory + params.Jobname + ".block");
	t2 = TickCount() - t2;
	ifstream fi(params.OutputDirectory + params.Jobname + ".block", ios::in | ios::binary);
	LOGSTR(LOGSTREAM, "Blocking," << params.Jobname << "," << ReadINT64(&fi) << "," << t2 << endl);
	fi.close();
	DataSource::ClearTokenID();
}

RPF Evaluate(SystemParameters params, INT32 setID, STRING strFilterFile)
{
	auto links = LoadResult(strFilterFile);
	auto allLinks = LoadReference(params.LabelFile);
	RepositorySpliter rep(params.SplitFile);
	auto srcIns = rep.GetInstanceList(setID);
	auto refTest = AdaptToSplit(&allLinks, &srcIns);
	return Evaluate(&links, &refTest);
}


INT32 GreaterThanCFG(Configuration a, Configuration b)
{
	return a.TrainingPerformance.F1 > b.TrainingPerformance.F1;
}
VECTOR<Configuration> SortSimilarityFunctions(VECTOR<ScoreEntryEx> *ens, Configuration *config, SET<INT64> *refLinks)
{
	INT32 funcSize = (INT32)config->Similarity.size();
	VECTOR<Configuration> allConfig;
	allConfig.reserve(funcSize);
	for (INT32 k = 0; k < funcSize; k++)
	{
		Configuration cfg(config->Similarity, config->FilteringFactor, config->Aggregate);
		cfg.sFOI.push_back(k);
		FindThreshold(ens, &cfg, refLinks);
		config->SimilarityThreshold[k] = cfg.FilteringThreshold;
		allConfig.push_back(cfg);
	}
	sort(allConfig.begin(), allConfig.end(), GreaterThanCFG);
	return allConfig;
}

INT32 GreaterThanPIF(pair<INT64, FLOAT> a, pair<INT64, FLOAT> b)
{
	return a.second > b.second;
}
VOID FindThreshold(VECTOR<ScoreEntryEx> *ens, Configuration *config, SET<INT64> *refLinks)
{
	VECTOR<ScoreEntry> score(ens->size());
	INT32 i = 0;
	for (auto en = ens->begin(); en != ens->end(); en++, i++)
	{
		score[i].Source = en->Source;
		score[i].Target = en->Target;
		score[i].Score = config->Aggregate.Aggregate(&*en, &config->sFOI, &config->SimilarityThreshold);
	}
	auto links = StableFiltering(&score, config->FilteringFactor, config->FilteringThreshold);

	sort(links.begin(), links.end(), GreaterThanPIF);
	INT32 toSelect = (INT32)MIN(refLinks->size(), links.size());
	INT32 nCorrect = 0;
	FLOAT minPositive = 0;
	FLOAT maxNegative = -1;
	for (INT32 i = 0; i < toSelect; i++)
	if (refLinks->find(links[i].first) != refLinks->end())
	{
		nCorrect++;
		minPositive = links[i].second;
	}
	else if (maxNegative == -1)
		maxNegative = links[i].second;
	config->FilteringThreshold = minPositive;
	config->TrainingPerformance.Expected = refLinks->size();
	config->TrainingPerformance.Detected = toSelect;
	config->TrainingPerformance.Correct = nCorrect;
	if (nCorrect > 0)
	{
		config->TrainingPerformance.Recall = (FLOAT)nCorrect / refLinks->size();
		config->TrainingPerformance.Precision = (FLOAT)nCorrect / toSelect;
		config->TrainingPerformance.F1 = 2 * config->TrainingPerformance.Recall * config->TrainingPerformance.Precision / (config->TrainingPerformance.Recall + config->TrainingPerformance.Precision);
	}
}

RPF Validate(VECTOR<ScoreEntryEx> *ens, SET<INT64> *refLink, Configuration* config)
{
	INT8 outBuf[1024];
	VECTOR<ScoreEntry> score(ens->size());
	INT32 i = 0;
	for (auto en = ens->begin(); en != ens->end(); en++, i++)
	{
		score[i].Source = en->Source;
		score[i].Target = en->Target;
		score[i].Score = config->Aggregate.Aggregate(&*en, &config->sFOI, &config->SimilarityThreshold);
	}
	auto links = StableFiltering(&score, config->FilteringFactor, config->FilteringThreshold);
	RPF performance = Evaluate(&links, refLink);
	return performance;
}

INT32 MapSimilarityFunctions(VECTOR<Configuration>* config, VECTOR<INT32>* map, INT32 topSim)
{
	INT32 toSelect = MIN((INT32)config->size(), topSim);
	map->resize(config->at(0).Similarity.size());
	VECTOR<INT32> candidate;
	for (INT32 k = 0; k < map->size(); k++)
		(*map)[k] = -1;
	for (INT32 k = 0; k < toSelect && config->at(k).TrainingPerformance.F1 > 0; k++)
		(*map)[config->at(k).sFOI[0]] = k;
	return toSelect;
}


