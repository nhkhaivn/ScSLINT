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
