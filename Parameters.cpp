#include "Parameters.h"
#include <stdlib.h>

VOID Parse(MAP<STRING, STRING> params, INT32 &value, STRING name) { if (Contains(params, name)) value = atoi(params[name].c_str()); }
VOID Parse(MAP<STRING, STRING> params, FLOAT &value, STRING name) { if (Contains(params, name)) value = atof(params[name].c_str()); }
VOID Parse(MAP<STRING, STRING> params, STRING &value, STRING name) { if (Contains(params, name)) value = params[name]; }

VOID AddSlash(STRING &str)
{
	if (str.back() != SLASH[0])
		str += SLASH;
}

SystemParameters::SystemParameters(MAP<STRING, STRING> params)
{
	Parse(params, SourceRepositoryPath, "src");
	Parse(params, TargetRepositoryPath, "trg");
	Parse(params, BlockFile, "block");
	Parse(params, ScoreFile, "score");
	Parse(params, AlignmentFile, "align");
	Parse(params, ConfigurationFile, "config");
	Parse(params, SplitFile, "split");
	Parse(params, LabelFile, "label");	
	Parse(params, Jobname, "name");
	Parse(params, TopSimFunctionCount, "topsim");
	Parse(params, ThreadCount, "thread");
	Parse(params, FilteringFactor, "filtering");
	Parse(params, BlockBuffer, "buffer");
	Parse(params, MinSplitCount, "minsplit");
	Parse(params, MaxSplitCount, "maxsplit");
	Parse(params, TrainingSplit, "training");
	Parse(params, ValidationSplit, "validation");
	Parse(params, PopulationSize, "popsize");
	Parse(params, MaxLoopCount, "loop");
	Parse(params, KFold, "folds");
	Parse(params, Alpha, "alpha");
	Parse(params, Kcover, "kcover");
	Parse(params, Kover, "kover");
	Parse(params, Ktok, "ktok");
	Parse(params, drop, "drop");
	Parse(params, Detail, "detail");
	Parse(params, TopK, "topk");
	
	STRING jobName;
	Parse(params, jobName, "job");
	STRING job[] = { "align", "block", "match", "filter", "split", "collect", "clink", "clearn", "learnblock", "default", "ser" };
	for (INT32 i = 0; i < 11; i++)
	if (jobName.compare(job[i]) == 0)
	{
		Job = i;
		break;
	}

	STRING learnAlgorithm;
	Parse(params, learnAlgorithm, "algorithm");
	STRING algo[] = { "naive", "heuristic", "exhaustive", "infogain", "genetic", "acceptall"};	
	for (INT32 i = 0; i < 6; i++)
	if (learnAlgorithm.compare(algo[i]) == 0)
	{
		LearnAlgorithm = i; 
		break;
	}

	TempDirectory = STRING(".") + SLASH;
	OutputDirectory = STRING(".") + SLASH;
	Parse(params, TempDirectory, "temp"); //if not available, use startup
	Parse(params, OutputDirectory, "output"); //if not available, use startup
	AddSlash(TempDirectory);
	AddSlash(OutputDirectory);
}

