#include "Resolution.h"
#include "Utility.h"
#include <thread>
#include <cmath>

VOID Detail(DataSource *dSrc, DataSource *dDes, MAP<INT32, SPOEntry> *enSrc, MAP<INT32, SPOEntry> *enDes, VECTOR<ScoreEntryEx>* score, Configuration *config, INT32 nBegin, INT32 nEnd);
VOID Final(DataSource *dSrc, DataSource *dDes, MAP<INT32, SPOEntry> *enSrc, MAP<INT32, SPOEntry> *enDes, VECTOR<ScoreEntry>* score, Configuration *config, INT32 nBegin, INT32 nEnd);

VECTOR<ScoreEntryEx> Match(DataSource *dSrc, DataSource *dDes, Configuration *config, SystemParameters params)
{
	BlockReader pBlock(params.BlockFile);
	INT64 entryCount = (INT32)pBlock.GetEntryCount();
	INT32 functionCount = (INT32)config->sFOI.size();
	INT64 processedEntry = 0;
	INT64 totalTime = 0;
	INT64 nLoop = 0;

	MAP<INT32, SPOEntry> enSrc;
	enSrc.reserve(dSrc->SPOEntryCount);

	VECTOR<ScoreEntryEx> score((INT32)entryCount);
	for (INT32 i = 0; i < entryCount; i++)
		score[i].Similarity.resize(functionCount);
	pBlock.SetBufferSize(0x8000);

	Loop(entryCount);
	while (processedEntry < entryCount)
	{
		auto entry = pBlock.Read();
		INT32 size = (INT32)entry.size();

		if (size > 0)
		{
			MAP<INT32, SPOEntry> enDes;
			enDes.reserve(size);
			for (INT32 i = 0, j = processedEntry; i < size; i++, j++)
			{
				score[j].Source = entry[i].first;
				score[j].Target = entry[i].second;
				if (!Contains(enSrc, score[j].Source)) //load
					enSrc[score[j].Source] = dSrc->ReadSPOEntry(score[j].Source);
				if (!Contains(enDes, score[j].Target)) //load
					enDes[score[j].Target] = dDes->ReadSPOEntry(score[j].Target);
			}

			//Match
			if (params.ThreadCount == 1)
			{
				Detail(dSrc, dDes, &enSrc, &enDes, &score, config, processedEntry, processedEntry + size);
			}
			else
			{
				INT32 seg = size / params.ThreadCount;
				VECTOR<thread> tMatch; tMatch.reserve(params.ThreadCount);
				for (INT32 j = 0, k = processedEntry; j < params.ThreadCount; j++, k += seg)
				{
					if (j == params.ThreadCount - 1)
						seg = size - seg * j;
					tMatch.push_back(thread(Detail, dSrc, dDes, &enSrc, &enDes, &score, config, k, k + seg));
				}
				for (INT32 j = 0; j < params.ThreadCount; j++)
					tMatch[j].join();
			}
			pBlock.SetBufferSize(params.BlockBuffer);
			processedEntry += size;
			Loop(processedEntry);
		}
	}
	return score;
}

