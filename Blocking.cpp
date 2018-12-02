#include "Resolution.h"
#include "Utility.h"
#include <algorithm>

VOID Merge(VECTOR<STRING> strFiles, STRING strOutput);
INT32 CompareBlockEntry(INT64 X, INT64 Y){ return X < Y; };
VOID SortAndRemoveDuplicate(VECTOR<INT64> &cans);
VOID Save(STRING strFile, VECTOR<INT64> &cans);

VOID Block(DataSource* dSrc, DataSource* dDes, VECTOR<Alignment> *alg, VECTOR<VECTOR<INT32>> *sharedTokens, SET<INT32> *srcIns, SystemParameters params, STRING strOutputFile)
{
	INT32 bufSize = 0x10000000; //2GB
	INT32 writeSize = bufSize >> 1;
	MAP<INT32, MAP<INT32, VECTOR<INT32>>> cacheSrc;
	MAP<INT32, MAP<INT32, VECTOR<INT32>>> cacheDes;
	INT64 tokenCount = 0;
	for (INT32 i = 0; i < sharedTokens->size(); i++) //note: sharedTokens->size() is smaller than alg->size()
	{
		INT32 src = alg->at(i).Source; //predicate src
		INT32 des = alg->at(i).Target; //predicate des
		if (!Contains(cacheSrc, src))
			cacheSrc[src] = dSrc->ReadPOSEntry(src);
		if (!Contains(cacheDes, des))
			cacheDes[des] = dDes->ReadPOSEntry(des);
		//cout << sharedTokens->at(i).size() << "," << cacheSrc[src].size() << "," << cacheDes[des].size() << "," << src << "," << des << endl;
		tokenCount += sharedTokens->at(i).size();
	}
	
	INT8 outBuf[1024];
	INT32 id = 0;
	VECTOR<STRING> strFiles;
	VECTOR<INT64> cans;
	cans.reserve(bufSize);
	Loop(tokenCount);
	
	for (INT32 i = 0; i < sharedTokens->size(); i++)
	{
		for (auto token : sharedTokens->at(i))
		{
			auto vs = cacheSrc[alg->at(i).Source][token];
			auto vd = cacheDes[alg->at(i).Target][token];
			for (auto s : vs)
			if (srcIns == 0 || (srcIns!=0 && RefContains(srcIns, s)))
			for (auto d : vd)
			{
				cans.push_back(((INT64)d << 32) | s);
				if (cans.size() == bufSize)
				{
					SortAndRemoveDuplicate(cans);
					if (cans.size() >= writeSize)
					{
						sprintf(outBuf, "%d", id++);
						STRING str(params.TempDirectory + params.Jobname + outBuf);
						Save(str, cans);
						cans.clear();
						strFiles.push_back(str);
					}
				}
			}
			Loop();
		}
	}

	if (cans.size() > 0)
		SortAndRemoveDuplicate(cans);

	if (id == 0)
	{
		Save(strOutputFile, cans);
		cans.clear();
	}
	else
	{
		if (cans.size() > 0)
		{
			sprintf(outBuf, "%d", id++);
			STRING str(params.TempDirectory + params.Jobname + outBuf);
			Save(str, cans);
			cans.clear();
			strFiles.push_back(str);
		}
		Merge(strFiles, strOutputFile);
		for (auto str : strFiles)
			CMD(RM + str);
	}
}

VOID SortAndRemoveDuplicate(VECTOR<INT64> &cans)
{
	sort(cans.begin(), cans.end(), CompareBlockEntry);
	INT32 size = 1;
	for (INT32 i = 1; i < cans.size(); i++)
	if (cans[size - 1] != cans[i])
		cans[size++] = cans[i];
	cans.resize(size);
}

VOID Save(STRING strFile, VECTOR<INT64> &cans)
{
	ofstream fo(strFile, ios::binary | ios::out);
	INT64 size = cans.size();
	fo.write((INT8*)&size,sizeof(INT64));
	fo.write((INT8*)&cans[0], size * sizeof(INT64));	
	fo.close();
}

VOID Merge(VECTOR<STRING> strFiles, STRING strOutput)
{
	INT32 N = strFiles.size();
	INT32 bufSize = 0x10000000;	
	INT32 bufSizeSep = bufSize / N;
	VECTOR<ifstream> fi(N);
	VECTOR<INT32> length(N);
	VECTOR<INT32> remain(N);
	VECTOR<VECTOR<INT64>> value(N);
	INT64 itemCount = 0;
	for (INT32 i = 0; i < N; i++)
	{
		fi[i].open(strFiles[i], ios::in | ios::binary);
		fi[i].read((INT8*)&remain[i], sizeof(INT64));
		value[i].resize(bufSizeSep);
		itemCount += remain[i];		
	}

	VECTOR<INT64> outBuf;
	outBuf.reserve(bufSize);
	INT64 prv = -1; 

	INT64 write = 0;
	ofstream fo(strOutput, ios::binary);
	fo.write((INT8*)&write, sizeof(INT64)); //reserved 
	
	INT64 processed = 0;
	Loop(itemCount);
	while (processed < itemCount)
	{
		//read
		for (INT32 i = 0; i < N; i++)
		{
			INT32 toRead = MIN(remain[i], bufSizeSep - length[i]);
			if (toRead > 0)
			{
				fi[i].read((INT8*)&value[i][length[i]], sizeof(INT64)* toRead);
				remain[i] -= toRead;
				length[i] += toRead;
				processed += toRead;
			}
		}

		//find min
		INT64 min = -1;
		for (INT32 i = 0; i < N; i++)
		if (length[i] > 0)
		{
			if (min == -1)
				min = value[i][length[i] - 1];
			else
				min = MIN(min, value[i][length[i] - 1]);
		}

		//output to buffer
		for (INT32 i = 0; i < N; i++)
		{
			INT32 j = 0;
			while (j < length[i] && value[i][j] <= min)
				outBuf.push_back(value[i][j++]);
			//remove 
			length[i] -= j;
			for (INT32 k = 0; k < length[i]; k++, j++)
				value[i][k] = value[i][j];
		}
		SortAndRemoveDuplicate(outBuf);		
		fo.write((INT8*)&outBuf[0], sizeof(INT64)* outBuf.size());
		write += outBuf.size();

		outBuf.clear();
		Loop(processed);		
	}

	fo.seekp(0, fo.beg);
	fo.write((INT8*)&write, sizeof(INT64));
	fo.close();
	for (INT32 i = 0; i < N; i++)
		fi[i].close();

	//CMD(STRING("pause"));
}

