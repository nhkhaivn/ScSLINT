#include "Resolution.h"
#include "Utility.h"
#include <thread>
#include <cmath>

VOID Detail_(DataSource *dSrc, DataSource *dDes, MAP<INT32, SPOEntry> *enSrc, MAP<INT32, SPOEntry> *enDes, VECTOR<ScoreEntryEx>* score, Configuration *config, INT32 nBegin, INT32 nEnd);
VOID Final_(DataSource *dSrc, DataSource *dDes, MAP<INT32, SPOEntry> *enSrc, MAP<INT32, SPOEntry> *enDes, VECTOR<ScoreEntry>* score, Configuration *config, INT32 nBegin, INT32 nEnd);
VOID MatchThread(DataSource *dSrc, DataSource *dDes, Configuration *config, SystemParameters params, STRING strOutputFile, BlockReader* pBlock, INT32 nID);

VOID FastMatch(DataSource *dSrc, DataSource *dDes, Configuration *config, SystemParameters params, STRING strOutputFile)
{
	BlockReader **pBlock = new BlockReader*[params.ThreadCount];
	for (INT32 i = 0; i < params.ThreadCount; i++)
	{
		pBlock[i] = new BlockReader(params.BlockFile);
		INT64 base = pBlock[i]->GetEntryCount() / params.ThreadCount;
		INT64 size;
		if (i != params.ThreadCount - 1)
			size = base;
		else
			size = pBlock[i]->GetEntryCount() - base * i;
		pBlock[i]->Select(i * base, size);
	}

	VECTOR<thread> tMatch; tMatch.reserve(params.ThreadCount);
	for (INT32 j = 0; j < params.ThreadCount; j++)
		tMatch.push_back(thread(MatchThread, dSrc, dDes, config, params, strOutputFile, pBlock[j], j));

	for (INT32 j = 0; j < params.ThreadCount; j++)
		tMatch[j].join();

	for (INT32 i = 0; i < params.ThreadCount; i++)
		delete pBlock[i];
	delete[] pBlock;

	//combine
}

VOID MatchThread(DataSource *dSrc, DataSource *dDes, Configuration *config, SystemParameters params, STRING strOutputFile, BlockReader* pBlock, INT32 nID)
{
	INT8 outBuf[1024];
	sprintf(outBuf, "%d", nID);
	LOGSTRSAFE(cout, pBlock->GetEntryCount()<< endl);

	INT64 entryCount = pBlock->GetEntryCount();
	INT32 functionCount = (INT32)config->sFOI.size();
	INT64 processedEntry = 0;
	INT64 totalTime = 0;
	INT64 nLoop = 0;
	MAP<INT32, SPOEntry> enSrc(dSrc->SPOEntryCount);

	ofstream fo(strOutputFile + outBuf, ios::binary | ios::out);
	if (params.Detail == 0)
	{
		fo.seekp(entryCount * sizeof(ScoreEntry)+sizeof(INT64)-1, fo.beg); fo.write("\0", 1); if (!fo.good()) return; fo.seekp(0, fo.beg); //disk space availability
	}
	else
	{
		fo.seekp(entryCount * (sizeof(ScoreEntry)+sizeof(FLOAT)*functionCount) + sizeof(INT64)-1, fo.beg); fo.write("\0", 1); if (!fo.good()) return; fo.seekp(0, fo.beg); //disk space availability
	}
	fo.write((INT8*)&entryCount, sizeof(INT64));
	pBlock->SetBufferSize(0x8000);

	while (processedEntry < entryCount)
	{
		auto entry = pBlock->Read();
		INT32 size = (INT32)entry.size();
		MAP<INT32, SPOEntry> enDes;
		for (INT32 i = 0; i < size; i++)
		{
			if (!Contains(enSrc, entry[i].first)) //load
				enSrc[entry[i].first] = dSrc->ReadSPOEntry(entry[i].first);
			if (!Contains(enDes, entry[i].second)) //load
				enDes[entry[i].second] = dDes->ReadSPOEntry(entry[i].second);
		}

		if (size > 0)
		{
			if (params.Detail == 0)
			{
				VECTOR<ScoreEntry> score(size);				
				for (INT32 i = 0; i < size; i++)
				{
					score[i].Source = entry[i].first;
					score[i].Target = entry[i].second;
				}
				//Match	
				Final_(dSrc, dDes, &enSrc, &enDes, &score, config, 0, size);
				fo.write((INT8*)&score[0], sizeof(ScoreEntry)*size);
			}
			else
			{
				VECTOR<ScoreEntryEx> score(size);
				for (INT32 i = 0, j = 0; i < size; i++, j++)
				{
					score[j].Source = entry[i].first;
					score[j].Target = entry[i].second;
				}
				Detail_(dSrc, dDes, &enSrc, &enDes, &score, config, 0, size);
				for (auto en : score)
				{
					fo.write((INT8*)&en.Source, sizeof(INT32));
					fo.write((INT8*)&en.Target, sizeof(INT32));
					fo.write((INT8*)&en.Weight, sizeof(FLOAT));
					Write(&fo, (INT8*)&en.Similarity[0], sizeof(FLOAT), en.Similarity.size());
				}
			}
			fo.flush();

			pBlock->SetBufferSize(params.BlockBuffer);
			processedEntry += size;
		}
	}
	fo.close();
}


VOID Detail_(DataSource *dSrc, DataSource *dDes, MAP<INT32, SPOEntry> *enSrc, MAP<INT32, SPOEntry> *enDes, VECTOR<ScoreEntryEx>* score, Configuration *config, INT32 nBegin, INT32 nEnd)
{
	auto end = score->begin() + nEnd;
	for (auto entry = score->begin() + nBegin; entry != end; entry++)
	{
		SPOEntry *src = &enSrc->operator[](entry->Source);
		SPOEntry *des = &enDes-> operator[](entry->Target);
		for (INT32 k : config->sFOI)
			entry->Similarity[k] = config->Similarity[k].Calculate(dSrc, dDes, src, des);
		entry->Weight = LOG(des->TripleCount + 1.0F, (FLOAT)dDes->MaxTripleCount);
	}
}

VOID Final_(DataSource *dSrc, DataSource *dDes, MAP<INT32, SPOEntry> *enSrc, MAP<INT32, SPOEntry> *enDes, VECTOR<ScoreEntry>* score, Configuration *config, INT32 nBegin, INT32 nEnd)
{
	ScoreEntryEx ex; ex.Similarity.resize(config->Similarity.size());	
	auto end = score->begin() + nEnd;	
	for (auto entry = score->begin() + nBegin; entry != end; entry++)
	{
		SPOEntry *src = &enSrc->operator[](entry->Source);
		SPOEntry *des = &enDes-> operator[](entry->Target);		
		for (INT32 k : config->sFOI)
			ex.Similarity[k] = config->Similarity[k].Calculate(dSrc, dDes, src, des);
		ex.Weight = LOG(des->TripleCount + 1.0F, (FLOAT)dDes->MaxTripleCount);
		entry->Score = config->Aggregate.Aggregate(&ex, &config->sFOI, &config->SimilarityThreshold);
	}
}
