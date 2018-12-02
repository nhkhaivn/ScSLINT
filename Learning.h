#pragma once

#include "Core.h"
#include "Resolution.h"

#define TRAININGSET			1
#define VALIDATIONSET		2
#define TESTSET				4
#define FULLSET				7	

//Learning framework functions

class RepositorySpliter
{
private:
	VECTOR<INT32> negativeSourceInstances;
	VECTOR<INT32> positiveSourceInstances;
	VECTOR<INT32> splitedSet[3];
	INT32 Folds, currentFold; FLOAT trainingSplit, validationSplit;

public:
	RepositorySpliter() {}
	RepositorySpliter(STRING strFile);
	VOID BeginSplit(SET<INT64> refLinks, INT32 nSrcInsCount, FLOAT trainingSplit, FLOAT validationSplit, INT32 folds);
	VOID Split();
	VOID Save(STRING strFile); //Save current split 
	VOID Load(STRING strFile); //Load saved split 
	SET<INT32> GetInstanceList(INT32 setID); //Get the split by set ID: TRAININGSET, VALIDATIONSET, TESTSET, FULLSET
};

//Split data
VOID GenerateRepositorySplit(SystemParameters params, INT32 nRepeat);
VOID GenerateBlock(SystemParameters params, INT32 setID); 

//learn blocking model
VOID LearnBlock(DataSource* dSrc, DataSource* dDes, VECTOR<Alignment> *alg, VECTOR<VECTOR<INT32>> *sharedTokens, SET<INT32> *srcIns, SET<INT64>* refLinks, SystemParameters params, STRING strOutputFile);

//Support functions
VECTOR<Configuration> SortSimilarityFunctions(VECTOR<ScoreEntryEx> *ens, Configuration *config, SET<INT64> *refLinks);
VOID FindThreshold(VECTOR<ScoreEntryEx> *ens, Configuration *config, SET<INT64> *refLinks);
INT32 MapSimilarityFunctions(VECTOR<Configuration>* config, VECTOR<INT32>* map, INT32 topSim);
RPF Validate(VECTOR<ScoreEntryEx> *ens, SET<INT64> *refLink, Configuration* config); //during learning

//Performance evaluation (output)
RPF Evaluate(SystemParameters params, INT32 setID, STRING strFilterFile);

VOID cLearn(SystemParameters params); //Learning algorithms
VOID Heuristic(VECTOR<ScoreEntryEx> *ensTrain, SET<INT64> *refLinksTrain, VECTOR<ScoreEntryEx> *ensValidation, SET<INT64> *refLinksValidation, Configuration* config, SystemParameters params, STRING strOutputFile, INT32 outID);
VOID Naive(VECTOR<ScoreEntryEx> *ensTrain, SET<INT64> *refLinksTrain, VECTOR<ScoreEntryEx> *ensValidation, SET<INT64> *refLinksValidation, Configuration* config, SystemParameters params, STRING strOutputFile, INT32 outID);
VOID AcceptAll(VECTOR<ScoreEntryEx> *ensTrain, SET<INT64> *refLinksTrain, VECTOR<ScoreEntryEx> *ensValidation, SET<INT64> *refLinksValidation, Configuration* config, SystemParameters params, STRING strOutputFile, INT32 outID);
VOID InfoGain(VECTOR<ScoreEntryEx> *ensTrain, SET<INT64> *refLinksTrain, VECTOR<ScoreEntryEx> *ensValidation, SET<INT64> *refLinksValidation, Configuration* config, SystemParameters params, STRING strOutputFile, INT32 outID);
VOID Genetic(VECTOR<ScoreEntryEx> *ensTrain, SET<INT64> *refLinksTrain, VECTOR<ScoreEntryEx> *ensValidation, SET<INT64> *refLinksValidation, Configuration* config, SystemParameters params, STRING strOutputFile, INT32 outID);
VOID Exhaustive(VECTOR<ScoreEntryEx> *ensTrain, SET<INT64> *refLinksTrain, VECTOR<ScoreEntryEx> *ensValidation, SET<INT64> *refLinksValidation, Configuration* config, SystemParameters params, STRING strOutputFile, INT32 outID);
typedef VOID(*Learner)(VECTOR<ScoreEntryEx> *ensTrain, SET<INT64> *refLinksTrain, VECTOR<ScoreEntryEx> *ensValidation, SET<INT64> *refLinksValidation, Configuration* config, SystemParameters params, STRING strOutputFile, INT32 outID);

