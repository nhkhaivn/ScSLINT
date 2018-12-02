#include "cLink.h"
#include "ScSLINT.h"
#include "Utility.h"
#include <algorithm>
#include <thread>

VOID cLink(SystemParameters params) //input: src, trg, split, align, label
{
	char outBuf[1024]; sprintf(outBuf, "%d\0", params.LearnAlgorithm);
	STRING originAlign = params.AlignmentFile;

	//learn blocking model
	if (!IsExist(params.OutputDirectory + params.Jobname + ".align.block"))
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
		auto srcIns = split.GetInstanceList(TRAININGSET | VALIDATIONSET |TESTSET);
		auto allLinks = LoadReference(params.LabelFile);
		auto refTrain = AdaptToSplit(&allLinks, &srcIns);
		
		INT64 t2 = TickCount();
		LearnBlock(&dSrc, &dDes, &alg, &sharedTokens, &srcIns, &refTrain, params, params.OutputDirectory + params.Jobname + ".align.block");
		t2 = TickCount() - t2;
		DataSource::ClearTokenID();
	}
	return;

	//Blocking
	if (!IsExist(params.OutputDirectory + params.Jobname + ".block.training"))
	{
		params.AlignmentFile = params.OutputDirectory + params.Jobname + ".align.block";
		GenerateBlock(params, TRAININGSET);
		CMD(RN + params.OutputDirectory + params.Jobname + ".block " + params.OutputDirectory + params.Jobname + ".block.training");
	}
	if (!IsExist(params.OutputDirectory + params.Jobname + ".block.validation"))
	{
		params.AlignmentFile = params.OutputDirectory + params.Jobname + ".align.block";
		GenerateBlock(params, VALIDATIONSET);
		CMD(RN + params.OutputDirectory + params.Jobname + ".block " + params.OutputDirectory + params.Jobname + ".block.validation");
	}
	if (!IsExist(params.OutputDirectory + params.Jobname + ".block.test"))
	{
		params.AlignmentFile = params.OutputDirectory + params.Jobname + ".align.block";
		GenerateBlock(params, TESTSET);
		CMD(RN + params.OutputDirectory + params.Jobname + ".block " + params.OutputDirectory + params.Jobname + ".block.test");
	}

	//Match training&validation to detailed score (ScoreEntryEx) file
	params.AlignmentFile = originAlign;
	if (!IsExist(params.OutputDirectory + params.Jobname + ".score.training"))
	{
		VECTOR<Alignment> alg = LoadAlignment(params.AlignmentFile);
		auto simFuncs = GenerateSimilarityFunction(&alg);
		Configuration cfg(simFuncs);
		cfg.Save(params.OutputDirectory + params.Jobname + ".config.default");
		params.ConfigurationFile = params.OutputDirectory + params.Jobname + ".config.default";
		params.BlockFile = params.OutputDirectory + params.Jobname + ".block.training";
		params.Detail = 1;
		Match(params);
		CMD(RN + params.OutputDirectory + params.Jobname + ".score " + params.OutputDirectory + params.Jobname + ".score.training");
	}
	if (!IsExist(params.OutputDirectory + params.Jobname + ".score.validation"))
	{
		VECTOR<Alignment> alg = LoadAlignment(params.AlignmentFile);
		auto simFuncs = GenerateSimilarityFunction(&alg);
		Configuration cfg(simFuncs);
		cfg.Save(params.OutputDirectory + params.Jobname + ".config.default");
		params.ConfigurationFile = params.OutputDirectory + params.Jobname + ".config.default";
		params.BlockFile = params.OutputDirectory + params.Jobname + ".block.validation";
		params.Detail = 1;
		Match(params);
		CMD(RN + params.OutputDirectory + params.Jobname + ".score " + params.OutputDirectory + params.Jobname + ".score.validation");
	}

	//learn configuration
	if (!IsExist(params.OutputDirectory + params.Jobname + ".config.clearn." + outBuf))
	{
		cLearn(params);
	}

	//Match test set
	if (!IsExist(params.OutputDirectory + params.Jobname + ".score." + outBuf))
	{
		params.AlignmentFile = params.OutputDirectory + params.Jobname + ".align.clearn." + outBuf;
		params.ConfigurationFile = params.OutputDirectory + params.Jobname + ".config.clearn." + outBuf;
		params.BlockFile = params.OutputDirectory + params.Jobname + ".block.test";
		params.Detail = 0;
		Match(params);
		CMD(RN + params.OutputDirectory + params.Jobname + ".score " + params.OutputDirectory + params.Jobname + ".score." + outBuf);
	}
	if (!IsExist(params.OutputDirectory + params.Jobname + ".result." + outBuf))
	{
		params.ConfigurationFile = params.OutputDirectory + params.Jobname + ".config.clearn." + outBuf;
		params.ScoreFile = params.OutputDirectory + params.Jobname + ".score." + outBuf;
		Filter(params);
		CMD(RN + params.OutputDirectory + params.Jobname + ".result " + params.OutputDirectory + params.Jobname + ".result." + outBuf);
		CMD(RM + params.ScoreFile);
	}
	auto rpf = Evaluate(params, TESTSET, params.OutputDirectory + params.Jobname + ".result." + outBuf);
	sprintf(outBuf, "%d,%d,%d,%f,%f,%f", rpf.Expected, rpf.Detected, rpf.Correct, rpf.Recall, rpf.Precision, rpf.F1);
	LOGALL("Test," << params.Jobname << "," << outBuf << endl);
}

