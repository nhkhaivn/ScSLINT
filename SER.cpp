#include "Utility.h"
#include "ScSLINT.h"
#include <iostream>
#include <memory>
#include <cstring>
#include <stdlib.h>

VOID ScoreToFeature(SystemParameters params);

//INT32 endsWith(std::string const &fullString, std::string const &ending)
//{
//	return fullString.length() >= ending.length() && (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending)); 
//}
//
VOID ser(SystemParameters params)
{
	ScoreToFeature(params);
}

VOID ScoreToFeature(SystemParameters params)
{
	ofstream foFtr(params.OutputDirectory + params.Jobname + ".feature", ios::binary);
	ofstream foInfo(params.OutputDirectory + params.Jobname + ".info");
	//ofstream foInfo(params.OutputDirectory + params.Jobname + ".info", ios::binary);
	auto filelength = FileLength(params.ScoreFile);
	auto allRefs = LoadReference(params.LabelFile);
	ifstream fi(params.ScoreFile, ios::binary);

	INT64 count = ReadINT64(&fi);
	INT32 ftrsize = ((filelength - sizeof(INT64) /*count*/) / count - sizeof(INT32)* 3 /*src,des,length*/) / sizeof(FLOAT);
	INT32 infosize = 3 /*src,des,label*/;
	//foInfo.write((INT8*)&count, sizeof(INT64));
	//foInfo.write((INT8*)&infosize, sizeof(INT32));
	foFtr.write((INT8*)&count, sizeof(INT64));
	foFtr.write((INT8*)&ftrsize, sizeof(INT32));

	FLOAT *ftr = new FLOAT[ftrsize];
	INT32 info[3];
	Loop(count);
	for (INT32 i = 0; i < count; i++)
	{
		//clear
		memset(info, 0, sizeof(INT32) * infosize);
		memset(ftr, 0, sizeof(FLOAT) * ftrsize);

		//Info
		fi.read((INT8*)info, sizeof(INT32)* 2);

		INT64 link = ((INT64)info[1] << 32) | info[0];
		info[2] = Contains(allRefs, link);
		foInfo << info[0] << "," << info[1] << "," << info[2] << endl;
		//foInfo.write((INT8*)info, sizeof(INT32)* 3);

		//Feature
		fi.read((INT8*)ftr, sizeof(FLOAT));				//weight
		INT32 length = ReadINT32(&fi);					//ignore length
		fi.read((INT8*)&ftr[1], sizeof(FLOAT)* length);	//similarities
		foFtr.write((INT8*)ftr, sizeof(FLOAT)* ftrsize);

		//check
		if (length != ftrsize - 1)
		{
			LOGALL("ERROR! size does not match, at loop:" << i << "(" << length << " vs " << ftrsize << ")" << endl);
			system("pause");
		}
		Loop();
	}

	delete[] ftr;
	fi.close();
	foInfo.close();
	foFtr.close();
}
//
//VOID FeatureToCSV(STRING strInput, STRING strOutput)
//{
//	ifstream fi(strInput, ios::binary);
//	ofstream fo(strOutput);
//	INT64 count = ReadINT64(&fi);
//	INT32 size = ReadINT32(&fi);
//	FLOAT buf[1024];
//	Loop(count);
//	for (INT32 i = 0; i < count; i++)
//	{
//		fi.read((INT8*)buf, sizeof(FLOAT)*size);
//		for (INT32 j = 0; j < size - 1; j++)
//			fo << buf[j] << ",";
//		fo << buf[size - 1] << endl;
//		Loop();
//	}
//	fi.close();
//	fo.close();
//}

//#include "ScSLINT.h"
//if (params.LearnAlgorithm == SVM || params.LearnAlgorithm == J48 || params.LearnAlgorithm == ADABOOST)
//{
//	STRING wekaClassifier[] = { "weka.classifiers.functions.SMO", "weka.classifiers.trees.J48", "weka.classifiers.meta.AdaBoostM1"};
//	STRING strTraining = strOutputDir + SLASH + params.Jobname + "_training.arff";
//	STRING strTest = strOutputDir + SLASH + params.Jobname + "_test.arff";
//	STRING wekaCall = "java -Xmx%s -classpath %sweka.jar weka.classifiers.meta.ThresholdSelector -C 1 -W %s -t %s -d %s > %s";
//	INT64 timer1 = TickCount();
//	GenerateExternalFile(params, strTraining, strTest);
//	timer1 = TickCount() - timer1;
//	INT64 timer2 = TickCount();
//	INT8 outBuf[4096];
//	sprintf(outBuf, wekaCall.c_str(), XMX,
//		(params.StartupDirectory + SLASH).c_str(), wekaClassifier[params.LearnAlgorithm - SVM].c_str(), strTraining.c_str(),
//		(strOutputDir + SLASH + params.Jobname + ".model").c_str(), (strOutputDir + SLASH + params.Jobname + "_training.log").c_str());

//	system(outBuf);
//	timer2 = TickCount() - timer2;
//	INT64 timer3 = TickCount();
//	sprintf(outBuf, wekaCall.c_str(), XMX,
//		(params.StartupDirectory + SLASH).c_str(), wekaClassifier[params.LearnAlgorithm - SVM].c_str(), strTest.c_str(),
//		(strOutputDir + SLASH + params.Jobname + ".model").c_str(), (strOutputDir + SLASH + params.Jobname + "_test.log").c_str());
//	system(outBuf);
//	timer3 = TickCount() - timer3;

//	//collect result
//	ifstream fi(strOutputDir + SLASH + params.Jobname + "_test.log");
//	while (fi.getline(outBuf, 4096))
//	{
//		auto set = ReadSeparatedLine(outBuf, " ");
//		if (set.size() > 0 && set[0].compare("a") == 0 && set[1].compare("b") == 0 && set[2].compare("<--") == 0)
//		{
//			INT32 confusion[2][2] = { 0 };
//			fi.getline(outBuf, 4096);
//			auto res = ReadSeparatedLine(outBuf, " ");
//			confusion[0][0] = atoi(res[0].c_str());
//			confusion[0][1] = atoi(res[1].c_str());

//			fi.getline(outBuf, 4096);
//			res = ReadSeparatedLine(outBuf, " ");
//			confusion[1][0] = atoi(res[0].c_str());
//			confusion[1][1] = atoi(res[1].c_str());

//			RPF eval = { 0 };
//			eval.Expected = confusion[0][0] + confusion[0][1];
//			eval.Detected = confusion[0][0] + confusion[1][0];
//			eval.Correct = confusion[0][0];
//			if (eval.Correct > 0)
//			{
//				eval.Precision = (FLOAT)eval.Correct / eval.Detected;
//				eval.Recall = (FLOAT)eval.Correct / eval.Expected;
//				eval.F1 = eval.Precision * eval.Recall * 2 / (eval.Precision + eval.Recall);
//			}				
//			LOGALL("Recall:" << eval.Recall << " Precision:" << eval.Precision << " F1:" << eval.F1 << " Training:"<< timer2 << " Test:" << timer3 << endl);
//			sprintf(outBuf, "%s%sresult_%s", strOutputDir.c_str(), SLASH, params.Jobname.c_str());
//			ofstream fo(outBuf, ios::out | ios::app);
//			fo << eval.Recall << "," << eval.Precision << "," << eval.F1 << "," << timer2 << "," << timer3 << endl;
//			fo.close();				
//			break;
//		}
//	}
//	fi.close();
//}
//else
//{

