#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include "Core.h"
#include "StreamIO.h"
#include "Evaluator.h"
#include "Parameters.h"
#include <fstream>
#include <mutex>

using namespace std;

struct Alignment
{
	INT32 Source;
	INT32 Target;
	INT32 Type;
	INT32 IsLongString;
	FLOAT Coverage;
	Alignment() {}
	Alignment(INT32 src, INT32 trg, INT32 type, INT32 isINT64, FLOAT cov){ Source = src; Target = trg; Type = type; IsLongString = isINT64; Coverage = cov; }
};

struct ScoreEntry
{
	INT32 Source;
	INT32 Target;
	FLOAT Score;
};

struct ScoreEntryEx
{
	INT32 Source;
	INT32 Target;
	FLOAT Weight;
	VECTOR<FLOAT> Similarity;
};

struct LinearAggregator
{
	INT32 K, useBoolean, useAverage, useWeighting;
	FLOAT Aggregate(ScoreEntryEx* ex, VECTOR<INT32>* functionID, VECTOR<FLOAT>* functionThresholds);
	LinearAggregator(){};
	LinearAggregator(INT32 mask);
	INT32 GetMask();
	STRING ToString();
};

class IndexedStream
{
public:
	INT32 Count;
private:
	ifstream input;
	INT32 current;
	VECTOR<INT64> index;

public:
	VOID Load(STRING strDataFile, STRING strIndexFile);
	ifstream* GetStream(INT32 ID = -1);
	INT8* Read(INT32 ID = -1);
	~IndexedStream();
	VOID ResetPointer();
};

class SPOEntry
{
public:
	MAP<INT32, VECTOR<VECTOR<INT32>>> Token;
	MAP<INT32, VECTOR<FLOAT>> Number;
	MAP<INT32, VECTOR<STRING>> Original;
	MAP<INT32, FLOAT> TF;
	INT32 TripleCount;
};

struct PredicateInfo
{
	STRING Name;
	INT32 ID;
	INT32 Frequency;
	INT32 ParentCount;
	FLOAT MeanLength;
	FLOAT StdLength;
	INT32 Type;
	FLOAT Coverage;
	FLOAT Diversity;
};

class DataSource
{
public:
	INT32 SPOEntryCount;
	INT32 MaxTokenCount;
	INT32 TotalTokenCount;
	INT32 MaxTripleCount;
	INT32 TotalTripleCount;
	MAP<INT32, FLOAT> IDF;

private:
	static MAP<STRING, INT32> tokenID;
	IndexedStream iSPO, iPO, iPOS;
	MAP<INT32, INT32> poI;
	STRING indexDirectory;

public:
	DataSource(){};
	DataSource(STRING indexDirectory);
	VOID ResetPointer();
	static VOID LoadTokenID(STRING strSouceindexDirectory, STRING strTargetIndexDirectory);
	static VOID ClearTokenID();
	VOID LoadIDF();
	DataSource* Duplicate();

	VOID LoadSPO(MAP<INT32, INT32> poI);
	VOID LoadPO(INT32 nFirstTokenCount = -1);
	VOID LoadPOS(INT32 nFirstTokenCount = -1);

	SPOEntry ReadSPOEntry(INT32 ID = -1);
	VECTOR<INT32> ReadPOEntry(INT32 ID = -1);
	VECTOR<STRING> ReadPOEntryRaw(INT32 ID = -1);
	MAP<INT32, VECTOR<INT32>> ReadPOSEntry(INT32 ID = -1);
	MAP<STRING, VECTOR<INT32>> ReadPOSEntryRaw(INT32 ID = -1);
	VECTOR<PredicateInfo> ReadPredicateInfo();

	VOID readSTRING(SPOEntry &en, INT32 nProperty, INT8* &ptr);
	VOID readNumber(SPOEntry &en, INT32 nProperty, INT8* &ptr);
	VOID readOriginal(SPOEntry &en, INT32 nProperty, INT8* &ptr);
	VECTOR<VOID(DataSource::*) (SPOEntry&, INT32, INT8*&)> readPropertyFunction;
};

class BlockReader
{
private:
	ifstream fi;
	INT64 *buffer;
	INT32 bufSize = 0x8000; //at initial
	INT64 readSize;
	INT64 entryCount;
	STRING blockFile;

public:
	BlockReader(){};
	BlockReader(STRING strBlockFile);
	BlockReader* Duplicate();
	VOID ResetPointer();
	INT32 GetBufferSize();
	VOID SetBufferSize(INT64 bufSize);
	INT64 GetEntryCount();
	VECTOR<pair<INT32, INT32>> Read();
	VOID Select(INT64 Start, INT64 Count);  //Set pointer between [Start, Start+Count] , entryCount is changed also
	~BlockReader();
};

struct SimilarityFunction
{
	INT32 pSrc, pDes, pFuncID;
	FLOAT Calculate(DataSource*, DataSource*, SPOEntry*, SPOEntry*);	
};


struct Configuration
{
	VECTOR<INT32> sFOI; //simFunction of interest
	LinearAggregator Aggregate;
	FLOAT FilteringThreshold = 0;
	FLOAT FilteringFactor = 1;
	RPF TrainingPerformance;
	RPF ValidationPerformance;
	VECTOR<struct SimilarityFunction> Similarity;
	VECTOR<FLOAT> SimilarityThreshold;

