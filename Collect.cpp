#include "Core.h"
#include "Learning.h"
#include "Evaluator.h"
#include "Utility.h"
#include "ScSLINT.h"
#include <stdlib.h>

VOID CollectResult(MAP<STRING, STRING> params)
{
	if (Contains(params, "block") && Contains(params, "label") && Contains(params, "split") && Contains(params, "fold")) //blocking
	{
		RepositorySpliter sp(params["split"]);
		auto ref = LoadReference(params["label"]);
		SET<INT32> ins;
		INT32 temp=0;
		if (params["fold"].find("training") != string::npos)
			temp |= TRAININGSET;
		if (params["fold"].find("validation") != string::npos)
			temp |= VALIDATIONSET;
		if (params["fold"].find("test") != string::npos)
			temp |= TESTSET;
		ins = sp.GetInstanceList(temp);
		SET<INT64> r = AdaptToSplit(&ref, &ins);
		auto rpf = Evaluate(params["block"], &r);
		LOGALL(params["block"] << "," << rpf.Detected << "," << rpf.Expected << "," << rpf.Correct << endl);
		return;
	}

	if (Contains(params, "block") && Contains(params, "label")) //blocking
	{
		auto ref = LoadReference(params["label"]);
		auto rpf = Evaluate(params["block"], &ref);
		LOGALL(params["block"] << "," << rpf.Detected << "," << rpf.Expected << "," << rpf.Correct << endl);
		return;
	}

	if (Contains(params, "config")) //learned similarity functions and aggregation
	{
		Configuration cfg;
		cfg.Load(params["config"]);
		LOGALL(params["config"] << "," << cfg.Aggregate.ToString());
		for (auto f : cfg.sFOI)
			LOGALL("," << simFuncName[cfg.Similarity[f].pFuncID]);
		LOGALL(endl);
		return;
	}

	if (Contains(params, "result") && Contains(params, "label")) //no fold in this function
	{
		INT8 outBuf[1024];
		auto links = LoadResult(params["result"]);
		auto allLinks = LoadReference(params["label"]);
		auto rpf = Evaluate(&links, &allLinks);
		sprintf(outBuf, "%d,%d,%d,%f,%f,%f", rpf.Expected, rpf.Detected, rpf.Correct, rpf.Recall, rpf.Precision, rpf.F1);
		LOGALL(params["result"] << "," << outBuf << endl);
		return; 
	}

	if (Contains(params, "align")) //similarity functions
	{
		auto alg = LoadAlignment(params["align"]);
		LOGALL(params["block"] << "," << GenerateSimilarityFunction(&alg).size() << endl);
		return;
	}

	if (Contains(params, "detail")) //count visited sets
	{
		ifstream fi(params["detail"]);
		INT8 line[4096];
		VECTOR<STRING> sepLine;
		INT32 count = 0;
		while (fi.getline(line, 4096))
			count++;
		LOGALL(params["detail"] + "," << count << endl);
		return;
	}
}

