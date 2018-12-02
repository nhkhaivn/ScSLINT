#include "Resolution.h"
#include "StreamIO.h"
#include <cmath>

DataSource::DataSource(STRING indexDirectory) 
{
	//Init
	readPropertyFunction.push_back(&DataSource::readSTRING);
	readPropertyFunction.push_back(&DataSource::readNumber);
	readPropertyFunction.push_back(&DataSource::readNumber);
	readPropertyFunction.push_back(&DataSource::readOriginal);
	readPropertyFunction.push_back(&DataSource::readOriginal);
	
	//Property of INT32erest
	this->indexDirectory = indexDirectory;

	//Read info
	INT8 *ptr, *buf;
	ptr = buf = ReadAllBytes(indexDirectory + SLASH + "info");
	SPOEntryCount = ReadINT32(ptr);
	TotalTokenCount = ReadINT32(ptr);
	MaxTokenCount = ReadINT32(ptr);
	TotalTripleCount = ReadINT32(ptr);
	MaxTripleCount = ReadINT32(ptr);
	delete[] buf;	
}

VOID DataSource::ResetPointer()
{
	iSPO.ResetPointer();
	iPO.ResetPointer();
	iPOS.ResetPointer();
}

VOID DataSource::LoadSPO(MAP<INT32, INT32> poI)
{
	this->poI = poI;
	iSPO.Load(indexDirectory + SLASH + "spo", indexDirectory + SLASH + "index" + SLASH + "spo_i");
}

VOID DataSource::LoadPO(INT32 nFirstTokenCount)
{
	if (nFirstTokenCount == -1)
		iPO.Load(indexDirectory + SLASH + "po", indexDirectory + SLASH + "index" + SLASH + "po_i");
	else
	{
		INT8 outBuf[16];
		sprintf(outBuf, "po.prefix%d", nFirstTokenCount);
		iPO.Load(indexDirectory + SLASH + outBuf, indexDirectory + SLASH + "index" + SLASH + outBuf + "_i");
	}	
}

VOID DataSource::LoadPOS(INT32 nFirstTokenCount)
{
	if (nFirstTokenCount == -1)
		iPOS.Load(indexDirectory + SLASH + "pos", indexDirectory + SLASH + "index" + SLASH + "pos_i");
	else
	{
		INT8 outBuf[16];
		sprintf(outBuf, "pos.prefix%d", nFirstTokenCount);
		iPOS.Load(indexDirectory + SLASH + outBuf, indexDirectory + SLASH + "index" + SLASH + outBuf + "_i");
	}
}

VOID DataSource::LoadIDF()
{
	//Read IDF 
	FLOAT min = 0, max = 0;
	INT8 *buf = ReadAllBytes(indexDirectory + SLASH + "idf");
	INT8* ptr = buf + (INT32)FileLength(indexDirectory + SLASH + "idf") - 8;
	INT32 count = (INT32)ReadINT64(ptr); ptr = buf;
	IDF.reserve(count);
	for (INT32 i = 0; i < count; i++)
	{
		STRING str = ReadSTRING(ptr);
		FLOAT idf = (FLOAT)ReadINT32(ptr);
		INT32 id = tokenID[str];		
		idf = log10((SPOEntryCount - idf + 0.5f) / (idf + 0.5f));
		IDF[id] = idf;
		if (min > idf)
			min = idf;
		if (max < idf)
			max = idf;
	}
	FLOAT del = max - min;
	min -= 1.4013e-045f;
	for (auto iter = IDF.begin(); iter != IDF.end(); iter++)
		iter->second = (iter->second - min) / del;
	delete[] buf;
}