//VOID SER(SystemParameters params, STRING strOutputDir)
//{
//	LOGALL("Running test " << params.Jobname << endl);	
//	STRING strBlockTrainingFile = params.TempDirectory + params.Jobname + ".block_Training";
//	STRING strBlockTestFile = params.TempDirectory + params.Jobname + ".block_Test";
//	STRING strSimTrainingFile = strOutputDir + params.Jobname + ".sim_Training.arff";	
//	STRING strSimTestFile = strOutputDir + params.Jobname + ".sim_Test.arff";
//	INT8* label[3] = { "no", "yes", "0" };
//
//	//Load ====================================================================================
//	DataSource dSrc(params.SourceRepositoryPath);
//	DataSource dDes(params.TargetRepositoryPath);	
//	DataSource::LoadTokenID(params.SourceRepositoryPath, params.TargetRepositoryPath);	
//	RepositorySpliter rep(params.SplitFile);
//	VECTOR<Alignment> alg;
//	VECTOR<VECTOR<INT32>> sharedTokens;
//	LoadStringAlignment(params.AlignmentFile, alg, sharedTokens);
//	alg = LoadAlignment(params.AlignmentFile);
//	auto simFunctions = GenerateSimilarityFunction(&alg);
//	Configuration def(simFunctions);
//	MAP<INT32, INT32> pSrc, pDes;
//	for (auto a : alg)
//	{
//		pSrc[a.Source] = a.Type;
//		pDes[a.Target] = a.Type;
//	}
//	dSrc.LoadPOS(params.Ktok); dDes.LoadPOS(params.Ktok);
//	dSrc.LoadSPO(pSrc);	dDes.LoadSPO(pDes);
//	dSrc.LoadIDF(); dDes.LoadIDF();
//	auto allLinks = LoadReference(params.LabelFile);
//	SET<INT64> refLinks;
//	SET<INT32> srcIns;
//	INT64 t;
//	//Training 
//	srcIns = rep.GetInstanceList(TRAININGSET | VALIDATIONSET);
//	refLinks = AdaptToSplit(&allLinks, &srcIns);
//	//t = TickCount();
//	//if (!IsExist(strBlockTrainingFile))
//	//Block(&dSrc, &dDes, &alg, &sharedTokens, &srcIns, params, strBlockTrainingFile);
//	//t = TickCount() - t;	
//	//LOGALL(strBlockTrainingFile << "," << t << endl);
//	//
//	BlockReader bBlock(strBlockTrainingFile);
//	dSrc.ResetPointer(); dDes.ResetPointer();
//	GenerateExternalFileHeader(strSimTrainingFile, simFunctions.size() + 3, ARFF); //+3: source, target, weight
//	t = TickCount();
//	MatchAndOutputDetail(&dSrc, &dDes, &bBlock, &def, &refLinks, params, label, strSimTrainingFile);	
//	t = TickCount() - t;
//	LOGALL(strSimTrainingFile << "," << t << endl);
//
//	//refLinks.clear();
//	//dSrc.ResetPointer(); dDes.ResetPointer();
//	
//	//Test
//	srcIns = rep.GetInstanceList(TESTSET);
//	refLinks = AdaptToSplit(&allLinks, &srcIns);
//	////t = TickCount();
//	////Block(&dSrc, &dDes, &alg, &sharedTokens, &srcIns, params, strBlockTestFile);
//	////t = TickCount() - t;
//	////LOGALL(strBlockTestFile << "," << t << endl);
//	//
//	BlockReader bBlockTest(strBlockTestFile);
//	dSrc.ResetPointer(); dDes.ResetPointer();
//	GenerateExternalFileHeader(strSimTestFile, simFunctions.size() + 3, ARFF); //+3: source, target, weight
//	t = TickCount();
//	MatchAndOutputDetail(&dSrc, &dDes, &bBlockTest, &def, &refLinks, params, label, strSimTestFile);
//	//t = TickCount() - t;
//	//LOGALL(strSimTestFile << "," << t << endl);
//
//	DataSource::ClearTokenID();
//}
//
//
//INT32 GreaterThanCFG(Configuration a, Configuration b)
//{
//	return a.TrainingPerformance.F1 > b.TrainingPerformance.F1;
//}
//
//
//INT32 GreaterThanPIF(pair<INT64, FLOAT> a, pair<INT64, FLOAT> b)
//{
//	return a.second > b.second;
//}
//
//VECTOR<Configuration> SortSimilarityFunctions(VECTOR<ScoreEntryEx> *ens, Configuration *config, SET<INT64> *refLinks)
//{
//	INT32 funcSize = (INT32)config->Similarity.size();
//	VECTOR<Configuration> allConfig;
//	allConfig.reserve(funcSize);
//	for (INT32 k = 0; k < funcSize; k++)
//	{
//		Configuration cfg(config->Similarity, config->FilteringFactor, config->Aggregate);
//		cfg.sFOI.push_back(k);
//		FindThreshold(ens, &cfg, refLinks);
//		config->SimilarityThreshold[k] = cfg.FilteringThreshold;
//		allConfig.push_back(cfg);
//	}
//	sort(allConfig.begin(), allConfig.end(), GreaterThanCFG);
//	return allConfig;
//}
//
//INT32 MapSimilarityFunctions(VECTOR<Configuration>* config, VECTOR<INT32>* map, INT32 topSim)
//{
//	INT32 toSelect = MIN((INT32)config->size(), topSim);
//	map->resize(config->at(0).Similarity.size());
//	VECTOR<INT32> candidate;
//	for (INT32 k = 0; k < map->size(); k++)
//		(*map)[k] = -1;
//	for (INT32 k = 0; k < toSelect && config->at(k).TrainingPerformance.F1 > 0; k++)
//		(*map)[config->at(k).sFOI[0]] = k;
//	return toSelect;
//}
//
//VOID FindThreshold(VECTOR<ScoreEntryEx> *ens, Configuration *config, SET<INT64> *refLinks)
//{
//	VECTOR<ScoreEntry> score(ens->size());
//	INT32 i = 0;
//	for (auto en = ens->begin(); en != ens->end(); en++, i++)
//	{
//		score[i].Source = en->Source;
//		score[i].Target = en->Target;
//		score[i].Score = config->Aggregate.pFunc(&*en, &config->sFOI, &config->SimilarityThreshold, config->Aggregate.useBoolean, config->Aggregate.useAverage, config->Aggregate.useWeighting);
//	}
//	auto links = StableFiltering(&score, config->FilteringFactor, config->FilteringThreshold);
//
//	sort(links.begin(), links.end(), GreaterThanPIF);
//	INT32 toSelect = (INT32)MIN(refLinks->size(), links.size());
//	INT32 nCorrect = 0;
//	FLOAT minPositive = 0;
//	FLOAT maxNegative = -1;
//	for (INT32 i = 0; i < toSelect; i++)
//	if (RefContains(refLinks, links[i].first))
//	{
//		nCorrect++;
//		minPositive = links[i].second;
//	}
//	else if (maxNegative == -1)
//		maxNegative = links[i].second;
//	//config->FilteringThreshold = (maxNegative + minPositive) / 2;
//	config->FilteringThreshold = minPositive;
//	config->TrainingPerformance.Expected = refLinks->size();
//	config->TrainingPerformance.Detected = toSelect;
//	config->TrainingPerformance.Correct = nCorrect;
//	if (nCorrect > 0)
//	{
//		config->TrainingPerformance.Recall = (FLOAT)nCorrect / refLinks->size();
//		config->TrainingPerformance.Precision = (FLOAT)nCorrect / toSelect;
//		config->TrainingPerformance.F1 = 2 * config->TrainingPerformance.Recall * config->TrainingPerformance.Precision / (config->TrainingPerformance.Recall + config->TrainingPerformance.Precision);
//	}
//}
//
//Configuration Validate(DataSource *dSrc, DataSource* dDes, BlockReader *bBlock, SET<INT64> *refValidation, Configuration* config, INT32 configCount, SystemParameters params, STRING strOutputDir)
//{
//	INT8 outBuf[1024];
//	INT32 maxID = -1;
//	for (INT32 j = 0; j < configCount; j++)
//	{
//		bBlock->ResetPointer(); dSrc->ResetPointer(); dDes->ResetPointer();
//		sprintf(outBuf, "%svalidation_%s", strOutputDir.c_str(), params.Jobname.c_str());
//		Match(dSrc, dDes, bBlock, config + j, params, outBuf);
//		auto links = StableFiltering(outBuf, config[j].FilteringFactor, config[j].FilteringThreshold);		
//		config[j].ValidationPerformance = Evaluate(&links, refValidation);
//		LOGALL(j << "," << config[j].ToString() << endl);
//		if (maxID == -1 || config[j].ValidationPerformance.F1 >= config[maxID].ValidationPerformance.F1)
//			maxID = j;
//		CMD(RM + outBuf); //delete file		
//	}
//	return config[maxID];
//}

