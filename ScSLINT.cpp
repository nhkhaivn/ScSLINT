#include "ScSLINT.h"
#include "Utility.h"
#include "StreamIO.h"
#include "Resolution.h"
#include <stdlib.h>
#include <iostream>

VOID ScSLINT(SystemParameters params)
{
	if (!IsExist(params.OutputDirectory + params.Jobname + ".align"))
		GenerateAlignment(params);
	params.AlignmentFile = params.OutputDirectory + params.Jobname + ".align";
	if (!IsExist(params.OutputDirectory + params.Jobname + ".block"))
		GenerateBlock(params);
	params.BlockFile  = params.OutputDirectory + params.Jobname + ".block";
//	if (!IsExist(params.OutputDirectory + params.Jobname + ".config.default"))
		CreateDefaultConfig(params);
	params.ConfigurationFile = params.OutputDirectory + params.Jobname + ".config.default";
	if (!IsExist(params.OutputDirectory + params.Jobname + ".score"))
		Match(params);
	params.ScoreFile = params.OutputDirectory + params.Jobname + ".score";
	//if (!IsExist(params.OutputDirectory + params.Jobname + ".result"))
		Filter(params);
}

VOID GenerateAlignment(SystemParameters params) //input: src, trg
{
	DataSource dSrc(params.SourceRepositoryPath);
	DataSource dDes(params.TargetRepositoryPath);
	DataSource::LoadTokenID(params.SourceRepositoryPath, params.TargetRepositoryPath);	

	VECTOR<Alignment> alg;
	VECTOR<VECTOR<INT32>> sharedTokens;
	INT64 t1 = TickCount();
	Align(&dSrc, &dDes, params, &alg, &sharedTokens);	

	t1 = TickCount() - t1;
	Save(params.OutputDirectory + params.Jobname + ".align", alg, sharedTokens);	
	LOGALL("Alignment," << params.Jobname << "," << t1 << endl);	
	DataSource::ClearTokenID();
}

VOID GenerateBlock(SystemParameters params) //input: src, trg, alg
{
	VECTOR<Alignment> alg;
	VECTOR<VECTOR<INT32>> sharedTokens;
	LoadStringAlignment(params.AlignmentFile, alg, sharedTokens);
	alg = LoadAlignment(params.AlignmentFile);

	DataSource dSrc(params.SourceRepositoryPath);
	DataSource dDes(params.TargetRepositoryPath);
	DataSource::LoadTokenID(params.SourceRepositoryPath, params.TargetRepositoryPath);
	dSrc.LoadPOS(params.Ktok); dDes.LoadPOS(params.Ktok);

	INT64 t2 = TickCount();
	Block(&dSrc, &dDes, &alg, &sharedTokens, 0, params, params.OutputDirectory + params.Jobname + ".block");
	t2 = TickCount() - t2;

	ifstream fi(params.OutputDirectory + params.Jobname + ".block", ios::in | ios::binary);
	LOGALL("Blocking," << params.Jobname << "," << ReadINT64(&fi) << "," << t2 << endl);
	fi.close();
	DataSource::ClearTokenID();
}

VOID CreateDefaultConfig(SystemParameters params) //input: alg
{
	VECTOR<Alignment> alg = LoadAlignment(params.AlignmentFile);
	VECTOR<Alignment> nalg;
	//Select max score
	MAP<INT32, INT32> src;
	for (INT32 i = 0; i < alg.size(); i++)
	if (!Contains(src, alg[i].Source) || (Contains(src, alg[i].Source) && alg[src[alg[i].Source]].Coverage < alg[i].Coverage))
		src[alg[i].Source] = i;
	for (auto en : src)
		nalg.push_back(alg[en.second]);
	auto simFuncs = GenerateSimilarityFunction(&nalg);
	Configuration cfg(simFuncs);
	cfg.FilteringFactor = params.FilteringFactor;
	cfg.Save(params.OutputDirectory + params.Jobname + ".config.default");
}

VOID Match(SystemParameters params) //input: src, trg, block, config
{	
	VECTOR<Alignment> alg;
	alg = LoadAlignment(params.AlignmentFile);	
	MAP<INT32, INT32> pSrc, pDes;
	for (auto a : alg)
	{
		pSrc[a.Source] = a.Type;
		pDes[a.Target] = a.Type;
	}	
	
	DataSource::LoadTokenID(params.SourceRepositoryPath, params.TargetRepositoryPath);
	DataSource dSrc(params.SourceRepositoryPath);
	DataSource dDes(params.TargetRepositoryPath);	
	dSrc.LoadSPO(pSrc);	dDes.LoadSPO(pDes);
	dSrc.LoadIDF(); dDes.LoadIDF();

	Configuration cfg;
	cfg.Load(params.ConfigurationFile);	
	cout << "Number of similarity functions:" <<cfg.sFOI.size() << endl;

	INT64 t3 = TickCount();
	//FastMatch(&dSrc, &dDes, &cfg, params, params.OutputDirectory + params.Jobname + ".score");
	Match(&dSrc, &dDes, &cfg, params, params.OutputDirectory + params.Jobname + ".score");
	t3 = TickCount() - t3;
	LOGALL("Matching," << params.Jobname << "," << t3 << endl);

	DataSource::ClearTokenID();
}

VOID Filter(SystemParameters params)
{
	Configuration cfg;
	if (params.ConfigurationFile.compare("")!=0)
		cfg.Load(params.ConfigurationFile);	

	cout << endl << "Threshold="<< cfg.FilteringThreshold << endl;
	if (params.TopK == 0)
	{
		auto links = StableFiltering(params.ScoreFile, cfg.FilteringFactor, cfg.FilteringThreshold);
		Save(params.OutputDirectory + params.Jobname + ".result", &links);
	}
	else
	{
		auto links = TopKFiltering(params.ScoreFile, params.TopK, cfg.FilteringThreshold);
		Save(params.OutputDirectory + params.Jobname + ".result", &links);
	}	
}

VOID Save(STRING strFile, VECTOR<pair<INT64,FLOAT>> *links)
{
	ofstream fo(strFile);
	for (auto en : *links)
		fo << (INT32)(en.first & 0xFFFFFFFF) << "\t" << (INT32)(en.first >> 32) << "\t" << en.second << endl;
	fo.close();
}

VECTOR<pair<INT64, FLOAT>> LoadResult(STRING strFile)
{
	VECTOR<pair<INT64, FLOAT>> links;
	ifstream fi(strFile);
	INT8 line[4096];
	while (fi.getline(line, 4096))
	{
		auto sepLine = ReadSeparatedLine(line, "\t");
		links.push_back(pair<INT64, FLOAT>(((INT64)atoi(sepLine[1].c_str()) << 32) | atoi(sepLine[0].c_str()), (FLOAT)atof(sepLine[2].c_str())));
	}
	fi.close();
	return links;
}
