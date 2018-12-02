#include "Resolution.h"
#include "StreamIO.h"

VECTOR<pair<INT64,FLOAT>> StableFiltering(VECTOR<ScoreEntry> *ens, FLOAT filteringFactor, FLOAT filteringThreshold)
{
	MAP<INT32, FLOAT> maxSrc;
	MAP<INT32, FLOAT> maxDes;
	for (auto entry = ens->begin(); entry != ens->end(); entry++)
	if (entry->Score > 0 && entry->Score >= filteringThreshold)
	{
		
		maxSrc[entry->Source] = MAX(maxSrc[entry->Source], entry->Score);
		maxDes[entry->Target] = MAX(maxDes[entry->Target], entry->Score);
	}
	VECTOR<pair<INT64, FLOAT>> res;
	res.reserve(maxSrc.size() + maxDes.size());
	for (auto entry = ens->begin(); entry != ens->end(); entry++)
	{
		FLOAT mS = maxSrc[entry->Source];
		FLOAT mD = maxDes[entry->Target];
		if (entry->Score > 0 && entry->Score >= filteringThreshold && entry->Score >= filteringFactor * MAX(mS, mD))
			res.push_back(pair<INT64, FLOAT>(((INT64)entry->Target << 32) | entry->Source, entry->Score));
	}
	res.shrink_to_fit();

	return res;
}

VECTOR<pair<INT64, FLOAT>> StableFiltering(STRING strScoreFile, FLOAT filteringFactor, FLOAT filteringThreshold)
{
	MAP<INT32, FLOAT> maxSrc;
	MAP<INT32, FLOAT> maxDes;
	ifstream fi(strScoreFile, ios::in | ios::binary);
	INT64 entryCount;
	fi.read((INT8*)&entryCount, sizeof(INT64));
	ScoreEntry ens[1024];
	INT64 read = 0;
	while (read < entryCount)
	{
		INT32 toread = (INT32)MIN(entryCount - read, 1024);
		fi.read((INT8*)&ens, sizeof(ScoreEntry) * toread);
		read += toread;
		for (INT32 i = 0; i < toread; i++)
		{
			auto &entry = ens[i];
			if (entry.Score > 0 && entry.Score >= filteringThreshold)
			{
				maxSrc[entry.Source] = MAX(maxSrc[entry.Source], entry.Score);
				maxDes[entry.Target] = MAX(maxDes[entry.Target], entry.Score);
			}
		}
	}
	
	fi.seekg(sizeof(INT64), fi.beg);
	read = 0;
	VECTOR<pair<INT64,FLOAT>> res;
	res.reserve(maxSrc.size() + maxDes.size());
	while (read < entryCount)
	{
		INT32 toread = (INT32)MIN(entryCount - read, 1024);
		fi.read((INT8*)&ens, sizeof(ScoreEntry)* toread);
		read += toread;
		for (INT32 i = 0; i < toread; i++)
		{
			auto &entry = ens[i];
			FLOAT mS = maxSrc[entry.Source];
			FLOAT mD = maxDes[entry.Target];
			if (entry.Score > 0 && entry.Score >= filteringThreshold && entry.Score >= filteringFactor * MAX(mS, mD))
				res.push_back(pair<INT64, FLOAT>(((INT64)entry.Target << 32) | entry.Source, entry.Score));
		}
	}
	res.shrink_to_fit();
	fi.close();
	return res;
}



VECTOR<pair<INT64, FLOAT>> TopKFiltering(STRING strScoreFile, INT32 topK, FLOAT filteringThreshold)
{
	MAP<INT32, VECTOR<ScoreEntry>> bySrc;
	ifstream fi(strScoreFile, ios::in | ios::binary);
	INT64 entryCount;
	fi.read((INT8*)&entryCount, sizeof(INT64));
	ScoreEntry ens[1024];
	INT64 read = 0;
	while (read < entryCount)
	{
		INT32 toread = (INT32)MIN(entryCount - read, 1024);
		fi.read((INT8*)&ens, sizeof(ScoreEntry)* toread);
		read += toread;

		for (INT32 i = 0; i < toread; i++)
		{
			auto &entry = ens[i];
			if (entry.Score > 0 && entry.Score >= filteringThreshold)
			{
				if (!Contains(bySrc, entry.Source))
					bySrc[entry.Source].reserve(topK);
				VECTOR<ScoreEntry> &v = bySrc[entry.Source];
				
				if (v.size() == 0)
					v.push_back(entry);
				else
				{
					auto pos = v.begin();
					while (pos != v.end() && pos->Score < entry.Score)
						pos++;
					if (v.size() < v.capacity())
						v.insert(pos, entry);
					else
						if (pos == v.begin() && pos->Score < entry.Score)
							v[0] = entry;
				}
			}
		}
	}

	VECTOR<pair<INT64, FLOAT>> res;
	res.reserve(bySrc.size()*topK);
	for (auto entry : bySrc)
		for(auto e : entry.second)
		if (e.Score > 0 && e.Score >= filteringThreshold)
			res.push_back(pair<INT64, FLOAT>(((INT64)e.Target << 32) | e.Source, e.Score));
	
	fi.close();
	return res;
}