//VOID GenerateExternalFile(SystemParameters params, STRING strTraining, STRING strTest)
//{
//	DataSource dSrc(params.SourceRepositoryPath);
//	DataSource dDes(params.TargetRepositoryPath);
//	DataSource::LoadTokenID(params.SourceRepositoryPath, params.TargetRepositoryPath);
//
//	auto alg = LoadAlignment(params.AlignmentFile);
//	MAP<INT32, INT32> pSrc, pDes;
//	for (auto a : alg)
//	{
//		pSrc[a.Source] = a.Type;
//		pDes[a.Target] = a.Type;
//	}
//	dSrc.LoadSPO(pSrc);	dDes.LoadSPO(pDes);
//	dSrc.LoadIDF(); dDes.LoadIDF();
//
//	BlockReader bBlock(params.BlockFile);
//	bBlock.Load(params.SplitFile);
//
//	auto refAll = LoadReference(params.LabelFile);
//	SET<INT64> refLinks[2];
//	for (INT64 pr : refAll)
//	{
//		INT32 target = bBlock.ContainedBy((INT32)pr) - 1;
//		refLinks[MAX(0, target)].insert(pr);
//	}
//	auto simFunctions = GenerateSimilarityFunction(alg);
//
//	//Header
//	ofstream fo;
//	fo.open(strTraining, ios::out);
//	fo << "@RELATION " << params.Jobname << endl;
//	for (INT32 i = 0; i <= simFunctions.size(); i++)
//		fo << "@ATTRIBUTE S" << i << " NUMERIC" << endl;
//	fo << "@ATTRIBUTE class {Y,N}" << endl;
//	fo << "@DATA" << endl;
//	fo.close();
//	fo.open(strTest, ios::out);
//	fo << "@RELATION " << params.Jobname << endl;
//	for (INT32 i = 0; i <= simFunctions.size(); i++)
//		fo << "@ATTRIBUTE S" << i << " NUMERIC" << endl;
//	fo << "@ATTRIBUTE class {Y,N}" << endl;
//	fo << "@DATA" << endl;
//	fo.close();
//
//	//Data
//	Configuration def(simFunctions);
//	//training data
//	bBlock.ResetPointer(TRAININGSET | VALIDATIONSET); dSrc.ResetPointer(); dDes.ResetPointer();
//	VECTOR<ScoreEntryEx> ens = Match(&dSrc, &dDes, &bBlock, &def, params); //match detail
//	auto ids = SelectBalanceDataset(&ens, &def, &refLinks[0]);
//	fo.open(strTraining, ios::out | ios::app);
//	char label[] = "NY";
//	for (auto id : ids)
//	{
//		auto sc = &ens[id];
//		for (auto s : sc->Similarity)
//		if (s != -1)
//			fo << s << ",";
//		else
//			fo << "?,";
//		fo << sc->Weight << "," << label[refLinks->find(((INT64)sc->Target << 32) | sc->Source) != refLinks->end()] << endl;
//	}
//	fo.close();
//	//Test data
//	bBlock.ResetPointer(TESTSET); dSrc.ResetPointer(); dDes.ResetPointer();
//	Match(&dSrc, &dDes, &bBlock, &def, &refLinks[1], params, strTest);
//	DataSource::ClearTokenID();
//}


//#include "ScSLINT.h"

