#pragma once

#include "Core.h"

//JOB
#define ALIGN		0
#define BLOCK		1
#define MATCH		2
#define FILTER		3
#define SPLIT		4
#define COLLECT		5
#define CLINK		6
#define CLEARN		7
#define LEARNBLOCK	8
#define DEFAULT		9
#define SER			10

//File format
#define CSV			0
#define ARFF		1

//LEARN
#define NAIVE		0
#define HEURISTIC	1
#define EXHAUSTIVE	2
#define INFOGAIN	3
#define GENETIC		4
#define ACCEPTALL	5

struct SystemParameters
{
	//Note: all path are for input, not for output
	STRING SourceRepositoryPath;
	STRING TargetRepositoryPath;
	STRING BlockFile;
	STRING ScoreFile;
	STRING AlignmentFile;
	STRING SplitFile;
	STRING LabelFile;
	STRING ConfigurationFile;
	STRING OutputDirectory;
	STRING TempDirectory;

	INT32 Job = -1;
	STRING Jobname = STRING("unnamed");
	INT32 LearnAlgorithm = -1;
	INT32 TopSimFunctionCount = 16;
	INT32 ThreadCount = 16;
	FLOAT FilteringFactor = 1.0F;
	INT32 BlockBuffer = 0x800000;
	INT32 MaxSplitCount = 0x7FFFFFF;
	INT32 MinSplitCount = 0;
	INT32 Detail = 0;
	INT32 TopK = 0; //topK filtering

	//learning split
	FLOAT TrainingSplit = 0.8;   //Note: Data is divided into TrainingSet and Testset by this ratio
	FLOAT ValidationSplit = 0.2; //Note: TrainingSet is divided into Training & Validation by this ratio. Final training example is TrainingSplit*ValidataionSplit
	INT32 KFold = -1;

	//Learn Block
	FLOAT drop = 1.0F;

	//Genetic algorithm
	INT32 PopulationSize = 100;
	INT32 MaxLoopCount = 100;

	//Predicate alignment and blocking
	FLOAT Alpha = 0.4F;
	INT32 Kcover = 4;
	INT32 Kover = 4;
	INT32 Ktok = 1;
	FLOAT KoverRAW = 0.05; //very small threshold to ignore trivial predicate alignments
	SystemParameters(MAP<STRING, STRING> params);
};