VOID cLearn(SystemParameters params) //input src, trg, align, split, name. Output: configuration, new Alignment
{
	RepositorySpliter rep(params.SplitFile);
	auto allLinks = LoadReference(params.LabelFile);
	auto srcIns = rep.GetInstanceList(TRAININGSET);
	auto refTrain = AdaptToSplit(&allLinks, &srcIns);
	srcIns = rep.GetInstanceList(VALIDATIONSET);
	auto refValidation = AdaptToSplit(&allLinks, &srcIns);

	VECTOR<Alignment> alg;
	VECTOR<VECTOR<INT32>> sharedTokens;
	LoadStringAlignment(params.AlignmentFile, alg, sharedTokens);
	alg = LoadAlignment(params.AlignmentFile);
	auto simFunctions = GenerateSimilarityFunction(&alg);

	//Load similarity vectors
	auto ensTrain = LoadScoreEntryEx(params.OutputDirectory + params.Jobname + ".score.training");
	auto ensValidation = LoadScoreEntryEx(params.OutputDirectory + params.Jobname + ".score.validation");

	//Learning
	Learner func[] = { Naive, Heuristic, Exhaustive, InfoGain, Genetic, AcceptAll };
	Configuration seed[16];
	VECTOR<thread> tLearn;

	INT8 outBuf[1024];
	sprintf(outBuf, "%sdetail_%s_%d_thread", params.TempDirectory.c_str(), params.Jobname.c_str(), params.LearnAlgorithm);
	for (INT32 j = 0; j < 16; j++)
	{
		seed[j] = Configuration(simFunctions, params.FilteringFactor, LinearAggregator(j));
		tLearn.push_back(thread(func[params.LearnAlgorithm], &ensTrain, &refTrain, &ensValidation, &refValidation, &seed[j], params, outBuf, j));
	}

	INT64 t2 = TickCount();
	for (INT32 j = 0; j < tLearn.size(); j++)
		tLearn[j].join();
	//Select max
	INT32 max = 0;
	for (INT32 j = 1; j < tLearn.size(); j++)
	if (seed[j].ValidationPerformance.F1 >= seed[max].ValidationPerformance.F1)
		max = j;
	Configuration cfg = seed[max];
	sort(cfg.sFOI.begin(), cfg.sFOI.end());
	t2 = TickCount() - t2;

	//Adjust new alignment
	VECTOR<Alignment> nalg;
	VECTOR<VECTOR<INT32>> nsharedTokens;
	SET<INT32> nID;
	auto map = GenerateSimilarityFunctionMap(&alg);
	for (auto f : cfg.sFOI)
	if (!Contains(nID, map[f]))
	{
		nID.insert(map[f]);
		nalg.push_back(alg[map[f]]);
		if (alg[map[f]].Type == 0)
			nsharedTokens.push_back(sharedTokens[map[f]]);
	}
	sprintf(outBuf, "%d", params.LearnAlgorithm);
	cfg.Save(params.OutputDirectory + params.Jobname + ".config.clearn." + outBuf);
	Save(params.OutputDirectory + params.Jobname + ".align.clearn." + outBuf, nalg, nsharedTokens);

	LOGALL("Learning," << params.Jobname + ".config.clearn." + outBuf << "," << cfg.ToString() << "," << t2 << endl);
}