//VOID Final(DataSource *dSrc, DataSource *dDes, MAP<INT32, SPOEntry> *enSrc, MAP<INT32, SPOEntry> *enDes, VECTOR<ScoreEntry>* score, Configuration *config, RegressionModel *model, INT32 nBegin, INT32 nEnd);
//
//VOID Regress0(SystemParameters params, STRING modelFile, STRING resultFile)
//{
//	DataSource dSrc(params.SourceRepositoryPath);
//	DataSource dDes(params.TargetRepositoryPath);
//	DataSource::LoadTokenID(params.SourceRepositoryPath, params.TargetRepositoryPath);
//	RepositorySpliter rep(params.SplitFile);
//	VECTOR<Alignment> alg;
//	VECTOR<VECTOR<INT32>> sharedTokens;
//	LoadStringAlignment(params.AlignmentFile, alg, sharedTokens);
//	alg = LoadAlignment(params.AlignmentFile);
//	auto simFunctions = GenerateSimilarityFunction(&alg);
//	MAP<INT32, INT32> pSrc, pDes;
//	for (auto a : alg)
//	{
//		pSrc[a.Source] = a.Type;
//		pDes[a.Target] = a.Type;
//	}
//	dSrc.LoadPOS(params.Ktok); dDes.LoadPOS(params.Ktok);
//	dSrc.LoadSPO(pSrc);	dDes.LoadSPO(pDes);
//	dSrc.LoadIDF(); dDes.LoadIDF();
//	auto allLinks = LoadReference(params.LabelFile);
//	SET<INT64> refLinks;
//	SET<INT32> srcIns;
//	INT64 t;
//	srcIns = rep.GetInstanceList(TESTSET);
//	refLinks = AdaptToSplit(&allLinks, &srcIns);
//	auto model = LoadModel(modelFile);
//	//AdaptBlockingModel(&model, &alg, &sharedTokens);
//	//t = TickCount();
//	//Block(&dSrc, &dDes, &alg, &sharedTokens, &srcIns, params, params.BlockFile);
//	//t = TickCount() - t;
//	//LOGALL(params.BlockFile << "," << t << endl);
//	dSrc.ResetPointer(); dDes.ResetPointer();
//
//	BlockReader bBlock(params.BlockFile);
//	Configuration config(simFunctions);
//	t = TickCount();
//	MatchAndOutputDetail(&dSrc, &dDes, &bBlock, &config, params, resultFile);
//	t = TickCount() - t;
//	LOGALL(resultFile << "," << t << endl);
//
//	/*AdaptConfiguration(&model, &config);
//	t = TickCount();
//	Match(&dSrc, &dDes, &bBlock, &config, &model, params, resultFile);
//	t = TickCount() - t;
//	LOGALL(resultFile << "," << t << endl);
//	auto res = StableFiltering(resultFile, params.FilteringFactor, 0.0F);
//	auto rpf = Evaluate(&res, &refLinks);
//	LOGALL(resultFile << "," << rpf.Detected << "," << rpf.Expected << "," << rpf.Correct << endl);**/
//
//	//Regress1(params, modelFile, resultFile);
//	//Regress2(params, modelFile, resultFile);
//
//	DataSource::ClearTokenID();
//}
//
//VOID Regress1(SystemParameters params, STRING modelFile, STRING resultFile)
//{
//	RepositorySpliter rep(params.SplitFile);
//	auto srcIns = rep.GetInstanceList(TESTSET);
//	auto allLinks = LoadReference(params.LabelFile);
//	auto refLinks = AdaptToSplit(&allLinks, &srcIns);
//	auto model = LoadModel(modelFile);
//	ifstream fi(resultFile, ios::binary);
//	INT64 fileLength = FileLength(resultFile);
//	INT64 entryCount = ReadINT64(&fi);
//	INT64 simCount =( (fileLength - sizeof(INT64)) / entryCount - sizeof(INT32)* 2)/sizeof(FLOAT);
//	VECTOR<pair<INT64, FLOAT>> res;
//	ScoreEntryEx ex; ex.Similarity.resize(simCount);
//	Loop(entryCount);
//	for (INT64 i = 0; i < entryCount; i++)
//	{
//		ex.Source = ReadINT32(&fi);
//		ex.Target = ReadINT32(&fi);
//		fi.read((INT8*)&ex.Similarity[0], sizeof(FLOAT)*simCount);
//		FLOAT score = model.weight[0];
//		for (INT32 j = 0; j < simCount; j++)
//			if (ex.Similarity[j]>0)
//				if (model.sigma[j] != 0)
//					score += (ex.Similarity[j] - model.mu[j]) / model.sigma[j] * model.weight[j + 1];
//				else
//					score += ex.Similarity[j] * model.weight[j + 1];
//		score = 1.0F / (1.0F + exp(-score));
//
//		if (score >= 0.5F)
//			res.push_back(pair<INT64,FLOAT>(((INT64)ex.Target << 32) | ex.Source, score));
//		Loop();
//	}
//	fi.close();
//	
//	auto rpf = Evaluate(&res, &refLinks);
//	LOGALL(resultFile << "," << rpf.Detected << "," << rpf.Expected << "," << rpf.Correct << endl);
//
//	DataSource::ClearTokenID();
//}
//
//VOID Regress3(SystemParameters params, STRING modelFile, STRING resultFile)
//{
//	RepositorySpliter rep(params.SplitFile);
//	auto srcIns = rep.GetInstanceList(TESTSET);
//	auto allLinks = LoadReference(params.LabelFile);
//	auto refLinks = AdaptToSplit(&allLinks, &srcIns);
//	auto model = LoadModel(modelFile);
//	ifstream fi(resultFile, ios::binary);
//	INT64 fileLength = FileLength(resultFile);
//	INT64 entryCount = ReadINT64(&fi);
//	INT64 simCount = ((fileLength - sizeof(INT64)) / entryCount - sizeof(INT32)* 2) / sizeof(FLOAT);
//	VECTOR<ScoreEntry> ens; ens.reserve(entryCount);
//	VECTOR<FLOAT> similarity(simCount);
//	for (INT64 i = 0; i < entryCount; i++)
//	{
//		ScoreEntry en;
//		en.Source = ReadINT32(&fi);
//		en.Target = ReadINT32(&fi);
//		fi.read((INT8*)&similarity[0], sizeof(FLOAT)*simCount);
//		en.Score = model.weight[0];
//		for (INT32 j = 0; j < simCount; j++)
//		if (similarity[j] > 0 && model.weight[j + 1] > 0)
//		if (model.sigma[j] != 0)
//			en.Score += (similarity[j] - model.mu[j]) / model.sigma[j] * model.weight[j + 1];
//		else
//			en.Score += similarity[j] * model.weight[j + 1];
//		en.Score = 1.0F / (1.0F + exp(-en.Score));
//		ens.push_back(en);
//	}
//	fi.close();
//
//	auto res = StableFiltering(&ens, params.FilteringFactor, 0.5F);
//	auto rpf = Evaluate(&res, &refLinks);
//	LOGALL(resultFile << "," << rpf.Detected << "," << rpf.Expected << "," << rpf.Correct << endl);
//
//	DataSource::ClearTokenID();
//}
//
//VOID Regress(SystemParameters params, STRING modelFile, STRING resultFile)
//{
//	RepositorySpliter rep(params.SplitFile);
//	auto srcIns = rep.GetInstanceList(TESTSET);
//	auto allLinks = LoadReference(params.LabelFile);
//	auto refLinks = AdaptToSplit(&allLinks, &srcIns);
//	auto model = LoadModel(modelFile);
//	ifstream fi(resultFile, ios::binary);
//	INT64 fileLength = FileLength(resultFile);
//	INT64 entryCount = ReadINT64(&fi);
//	INT64 simCount = ((fileLength - sizeof(INT64)) / entryCount - sizeof(INT32)* 2) / sizeof(FLOAT);
//	VECTOR<ScoreEntry> ens; ens.reserve(entryCount);
//	VECTOR<FLOAT> similarity(simCount);
//	for (INT64 i = 0; i < entryCount; i++)
//	{
//		ScoreEntry en = { 0 };
//		en.Source = ReadINT32(&fi);
//		en.Target = ReadINT32(&fi);
//		fi.read((INT8*)&similarity[0], sizeof(FLOAT)*simCount);
//		for (INT32 j = 0; j < simCount - 1; j++)
//		if (similarity[j]>0)
//			en.Score += similarity[j];
//		en.Score *= similarity[simCount - 1];
//		ens.push_back(en);
//	}
//	fi.close();
//
//	auto res = StableFiltering(&ens, params.FilteringFactor, 0.0F);
//	auto rpf = Evaluate(&res, &refLinks);
//	LOGALL(resultFile << "," << rpf.Detected << "," << rpf.Expected << "," << rpf.Correct << endl);
//
//	DataSource::ClearTokenID();
//}
//
//RegressionModel LoadModel(STRING strFile)
//{
//	RegressionModel model;
//	ifstream fi(strFile);
//	INT8 line[4096];
//	VECTOR<STRING> sepLine;
//	VECTOR<FLOAT>* t[] = { &model.mu, &model.sigma, &model.weight };
//	INT32 i = 0;
//	while (fi.getline(line, 4096) && i < 3)
//	{
//		sepLine = ReadSeparatedLine(line, ",");
//		for (auto j : sepLine)
//			t[i]->push_back(atof(j.c_str()));
//		i++;
//	}
//	return model;
//}
//
//VOID AdaptBlockingModel(RegressionModel *model, VECTOR<Alignment> *alg, VECTOR<VECTOR<INT32>> *sharedToken)
//{
//	auto idx = GenerateSimilarityFunctionMap(alg);
//	SET<INT32> sel;
//	for (INT32 i = 1; i < model->weight.size() - 1; i++) //skip weight of intercept and weight(target_instance)
//	if (model->weight[i] > 0)
//		sel.insert(idx[i - 1]);
//	
//	//modify alg & sharedToken
//	INT32 k = 0;
//	for (INT32 i = 0; i < sharedToken->size(); i++)
//	if (Contains(sel, i))
//	{
//		if (i != k)
//		{
//			sharedToken->at(k).assign(sharedToken->at(i).begin(), sharedToken->at(i).end());
//			(*alg)[k] = (*alg)[i];
//
//		}
//		k++;
//	}
//	sharedToken->resize(k);	
//}
//
//VOID AdaptConfiguration(RegressionModel *model, Configuration* config)
//{
//	config->sFOI.clear();
//	for (INT32 i = 1; i < model->weight.size() - 1; i++) //skip weight of intercept and weight(target_instance)
//		if (model->weight[i] > 0)
//			config->sFOI.push_back(i - 1);
//}
//
//VOID Match(DataSource *dSrc, DataSource *dDes, BlockReader *pBlock, Configuration *config, RegressionModel* model, SystemParameters params, STRING strOutputFile)
//{
//	INT64 entryCount = pBlock->GetEntryCount();
//	INT32 functionCount = (INT32)config->sFOI.size();
//	INT64 processedEntry = 0;
//	INT64 totalTime = 0;
//	INT64 nLoop = 0;
//	MAP<INT32, SPOEntry> enSrc(dSrc->SPOEntryCount);
//
//	ofstream fo(strOutputFile, ios::out | ios::binary);
//	fo.seekp(entryCount * sizeof(ScoreEntry)+sizeof(INT64)-1, fo.beg); 
//	fo.write("\0", 1); if (!fo.good()) return; fo.seekp(0, fo.beg); //disk space availability
//	fo.write((INT8*)&entryCount, sizeof(INT64));
//
//	pBlock->SetBufferSize(0x8000);
//	Loop(entryCount);
//	while (processedEntry < entryCount)
//	{
//		auto entry = pBlock->Read();
//
//		INT32 size = (INT32)entry.size();
//		if (size > 0)
//		{
//			VECTOR<ScoreEntry> score(size);
//			MAP<INT32, SPOEntry> enDes;
//			for (INT32 i = 0, j = 0; i < size; i++, j++)
//			{
//				score[j].Source = entry[i].first;
//				score[j].Target = entry[i].second;
//				if (!Contains(enSrc, score[j].Source))//load
//					enSrc[score[j].Source] = dSrc->ReadSPOEntry(score[j].Source);
//				if (!Contains(enDes, score[j].Target)) //load
//					enDes[score[j].Target] = dDes->ReadSPOEntry(score[j].Target);
//			}
//			//Match
//			if (params.ThreadCount == 1)
//				Final(dSrc, dDes, &enSrc, &enDes, &score, config, model, 0, size);
//			else
//			{
//				INT32 seg = size / params.ThreadCount;
//				VECTOR<thread> tMatch; tMatch.reserve(params.ThreadCount);
//				for (INT32 j = 0, k = 0; j < params.ThreadCount; j++, k += seg)
//				{
//					if (j == params.ThreadCount - 1)
//						seg = size - seg * j;
//					tMatch.push_back(thread(Final, dSrc, dDes, &enSrc, &enDes, &score, config, model, k, k + seg));
//				}
//				for (INT32 j = 0; j < params.ThreadCount; j++)
//					tMatch[j].join();
//			}
//
//			fo.write((INT8*)&score[0], sizeof(ScoreEntry)*size);
//			fo.flush();
//			pBlock->SetBufferSize(params.BlockBuffer);
//			processedEntry += size;
//			Loop(processedEntry);
//		}
//	}
//	fo.close();
//}
//
//VOID Final(DataSource *dSrc, DataSource *dDes, MAP<INT32, SPOEntry> *enSrc, MAP<INT32, SPOEntry> *enDes, VECTOR<ScoreEntry>* score, Configuration *config, RegressionModel *model, INT32 nBegin, INT32 nEnd)
//{
//	auto end = score->begin() + nEnd;
//	for (auto entry = score->begin() + nBegin; entry != end; entry++)
//	{
//		SPOEntry *src = &enSrc->operator[](entry->Source);
//		SPOEntry *des = &enDes-> operator[](entry->Target);
//		entry->Score = model->weight[0]; //intercept
//		for (INT32 k : config->sFOI)
//		{
//			auto sim = config->Similarity[k].pFunc(dSrc, dDes, src, des, config->Similarity[k].pSrc, config->Similarity[k].pDes);
//			if (model->sigma[k] > 0)
//				entry->Score += sim * model->mu[k] * model->weight[k + 1] / model->sigma[k];
//		}
//		entry->Score += LOG(des->TripleCount + 1.0F, (FLOAT)dDes->MaxTripleCount) * model->weight.back();
//
//		//Sigmoid function
//		entry->Score = 1.0F / (1.0F + exp(-entry->Score));
//	}
//}
//