VECTOR<PredicateInfo> DataSource::ReadPredicateInfo()
{
	INT8 *ptr, *buf;
	buf = ReadAllBytes(indexDirectory + SLASH + "p_ex");
	ptr = buf + (INT32)FileLength(indexDirectory + SLASH + "p_ex") - 8;
	if (!buf)
	{
		buf = ReadAllBytes(indexDirectory + SLASH + "p");
		ptr = buf + (INT32)FileLength(indexDirectory + SLASH + "p") - 8;
	}
	
	INT32 count = (INT32)ReadINT64(ptr); ptr = buf;
	VECTOR<PredicateInfo> pInfo(count);
	for (INT32 i = 0; i < count; i++)
	{
		pInfo[i].Name = ReadSTRING(ptr);
		pInfo[i].ID = ReadINT32(ptr);
		pInfo[i].Frequency = ReadINT32(ptr);
		pInfo[i].ParentCount = ReadINT32(ptr);
		pInfo[i].MeanLength = (FLOAT)ReadDOUBLE(ptr);
		pInfo[i].StdLength = (FLOAT)ReadDOUBLE(ptr);
		ptr += sizeof(INT32) + 5 * 6 * sizeof(INT32);
		pInfo[i].Type = ReadINT32(ptr);
		pInfo[i].Coverage = (FLOAT)ReadDOUBLE(ptr);
		pInfo[i].Diversity = (FLOAT)ReadDOUBLE(ptr);
		ptr += sizeof(DOUBLE);
	}
	return pInfo;
}

VECTOR<INT32> DataSource::ReadPOEntry(INT32 ID) //read entry and conver to token ID
{
	INT8 *buf = iPO.Read(ID);
	INT8 *ptr = buf;
	VECTOR<INT32> token;
	INT32 id = ReadINT32(ptr);
	if (id == ID)
	{
		INT32 count = ReadINT32(ptr);
		token.resize(count);
		for (INT32 i = 0; i < count; i++)
			token[i] = tokenID[ReadSTRING(ptr)];
	}
	delete[] buf;
	return token;
}

VECTOR<STRING> DataSource::ReadPOEntryRaw(INT32 ID) //read entry and return original value
{
	VECTOR<STRING> token;
	INT8 *buf = iPO.Read(ID);	
	if (buf == 0) //TODO large data
		return token;
	INT8 *ptr = buf;
	INT32 id = ReadINT32(ptr);
	if (id == ID)
	{
		INT32 count = ReadINT32(ptr);
		token.resize(count);
		for (INT32 i = 0; i < count; i++)
			token[i] = ReadSTRING(ptr);
	}
	delete[] buf;
	return token;
}

MAP<INT32, VECTOR<INT32>> DataSource::ReadPOSEntry(INT32 ID) //Input P, read OS 
{
	MAP<INT32, VECTOR<INT32>> osEntry;
	ifstream *fi = iPOS.GetStream(ID);
	INT32 id = ReadINT32(fi);
	if (id == ID)
	{
		INT32 osCount = ReadINT32(fi);
		while (osCount--)
		{
			INT32 token = tokenID[ReadSTRING(fi)]; 
			INT32 sCount = ReadINT32(fi);
			INT32* buf = new INT32[sCount];
			fi->read((INT8*)buf, sCount * sizeof(INT32));
			osEntry[token] = VECTOR<INT32>(buf, buf + sCount);
			delete[] buf;
		}
	}
	return osEntry;
}


MAP<STRING, VECTOR<INT32>> DataSource::ReadPOSEntryRaw(INT32 ID)
{
	MAP<STRING, VECTOR<INT32>> osEntry;
	ifstream *fi = iPOS.GetStream(ID);
	INT32 id = ReadINT32(fi);
	if (id == ID)
	{
		INT32 osCount = ReadINT32(fi);
		while (osCount--)
		{
			STRING token = ReadSTRING(fi);
			INT32 sCount = ReadINT32(fi);
			INT32* buf = new INT32[sCount];
			fi->read((INT8*)buf, sCount * sizeof(INT32));
			osEntry[token] = VECTOR<INT32>(buf, buf + sCount);
			delete[] buf;
		}
	}
	return osEntry;
}