VOID Match(DataSource *dSrc, DataSource *dDes, Configuration *config, SystemParameters params, STRING strOutputFile)
{
	BlockReader pBlock(params.BlockFile);
	INT64 entryCount = pBlock.GetEntryCount();
	INT32 functionCount = (INT32)config->sFOI.size();
	INT64 processedEntry = 0;
	INT64 totalTime = 0;
	INT64 nLoop = 0;
	MAP<INT32, SPOEntry> enSrc(dSrc->SPOEntryCount);

	ofstream fo(strOutputFile, ios::out | ios::binary);
	if (params.Detail == 0)
	{
		fo.seekp(entryCount * sizeof(ScoreEntry)+sizeof(INT64)-1, fo.beg); fo.write("\0", 1); if (!fo.good()) return; fo.seekp(0, fo.beg); //disk space availability
	}
	else
	{
		fo.seekp(entryCount * (sizeof(ScoreEntry)+sizeof(FLOAT)*functionCount) + sizeof(INT64)-1, fo.beg); fo.write("\0", 1); if (!fo.good()) return; fo.seekp(0, fo.beg); //disk space availability
	}
	
	fo.write((INT8*)&entryCount, sizeof(INT64));

	pBlock.SetBufferSize(0x8000);
	Loop(entryCount);
	while (processedEntry < entryCount)
	{
		auto entry = pBlock.Read();
		INT32 size = (INT32)entry.size();
		if (size > 0)
		{
			if (params.Detail == 0)
			{
				VECTOR<ScoreEntry> score(size);
				MAP<INT32, SPOEntry> enDes;
				for (INT32 i = 0, j = 0; i < size; i++, j++)
				{
					score[j].Source = entry[i].first;
					score[j].Target = entry[i].second;
					if (!Contains(enSrc, score[j].Source)) //load
						enSrc[score[j].Source] = dSrc->ReadSPOEntry(score[j].Source);
					if (!Contains(enDes, score[j].Target)) //load
						enDes[score[j].Target] = dDes->ReadSPOEntry(score[j].Target);
				}
				if (params.ThreadCount == 1)
					Final(dSrc, dDes, &enSrc, &enDes, &score, config, 0, size);
				else
				{
					INT32 seg = size / params.ThreadCount;
					VECTOR<thread> tMatch; tMatch.reserve(params.ThreadCount);
					for (INT32 j = 0, k = 0; j < params.ThreadCount; j++, k += seg)
					{
						if (j == params.ThreadCount - 1)
							seg = size - seg * j;
						tMatch.push_back(thread(Final, dSrc, dDes, &enSrc, &enDes, &score, config, k, k + seg));
					}
					for (INT32 j = 0; j < params.ThreadCount; j++)
						tMatch[j].join();
				}

				fo.write((INT8*)&score[0], sizeof(ScoreEntry)*size);
				fo.flush();
			}
			else
			{
				VECTOR<ScoreEntryEx> score(size);
				MAP<INT32, SPOEntry> enDes;
				for (INT32 i = 0, j = 0; i < size; i++, j++)
				{
					score[j].Source = entry[i].first;
					score[j].Target = entry[i].second;
					if (!Contains(enSrc, score[j].Source)) //load
						enSrc[score[j].Source] = dSrc->ReadSPOEntry(score[j].Source);
					if (!Contains(enDes, score[j].Target)) //load
						enDes[score[j].Target] = dDes->ReadSPOEntry(score[j].Target);
					score[j].Similarity.resize(functionCount);
				}

				if (params.ThreadCount == 1)
					Detail(dSrc, dDes, &enSrc, &enDes, &score, config, 0, size);
				else
				{
					INT32 seg = size / params.ThreadCount;
					VECTOR<thread> tMatch; tMatch.reserve(params.ThreadCount);
					for (INT32 j = 0, k = 0; j < params.ThreadCount; j++, k += seg)
					{
						if (j == params.ThreadCount - 1)
							seg = size - seg * j;
						tMatch.push_back(thread(Detail, dSrc, dDes, &enSrc, &enDes, &score, config, k, k + seg));
					}
					for (INT32 j = 0; j < params.ThreadCount; j++)
						tMatch[j].join();
				}

				for (auto en : score)
				{
					fo.write((INT8*)&en.Source, sizeof(INT32));
					fo.write((INT8*)&en.Target, sizeof(INT32));
					fo.write((INT8*)&en.Weight, sizeof(FLOAT));
					Write(&fo, (INT8*)&en.Similarity[0], sizeof(FLOAT), en.Similarity.size());
				}
				fo.flush();
			}

			pBlock.SetBufferSize(params.BlockBuffer);
			processedEntry += size;
			Loop(processedEntry);
		}
	}
	fo.close();
}

VOID Detail(DataSource *dSrc, DataSource *dDes, MAP<INT32, SPOEntry> *enSrc, MAP<INT32, SPOEntry> *enDes, VECTOR<ScoreEntryEx>* score, Configuration *config, INT32 nBegin, INT32 nEnd)
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

VOID Final(DataSource *dSrc, DataSource *dDes, MAP<INT32, SPOEntry> *enSrc, MAP<INT32, SPOEntry> *enDes, VECTOR<ScoreEntry>* score, Configuration *config, INT32 nBegin, INT32 nEnd)
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
		//cout << endl << ex.Weight << endl;

		entry->Score = config->Aggregate.Aggregate(&ex, &config->sFOI, &config->SimilarityThreshold);

	}
}

VECTOR<ScoreEntryEx> LoadScoreEntryEx(STRING strFile)
{
	ifstream fi(strFile, ios::binary);
	INT64 size = ReadINT64(&fi);
	INT32 length = 0;
	VECTOR<ScoreEntryEx> ret(size);
	for (INT32 i = 0; i < size; i++)
	{
		fi.read((INT8*)&ret[i].Source, sizeof(INT32));
		fi.read((INT8*)&ret[i].Target, sizeof(INT32));
		fi.read((INT8*)&ret[i].Weight, sizeof(FLOAT));
		fi.read((INT8*)&length, sizeof(FLOAT));
		ret[i].Similarity.resize(length);
		fi.read((INT8*)&ret[i].Similarity[0], sizeof(FLOAT)* length);
	}
	fi.close();

	return ret;
}