//
//VOID GenerateExternalFileHeader(STRING strFile, INT32 nDimension, INT32 format)
//{
//	ofstream fo;
//	fo.open(strFile, ios::out);
//	if (format == ARFF) //ARFF
//	{
//		fo << "@RELATION SER" << endl;
//		for (INT32 i = 0; i < nDimension; i++)
//			fo << "@ATTRIBUTE S" << i << " NUMERIC" << endl;
//		fo << "@ATTRIBUTE class {yes,no}" << endl;
//		fo << "@DATA" << endl;
//		fo.close();
//	}
//	if (format == CSV)
//	{
//		for (INT32 i = 0; i < nDimension; i++)
//			fo << "S" << i << ",";
//		fo << "class" << endl;
//	}
//
//	fo.close();	
//}
//
//VECTOR<INT32> SelectBalanceDataset(VECTOR<ScoreEntryEx> *ens, Configuration *def, SET<INT64> *refLinks)
//{
//	//VECTOR<INT32> idx(ens->size());
//	//for (INT32 i = 0; i < ens->size(); i++)
//	//	idx[i] = i; 
//	//return idx;
//	MAP<INT32, pair<INT32, FLOAT>> src;
//
//	VECTOR<INT32> idx;
//	src.reserve(ens->size());
//	idx.reserve(refLinks->size() * 2);
//
//	INT32 i = 0;
//	for (auto en = ens->begin(); en != ens->end(); en++, i++)
//	{
//		INT64 entry = ((INT64)en->Target << 32) | en->Source;
//		if (refLinks->find(entry) != refLinks->end()) //positive
//			idx.push_back(i);
//		else
//		{
//			//negative --> find max score
//			FLOAT score = def->Aggregate.pFunc(&*en, &def->sFOI, &def->SimilarityThreshold,
//				def->Aggregate.useBoolean, def->Aggregate.useAverage, def->Aggregate.useWeighting);
//			if (score > 0)
//			{
//				INT32 exist = src.find(en->Source) != src.end();
//				if (!exist || (exist && score > src[en->Source].second))
//					src[en->Source] = pair<INT32, FLOAT>(i, score);
//			}
//		}
//	}
//
//	for (auto entry = src.begin(); entry != src.end(); entry++)
//		idx.push_back(entry->second.first);
//	return idx;
//}
//
//VOID PrintExample(INT8* line, ScoreEntryEx &x, INT8* label)
//{	
//	char *p = line + sprintf(line, "%d,%d,", x.Source, x.Target);
//	for (auto s : x.Similarity)
//		p += sprintf(p, "%f,", s);
//	sprintf(p, "%s", label);
//}
//
//FLOAT Distance(ScoreEntryEx &x, ScoreEntryEx &y)
//{
//	FLOAT s = 0;
//	FLOAT t = 0;
//	for (INT32 i = 0; i < x.Similarity.size() - 1; i++) //-1: weight
//	{
//		t = x.Similarity[i] - y.Similarity[i];
//		s += t*t;
//	}
//	s = sqrt(s);
//	return s;
//}
//
//INT32 MinIndex(VECTOR<ScoreEntryEx> &v)
//{
//	if (v.size() == 0)
//		return -1;
//	int min = 0;
//	for (INT32 i = 1; i < v.size(); i++)
//	if (v[i].Weight < v[min].Weight)
//		min = i;
//	return min;
//}
//
//INT32 ScoreEntryExCompare(ScoreEntryEx x, ScoreEntryEx y)
//{
//	return x.Weight > y.Weight;
//}
//
//VOID SelectTrainingExample(STRING strInputFile, STRING strOutputFile, INT32 nNegativeSize)
//{
//	//strInputFile: DetailedScore file generated by MatchAndOutputDetail (Matching.cpp)
//	//strInputFile: sorted by Target
//	ifstream fi(strInputFile);	
//	MAP<INT32, VECTOR<ScoreEntryEx>> pos;
//	MAP<INT32, VECTOR<ScoreEntryEx>> neg;
//
//	//postives
//	INT8 line[4096];
//	VECTOR<STRING> sepLine;
//	ScoreEntryEx ex;
//	INT32 simCount = 0;
//	
//	//header
//	while (fi.getline(line, 4096) &&  line[0] == '@')
//		simCount++;
//	simCount -= 5;//- source , target
//	ex.Similarity.resize(simCount); 
//
//	do
//	{
//		//Parse
//		sepLine = ReadSeparatedLine(line, ",");		
//		if (sepLine.back().compare("yes")==0) //positive
//		{
//			ex.Source = atoi(sepLine[0].c_str());
//			ex.Target = atoi(sepLine[1].c_str());		
//			for (INT32 i = 0; i < simCount; i++)
//				ex.Similarity[i] = atof(sepLine[i + 2].c_str());
//			pos[ex.Source].push_back(ex);
//		}		
//	}
//	while (fi.getline(line, 4096));
//
//	//negatives
//	fi.clear();
//	fi.seekg(0, ios::beg);
//	while (fi.getline(line, 4096) && line[0] == '@');
//
//	do
//	{
//		//Parse
//		sepLine = ReadSeparatedLine(line, ",");
//		if (sepLine.back().compare("no") == 0) //negative
//		{
//			ex.Source = atoi(sepLine[0].c_str());
//			ex.Target = atoi(sepLine[1].c_str());
//			for (INT32 i = 0; i < simCount; i++)
//				ex.Similarity[i] = atof(sepLine[i + 2].c_str());
//
//			if (!Contains(pos, ex.Source)) //Summation
//			{
//				ex.Weight = 0;
//				for (auto s : ex.Similarity)
//					ex.Weight += s;
//				ex.Weight = -ex.Weight;
//			}
//			else //MIN complete link to postive, 
//			{
//				ex.Weight = 0;
//				for (auto x : pos[ex.Source])
//				{
//					auto s = Distance(x, ex);
//					ex.Weight = MIN(ex.Weight, s);
//				}
//			}
//
//			if (!Contains(neg, ex.Source))
//			{
//				neg[ex.Source].reserve(100);
//				neg[ex.Source].push_back(ex);
//			}
//			else
//			{
//				auto &q = neg[ex.Source];
//				if (q.size() < nNegativeSize)
//					q.push_back(ex);
//				else
//				{
//					auto min = MinIndex(q);
//					if (q[min].Weight > ex.Weight)
//						q[min] = ex;
//				}
//			}
//		}
//	}
//	while (fi.getline(line, 4096));
//	fi.close();
//	
//	VECTOR<VECTOR<ScoreEntryEx>> sortedNeg;
//	sortedNeg.reserve(neg.size());
//	INT32 longest = 0;
//	for (auto e : neg)
//	{
//		sort(e.second.begin(), e.second.end(), ScoreEntryExCompare);
//		sortedNeg.push_back(e.second);		
//		longest = MAX(longest, e.second.size());
//	}
//
//	//Output	
//	for (INT32 nNegative = 5; nNegative < nNegativeSize; nNegative += 10)
//	{
//		if (nNegative > longest)
//			break;
//
//		sprintf(line, "%s_%d.arff", strOutputFile.c_str(), nNegative);
//		STRING strOutput(line);
//
//		GenerateExternalFileHeader(strOutput, simCount + 2, ARFF); //+2: source, target 
//		ofstream fo(strOutput, ios::app);
//		for (auto e : pos)
//		for (auto x : e.second)
//		{
//			memset(line, 0, 4096);
//			PrintExample(line, x, "yes");
//			fo << line << endl;
//		}
//
//		for (auto e : sortedNeg)
//		{
//			INT32 write = MIN(nNegative, e.size());
//			for (INT32 i = 0; i < write; i++)
//			{
//				memset(line, 0, 4096);
//				PrintExample(line, e[i], "no");
//				fo << line << endl;
//			}
//		}
//		fo.close();
//	}
//}
//
//
//VOID MatchWithDefaultConfiguration(SystemParameters params, STRING strOutDir)
//{
//	DataSource dSrc(params.SourceRepositoryPath);
//	DataSource dDes(params.TargetRepositoryPath);
//	DataSource::LoadTokenID(params.SourceRepositoryPath, params.TargetRepositoryPath);
//	VECTOR<Alignment> alg;
//	VECTOR<VECTOR<INT32>> sharedTokens;
//	INT64 t1 = TickCount();
//	Align(&dSrc, &dDes, params, &alg, &sharedTokens);
//	t1 = TickCount() - t1;
//	INT64 t2 = TickCount();
//	Block(&dSrc, &dDes, &alg, &sharedTokens, params, strOutDir + SLASH + params.Jobname + ".block");
//	t2 = TickCount() - t2;
//	MAP<INT32, INT32> pSrc, pDes;
//	for (auto a : alg)
//	{
//		pSrc[a.Source] = a.Type;
//		pDes[a.Target] = a.Type;
//	}
//	dSrc.LoadSPO(pSrc);	dDes.LoadSPO(pDes);
//	dSrc.LoadIDF(); dDes.LoadIDF();
//	BlockReader bBlock(strOutDir + SLASH + params.Jobname + ".block");
//	dSrc.ResetPointer(); dDes.ResetPointer();
//	auto simFunctions = GenerateSimilarityFunction(alg);
//	Configuration def(simFunctions);
//	INT64 t3 = TickCount();
//	Match(&dSrc, &dDes, &bBlock, &def, params, strOutDir + SLASH + params.Jobname + ".score");
//	t3 = TickCount() - t3;
//	INT64 t4 = TickCount();
//	StableFiltering(strOutDir + SLASH + params.Jobname + ".score", params.FilteringFactor, 0);
//	t4 = TickCount() - t4;
//	INT8 str[1024];
//	sprintf(str, "%s,%d,%d,%d,%d,%d,%d,%d,%d", params.Jobname.c_str(), dSrc.SPOEntryCount, dDes.SPOEntryCount, bBlock.GetEntryCount(), simFunctions.size(), t1, t2, t3, t4);
//	LOGALL(str << endl);
//
//	DataSource::ClearTokenID();
//}