SPOEntry DataSource::ReadSPOEntry(INT32 ID)
{
	INT8 *buf = iSPO.Read(ID);
	INT8 *ptr = buf;

	SPOEntry en;
	INT32 id = ReadINT32(ptr); 
	if (id != ID) //double check
		return en; 
	STRING name = ReadSTRING(ptr);
	en.TripleCount = ReadINT32(ptr);
	INT32 propertyCount = ReadINT32(ptr);

	while (propertyCount--)
	{
		INT32 p = ReadINT32(ptr);
		if (Contains(poI,p)) //found
			(this->*readPropertyFunction[poI[p]])(en, p, ptr);
		else
		{
			INT32 objCount = ReadINT32(ptr);
			while (objCount--)
			{
				ReadSTRING(ptr);
				INT32 tokCount = ReadINT32(ptr);
				while (tokCount--)
					ReadSTRING(ptr);
			}
		}
	}

	//Read TF
	INT32 tfCount = ReadINT32(ptr);
	INT32 maxTF = 0;
	en.TF.reserve(tfCount);
	while (tfCount--)
	{
		STRING tok = ReadSTRING(ptr);
		INT32 frequency = ReadINT32(ptr);
		en.TF[tokenID[tok]] = (FLOAT)frequency;
		if (frequency > maxTF)
			maxTF = frequency;
	}
	for (auto iter = en.TF.begin(); iter != en.TF.end(); iter++)
		iter->second /= maxTF;	

	delete[] buf;

	return en;	
}

VOID DataSource::readOriginal(SPOEntry &en, INT32 nProperty, INT8* &ptr)
{
	INT32 objCount = ReadINT32(ptr);	
	auto &vec = en.Original[nProperty];
	vec.resize(objCount);
	for (INT32 i = 0; i < objCount; i++)
	{
		vec[i] = ReadSTRING(ptr);
		INT32 tokCount = ReadINT32(ptr);
		while (tokCount--)
			ReadSTRING(ptr);
	}
}

VOID DataSource::readNumber(SPOEntry &en, INT32 nProperty, INT8* &ptr)
{
	INT32 objCount = ReadINT32(ptr);
	auto &vec = en.Number[nProperty];
	vec.reserve(objCount);
	for (INT32 i = 0; i < objCount; i++)
	{
		FLOAT number;
		if (ParseNumber(ReadSTRING(ptr), number))
			vec.push_back(number);
		INT32 tokCount = ReadINT32(ptr);
		while (tokCount--)
			ReadSTRING(ptr);
	}
}

VOID DataSource::readSTRING(SPOEntry &en, INT32 nProperty, INT8* &ptr)
{
	INT32 objCount = ReadINT32(ptr);
	auto &vecOriginal = en.Original[nProperty];
	auto &vecToken = en.Token[nProperty];
	vecOriginal.resize(objCount);
	vecToken.resize(objCount);
	for (INT32 i = 0; i < objCount; i++)
	{
		vecOriginal[i] = (ReadSTRING(ptr));
		INT32 tokCount = ReadINT32(ptr);
		vecToken[i].resize(tokCount);
		for (INT32 j = 0; j < tokCount; j++)
			vecToken[i][j] = tokenID[ReadSTRING(ptr)];
	}
}

MAP<STRING, INT32> DataSource::tokenID;

VOID DataSource::LoadTokenID(STRING strSource, STRING strTarget)
{
	VECTOR<STRING> strIndexDirs;
	strIndexDirs.push_back(strSource);
	strIndexDirs.push_back(strTarget);

	INT32 id = 0;
	for (INT32 i = 0; i < strIndexDirs.size(); i++)
	{
		INT8 *buf = ReadAllBytes(strIndexDirs[i] + SLASH + "idf");
		INT8* ptr = buf + (INT32)FileLength(strIndexDirs[i] + SLASH + "idf") - 8;
		INT32 count = (INT32)ReadINT64(ptr); ptr = buf;
		tokenID.reserve(tokenID.size() + count);
		for (INT32 j = 0; j < count; j++)
		{
			STRING token = ReadSTRING(ptr);
			if (i == 0 || Contains(tokenID,token)) //First loop: no redundancy, second loop: there is maybe 
				tokenID[token] = id++;
			ptr += sizeof(INT32); //skip the token frequency 
		}
		delete[] buf;
	}
}

VOID DataSource::ClearTokenID()
{
	tokenID.clear();
}

DataSource* DataSource::Duplicate()
{
	return new DataSource(indexDirectory);
}