	Configuration(){}
	Configuration(const Configuration &config);
	Configuration(VECTOR<SimilarityFunction> &simFunctions);
	Configuration(VECTOR<SimilarityFunction> &simFunctions, FLOAT filteringFactor, LinearAggregator LinearAggregator);
	VOID Load(STRING strFile);
	VOID Save(STRING strFile);
	STRING ToString();
};


//Similarity.cpp =============================================================================================================
FLOAT ExactMatching(DataSource *dSrc, DataSource *dDes, SPOEntry *src, SPOEntry *des, INT32 pSrc, INT32 pDes);
FLOAT ReverseDifference(DataSource *dSrc, DataSource *dDes, SPOEntry *src, SPOEntry *des, INT32 pSrc, INT32 pDes);
FLOAT Levenshtein(DataSource *dSrc, DataSource *dDes, SPOEntry *src, SPOEntry *des, INT32 pSrc, INT32 pDes);
FLOAT JaroWinkler(DataSource *dSrc, DataSource *dDes, SPOEntry *src, SPOEntry *des, INT32 pSrc, INT32 pDes);
FLOAT LongestCommonSubstring(DataSource *dSrc, DataSource *dDes, SPOEntry *src, SPOEntry *des, INT32 pSrc, INT32 pDes);
FLOAT LongestCommonPrefix(DataSource *dSrc, DataSource *dDes, SPOEntry *src, SPOEntry *des, INT32 pSrc, INT32 pDes);
FLOAT Jaccard(DataSource *dSrc, DataSource *dDes, SPOEntry *src, SPOEntry *des, INT32 pSrc, INT32 pDes);
FLOAT TFIDFCosine(DataSource *dSrc, DataSource *dDes, SPOEntry *src, SPOEntry *des, INT32 pSrc, INT32 pDes);
FLOAT ModifiedBM25(DataSource *dSrc, DataSource *dDes, SPOEntry *src, SPOEntry *des, INT32 pSrc, INT32 pDes);
FLOAT JaccardBM25(DataSource *dSrc, DataSource *dDes, SPOEntry *src, SPOEntry *des, INT32 pSrc, INT32 pDes);

typedef FLOAT(*SimilarityFunc)(DataSource*, DataSource*, SPOEntry*, SPOEntry*, INT32 pSrc, INT32 pDes);
extern SimilarityFunc simFuncID[];
extern STRING simFuncName[];
extern INT32 simFuncCount;

//Alignment.cpp
VOID Align(DataSource *dSrc, DataSource *dDes, SystemParameters params, VECTOR<Alignment> *alg, VECTOR<VECTOR<INT32>> *sharedTokens);
VOID Save(STRING strFile, VECTOR<Alignment> &alg, VECTOR<VECTOR<INT32>> &sharedTokens);
VOID Save(STRING strFile, VECTOR<Alignment> &alg, VECTOR<VECTOR<INT32>> &sharedTokens, VECTOR<INT32> sharedIndex);
VECTOR<Alignment> LoadAlignment(STRING strFile);
VOID LoadStringAlignment(STRING strFile, VECTOR<Alignment> &alg, VECTOR<VECTOR<INT32>> &sharedTokens);
VECTOR<SimilarityFunction> GenerateSimilarityFunction(VECTOR<Alignment> *alg);
VECTOR<INT32> GenerateSimilarityFunctionMap(VECTOR<Alignment> *alg);

//Blocking.cpp =============================================================================================================
VOID Block(DataSource* dSrc, DataSource* dDes, VECTOR<Alignment> *alg, VECTOR<VECTOR<INT32>> *sharedTokens, SET<INT32> *srcIns, SystemParameters params, STRING strOutputFile); 

//Matching.cpp =============================================================================================================
VECTOR<ScoreEntryEx> Match(DataSource *dSrc, DataSource *dDes, BlockReader *pBlock, Configuration *config, SystemParameters params);
VOID Match(DataSource *dSrc, DataSource *dDes, Configuration *config, SystemParameters params, STRING strOutputFile);
VECTOR<ScoreEntryEx> LoadScoreEntryEx(STRING strFile);

//FastMatching.cpp
VOID FastMatch(DataSource *dSrc, DataSource *dDes, Configuration *config, SystemParameters params, STRING strOutputFile);

//LinearAggregator.cpp
FLOAT Linear(ScoreEntryEx *entry, VECTOR<INT32> *sFOI, VECTOR<FLOAT> *sFOIThreshold, INT32 useBoolean, INT32 useAverage, INT32 useWeighting, INT32 K);

//Filter.cpp =============================================================================================================
VECTOR<pair<INT64, FLOAT>> StableFiltering(STRING strScoreFile, FLOAT filteringFactor, FLOAT filteringThreshold);
VECTOR<pair<INT64, FLOAT>> TopKFiltering(STRING strScoreFile, INT32 topK, FLOAT filteringThreshold);
VECTOR<pair<INT64, FLOAT>> StableFiltering(VECTOR<ScoreEntry> *ens, FLOAT filteringFactor, FLOAT filteringThreshold);