//VOID DetailAndSum(DataSource *dSrc, DataSource *dDes, MAP<INT32, SPOEntry> *enSrc, MAP<INT32, SPOEntry> *enDes, VECTOR<ScoreEntryEx>* score, Configuration *config, INT32 nBegin, INT32 nEnd)
//{
//	auto end = score->begin() + nEnd;
//	INT32 size = config->sFOI.size();
//	for (auto entry = score->begin() + nBegin; entry != end; entry++)
//	{
//		SPOEntry *src = &enSrc->operator[](entry->Source);
//		SPOEntry *des = &enDes-> operator[](entry->Target);
//		for (INT32 k : config->sFOI)
//		{
//			entry->Similarity[k] = config->Similarity[k].pFunc(dSrc, dDes, src, des, config->Similarity[k].pSrc, config->Similarity[k].pDes);
//			if (entry->Similarity[k] != -1)
//				entry->Similarity[size] += entry->Similarity[k];
//		}
//		entry->Weight = LOG(des->TripleCount + 1.0F, (FLOAT)dDes->MaxTripleCount);
//	}
//}
//
//
//VOID MatchAndOutputDetail(DataSource *dSrc, DataSource *dDes, BlockReader *pBlock, Configuration *config, SystemParameters params, STRING strOutputFile)
//{
//	INT64 entryCount = pBlock->GetEntryCount();
//	INT32 functionCount = (INT32)config->sFOI.size();
//	INT64 processedEntry = 0;
//	INT64 totalTime = 0;
//	INT64 nLoop = 0;
//	MAP<INT32, SPOEntry> enSrc(dSrc->SPOEntryCount);
//
//	ofstream fo(strOutputFile, ios::out | ios::binary);
//	fo.seekp(entryCount * (sizeof(INT32)* 2 + (functionCount + 1) * sizeof(FLOAT)) + sizeof(INT64)-1, fo.beg); fo.write("\0", 1); if (!fo.good()) return; fo.seekp(0, fo.beg); //disk space availability
//	fo.write((INT8*)&entryCount, sizeof(INT64));
//
//	pBlock->SetBufferSize(0x8000);
//	Loop(entryCount);
//	while (processedEntry < entryCount)
//	{
//		auto entry = pBlock->Read();
//
//		INT32 size = (INT32)entry.size();
//		if (size > 0)
//		{
//			VECTOR<ScoreEntryEx> score(size);
//			MAP<INT32, SPOEntry> enDes;
//			for (INT32 i = 0, j = 0; i < size; i++, j++)
//			{
//				score[j].Source = entry[i].first;
//				score[j].Target = entry[i].second;
//				if (!Contains(enSrc, score[j].Source)) //load
//					enSrc[score[j].Source] = dSrc->ReadSPOEntry(score[j].Source);
//				if (!Contains(enDes, score[j].Target)) //load
//					enDes[score[j].Target] = dDes->ReadSPOEntry(score[j].Target);
//				score[i].Similarity.resize(functionCount);
//			}
//			//Match
//			if (params.ThreadCount == 1)
//				Detail(dSrc, dDes, &enSrc, &enDes, &score, config, 0, size);
//			else
//			{
//				INT32 seg = size / params.ThreadCount;
//				VECTOR<thread> tMatch; tMatch.reserve(params.ThreadCount);
//				for (INT32 j = 0, k = 0; j < params.ThreadCount; j++, k += seg)
//				{
//					if (j == params.ThreadCount - 1)
//						seg = size - seg * j;
//					tMatch.push_back(thread(Detail, dSrc, dDes, &enSrc, &enDes, &score, config, k, k + seg));
//				}
//				for (INT32 j = 0; j < params.ThreadCount; j++)
//					tMatch[j].join();
//			}
//
//			for (auto sc = score.begin(); sc != score.end(); sc++)
//			{
//				fo.write((INT8*)&sc->Source, sizeof(INT32));
//				fo.write((INT8*)&sc->Target, sizeof(INT32));
//				fo.write((INT8*)&sc->Similarity[0], sc->Similarity.size() * sizeof(FLOAT));
//				fo.write((INT8*)&sc->Weight, sizeof(FLOAT));
//			}
//			fo.flush();
//
//			pBlock->SetBufferSize(params.BlockBuffer);
//			processedEntry += size;
//			Loop(processedEntry);
//		}
//	}
//	fo.close();
//}
//
////Output to textfile
//VOID MatchAndOutputDetail(DataSource *dSrc, DataSource *dDes, BlockReader *pBlock, Configuration *config, SET<INT64> *refLinks, SystemParameters params, INT8* label[3], STRING strOutput)
//{
//	INT64 entryCount = pBlock->GetEntryCount();
//	INT32 functionCount = (INT32)config->sFOI.size();
//	INT64 processedEntry = 0;
//	INT64 totalTime = 0;
//	INT64 nLoop = 0;
//	MAP<INT32, SPOEntry> enSrc(dSrc->SPOEntryCount);
//
//	ofstream fo(strOutput, ios::out | ios::app);
//
//	pBlock->SetBufferSize(0x8000);
//	Loop(entryCount);
//	while (processedEntry < entryCount)
//	{
//		auto entry = pBlock->Read();
//		INT32 size = (INT32)entry.size();
//		if (size > 0)
//		{
//			VECTOR<ScoreEntryEx> score(size);
//			MAP<INT32, SPOEntry> enDes;
//			for (INT32 i = 0, j = 0; i < size; i++, j++)
//			{
//				score[j].Source = entry[i].first;
//				score[j].Target = entry[i].second;
//				if (!Contains(enSrc, score[j].Source)) //load
//					enSrc[score[j].Source] = dSrc->ReadSPOEntry(score[j].Source);
//				if (!Contains(enDes, score[j].Target)) //load
//					enDes[score[j].Target] = dDes->ReadSPOEntry(score[j].Target);
//				score[j].Similarity.resize(functionCount);
//			}
//
//			//Match
//			if (params.ThreadCount == 1)
//				Detail(dSrc, dDes, &enSrc, &enDes, &score, config, 0, size);
//			else
//			{
//				INT32 seg = size / params.ThreadCount;
//				VECTOR<thread> tMatch; tMatch.reserve(params.ThreadCount);
//				for (INT32 j = 0, k = 0; j < params.ThreadCount; j++, k += seg)
//				{
//					if (j == params.ThreadCount - 1)
//						seg = size - seg * j;
//					tMatch.push_back(thread(Detail, dSrc, dDes, &enSrc, &enDes, &score, config, k, k + seg));
//				}
//				for (INT32 j = 0; j < params.ThreadCount; j++)
//					tMatch[j].join();
//			}
//
//			for (auto sc = score.begin(); sc != score.end(); sc++)
//			{
//				fo << sc->Source << "," << sc->Target << ",";
//				for (auto s : sc->Similarity)
//				if (s >= 0)
//					fo << s << ",";
//				else
//					fo << label[2] << ",";
//				fo << sc->Weight << "," << label[RefContains(refLinks, ((INT64)sc->Target << 32) | sc->Source)] << endl;
//			}
//
//			fo.flush();
//			pBlock->SetBufferSize(params.BlockBuffer);
//			processedEntry += size;
//			Loop(processedEntry);
//		}
//	}
//	fo.close();
//}
//
////Output to textfile
//VOID MatchAndBalance(DataSource *dSrc, DataSource *dDes, BlockReader *pBlock, Configuration *config, SET<INT64> *refLinks, SystemParameters params, INT8* label[3], STRING strOutput)
//{
//	INT64 entryCount = pBlock->GetEntryCount();
//	INT32 functionCount = (INT32)config->sFOI.size();
//	INT64 processedEntry = 0;
//	INT64 totalTime = 0;
//	INT64 nLoop = 0;
//	MAP<INT32, SPOEntry> enSrc(dSrc->SPOEntryCount);
//	MAP<INT32, VECTOR<ScoreEntryEx>> scPositive;
//	MAP<INT32, ScoreEntryEx> scNegative;
//
//	pBlock->SetBufferSize(0x8000);
//	Loop(entryCount);
//	while (processedEntry < entryCount)
//	{
//		auto entry = pBlock->Read();
//
//		INT32 size = (INT32)entry.size();
//		if (size > 0)
//		{
//			VECTOR<ScoreEntryEx> score(size);
//			MAP<INT32, SPOEntry> enDes;
//			for (INT32 i = 0, j = 0; i < size; i++, j++)
//			{
//				score[j].Source = entry[i].first;
//				score[j].Target = entry[i].second;
//				INT32 entryLabel = RefContains(refLinks, ((INT64)score[j].Target << 32) | score[j].Source);
//				if (!Contains(enSrc, score[j].Source)) //load
//					enSrc[score[j].Source] = dSrc->ReadSPOEntry(score[j].Source);
//				if (!Contains(enDes, score[j].Target)) //load
//					enDes[score[j].Target] = dDes->ReadSPOEntry(score[j].Target);
//				score[j].Similarity.resize(functionCount + 2); //IMPORTANT: last= label, second last= sum
//				score[j].Similarity[functionCount + 1] = entryLabel; //label
//			}
//
//
//			//Match
//			if (params.ThreadCount == 1)
//				DetailAndSum(dSrc, dDes, &enSrc, &enDes, &score, config, 0, size);
//			else
//			{
//				INT32 seg = size / params.ThreadCount;
//				VECTOR<thread> tMatch; tMatch.reserve(params.ThreadCount);
//				for (INT32 j = 0, k = 0; j < params.ThreadCount; j++, k += seg)
//				{
//					if (j == params.ThreadCount - 1)
//						seg = size - seg * j;
//					tMatch.push_back(thread(DetailAndSum, dSrc, dDes, &enSrc, &enDes, &score, config, k, k + seg));
//				}
//				for (INT32 j = 0; j < params.ThreadCount; j++)
//					tMatch[j].join();
//			}
//
//			//Output to scBalance
//			for (auto sc : score)
//			if (sc.Similarity[functionCount + 1] == 1)
//				scPositive[sc.Source].push_back(sc);
//			else
//			{
//				auto sr = scNegative.find(sc.Source);
//				if (sr == scNegative.end() ||
//					sc.Similarity[functionCount] > sr->second.Similarity[functionCount])
//					scNegative[sc.Source] = sc;
//			}
//			pBlock->SetBufferSize(params.BlockBuffer);
//			processedEntry += size;
//			Loop(processedEntry);
//		}
//	}
//
//	//Output
//	ofstream fo(strOutput, ios::out | ios::app);
//	for (auto entry : scPositive)
//	for (auto sc : entry.second)
//	{
//		for (INT32 i = 0; i < functionCount; i++)
//		if (sc.Similarity[i] != -1)
//			fo << sc.Similarity[i] << ",";
//		else
//			fo << label[2] << ",";
//		fo << sc.Weight << "," << label[1] << endl;
//	}
//	for (auto entry : scNegative)
//	{
//		auto &sc = entry.second;
//		if (sc.Similarity[functionCount] > 0)
//		{
//			for (INT32 i = 0; i < functionCount; i++)
//			if (sc.Similarity[i] != -1)
//				fo << sc.Similarity[i] << ",";
//			else
//				fo << label[2] << ",";
//			fo << sc.Weight << "," << label[0] << endl;
//		}
//	}
//	fo.close();
//}
//
