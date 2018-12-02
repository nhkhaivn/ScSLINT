#include "Resolution.h"
#include "Utility.h"
#include <cmath>

FLOAT exactMatch(STRING s, STRING d);
FLOAT reverseDifferent(FLOAT s, FLOAT d);
FLOAT levenshtein(VECTOR<INT32> &s, VECTOR<INT32> &d);
FLOAT jaroWinkler(VECTOR<INT32> &s, VECTOR<INT32> &d);
FLOAT longestCommonSubstring(VECTOR<INT32>& s, VECTOR<INT32>& d);
FLOAT longestCommonPrefix(VECTOR<INT32>& s, VECTOR<INT32>& d);
FLOAT jaccard(VECTOR<INT32> &s, VECTOR<INT32> &d);
FLOAT tfidfCosine(DataSource *dSrc, DataSource *dDes, SPOEntry *src, SPOEntry *des, VECTOR<INT32> &s, VECTOR<INT32> &d);
FLOAT modifiedBM25(DataSource *dSrc, DataSource *dDes, SPOEntry *src, SPOEntry *des, VECTOR<INT32> &s, VECTOR<INT32> &d);
FLOAT jaccardBM25(DataSource *dSrc, DataSource *dDes, SPOEntry *src, SPOEntry *des, VECTOR<INT32> &s, VECTOR<INT32> &d);

//Exact match
FLOAT ExactMatching(DataSource *dSrc, DataSource *dDes, SPOEntry *src, SPOEntry *des, INT32 pSrc, INT32 pDes)
{
	if (!Contains(src->Original, pSrc) || !Contains(des->Original, pDes))
		return -1;
	auto vecS = &src->Original[pSrc];
	auto vecD = &des->Original[pDes];
	INT32 oSrc = (INT32)vecS->size();
	INT32 oDes = (INT32)vecD->size();
	for (INT32 i = 0; i < oSrc; i++)
	for (INT32 j = 0; j < oDes; j++)
	if (vecS->at(i).compare(vecD->at(j)) == 0)
		return 1;
	return 0;
}

//for number
FLOAT ReverseDifference(DataSource *dSrc, DataSource *dDes, SPOEntry *src, SPOEntry *des, INT32 pSrc, INT32 pDes)
{
	if (!Contains(src->Number, pSrc) || !Contains(des->Number, pDes))
		return -1;
	FLOAT max = 0;
	auto vecS = &src->Number[pSrc];
	auto vecD = &des->Number[pDes];
	INT32 oSrc = (INT32)vecS->size();
	INT32 oDes = (INT32)vecD->size();
	for (INT32 i = 0; i < oSrc; i++)
	for (INT32 j = 0; j < oDes; j++)
	{
		FLOAT temp = 1.0f / (1.0f + fabs(vecS->at(i) - vecD->at(j))); 
		if (max < temp)
			max = temp;
	}
	return max;
}

FLOAT Levenshtein(DataSource *dSrc, DataSource *dDes, SPOEntry *src, SPOEntry *des, INT32 pSrc, INT32 pDes)
{
	if (!Contains(src->Token, pSrc) || !Contains(des->Token, pDes))
		return -1;
	FLOAT max = 0;
	auto vecS = &src->Original[pSrc];
	auto vecD = &des->Original[pDes];
	INT32 oSrc = (INT32)vecS->size();
	INT32 oDes = (INT32)vecD->size();
	for (INT32 i = 0; i < oSrc; i++)
	for (INT32 j = 0; j < oDes; j++)
	{
		auto s = ToUTF32(vecS->at(i));
		auto d = ToUTF32(vecD->at(j));
		FLOAT temp = levenshtein(s, d);
		if (max < temp)
			max = temp;
	}
	return max;
}

FLOAT JaroWinkler(DataSource *dSrc, DataSource *dDes, SPOEntry *src, SPOEntry *des, INT32 pSrc, INT32 pDes)
{
	if (!Contains(src->Token, pSrc) || !Contains(des->Token, pDes))
		return -1;
	FLOAT max = 0;
	auto vecS = &src->Original[pSrc];
	auto vecD = &des->Original[pDes];
	INT32 oSrc = (INT32)vecS->size();
	INT32 oDes = (INT32)vecD->size();
	for (INT32 i = 0; i < oSrc; i++)
	for (INT32 j = 0; j < oDes; j++)
	{
		auto s = ToUTF32(vecS->at(i));
		auto d = ToUTF32(vecD->at(j));
		FLOAT temp = jaroWinkler(s, d);
		if (max < temp)
			max = temp;
	}
	return max;
}

FLOAT LongestCommonSubstring(DataSource *dSrc, DataSource *dDes, SPOEntry *src, SPOEntry *des, INT32 pSrc, INT32 pDes)
{
	if (!Contains(src->Token, pSrc) || !Contains(des->Token, pDes))
		return -1;
	FLOAT max = 0;
	auto vecS = &src->Original[pSrc];
	auto vecD = &des->Original[pDes];
	INT32 oSrc = (INT32)vecS->size();
	INT32 oDes = (INT32)vecD->size();
	for (INT32 i = 0; i < oSrc; i++)
	for (INT32 j = 0; j < oDes; j++)
	{
		auto s = ToUTF32(vecS->at(i));
		auto d = ToUTF32(vecD->at(j));
		FLOAT temp = longestCommonSubstring(s, d);
		if (max < temp)
			max = temp;
	}
	return max;
}

FLOAT LongestCommonPrefix(DataSource *dSrc, DataSource *dDes, SPOEntry *src, SPOEntry *des, INT32 pSrc, INT32 pDes)
{
	if (!Contains(src->Token, pSrc) || !Contains(des->Token, pDes))
		return -1;
	FLOAT max = 0;
	auto vecS = &src->Original[pSrc];
	auto vecD = &des->Original[pDes];
	INT32 oSrc = (INT32)vecS->size();
	INT32 oDes = (INT32)vecD->size();
	for (INT32 i = 0; i < oSrc; i++)
	for (INT32 j = 0; j < oDes; j++)
	{
		auto s = ToUTF32(vecS->at(i));
		auto d = ToUTF32(vecD->at(j));
		FLOAT temp = longestCommonPrefix(s, d);
		if (max < temp)
			max = temp;
	}
	return max;
}

FLOAT Jaccard(DataSource *dSrc, DataSource *dDes, SPOEntry *src, SPOEntry *des, INT32 pSrc, INT32 pDes)
{
	if (!Contains(src->Token, pSrc) || !Contains(des->Token, pDes))
		return -1;
	FLOAT max = 0;
	auto vecS = &src->Token[pSrc];
	auto vecD = &des->Token[pDes];
	INT32 oSrc = (INT32)vecS->size();
	INT32 oDes = (INT32)vecD->size();
	for (INT32 i = 0; i < oSrc; i++)
	for (INT32 j = 0; j < oDes; j++)
	{
		auto s = vecS->at(i);
		auto d = vecD->at(j);
		FLOAT temp = jaccard(s, d);
		if (max < temp)
			max = temp;
	}
	return max;
}

FLOAT TFIDFCosine(DataSource *dSrc, DataSource *dDes, SPOEntry *src, SPOEntry *des, INT32 pSrc, INT32 pDes)
{
	if (!Contains(src->Token, pSrc) || !Contains(des->Token, pDes))
		return -1;
	FLOAT max = 0;
	auto vecS = &src->Token[pSrc];
	auto vecD = &des->Token[pDes];
	INT32 oSrc = (INT32)vecS->size();
	INT32 oDes = (INT32)vecD->size();
	for (INT32 i = 0; i < oSrc; i++)
	for (INT32 j = 0; j < oDes; j++)
	{
		auto s = vecS->at(i);
		auto d = vecD->at(j);
		FLOAT temp = tfidfCosine(dSrc, dDes, src, des, s, d);
		if (max < temp)
			max = temp;
	}

	return max;
}


FLOAT ModifiedBM25(DataSource *dSrc, DataSource *dDes, SPOEntry *src, SPOEntry *des, INT32 pSrc, INT32 pDes)
{
	if (!Contains(src->Token, pSrc) || !Contains(des->Token, pDes))
		return -1;
	auto vecS = &src->Token[pSrc];
	auto vecD = &des->Token[pDes];
	INT32 oSrc = (INT32)vecS->size();
	INT32 oDes = (INT32)vecD->size();
	FLOAT max = 0;
	for (INT32 i = 0; i < oSrc; i++)
	for (INT32 j = 0; j < oDes; j++)
	{
		auto s = vecS->at(i);
		auto d = vecD->at(j);
		FLOAT temp = modifiedBM25(dSrc, dDes, src, des, s, d);
		if (max < temp)
			max = temp;
	}
	return max;
}


FLOAT JaccardBM25(DataSource *dSrc, DataSource *dDes, SPOEntry *src, SPOEntry *des, INT32 pSrc, INT32 pDes)
{
	if (!Contains(src->Token, pSrc) || !Contains(des->Token, pDes))
		return -1;
	auto vecS = &src->Token[pSrc];
	auto vecD = &des->Token[pDes];
	INT32 oSrc = (INT32)vecS->size();
	INT32 oDes = (INT32)vecD->size();
	FLOAT max = 0;
	for (INT32 i = 0; i < oSrc; i++)
		for (INT32 j = 0; j < oDes; j++)
		{
			auto s = vecS->at(i);
			auto d = vecD->at(j);
			FLOAT temp = jaccardBM25(dSrc, dDes, src, des, s, d);
			if (max < temp)
				max = temp;
		}
	return max;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Exact match
FLOAT exactMatch(STRING &s, STRING &d)
{
	return s.compare(d) == 0;
}

FLOAT reverseDifferent(FLOAT s, FLOAT d)
{
	return 1.0f / (1.0f + fabs(s - d));
}

FLOAT levenshtein(VECTOR<INT32> &s, VECTOR<INT32> &d)
{
	INT32 ls = (INT32)s.size(), ld = (INT32)d.size();
	VECTOR<INT32> col(ld + 1), prevCol(ld + 1);
	for (INT32 x = 0; x < prevCol.size(); x++)
		prevCol[x] = x;
	for (INT32 x = 0; x < ls; x++)
	{
		col[0] = x + 1;
		for (INT32 y = 0; y < ld; y++)
			col[y + 1] = MIN(MIN(prevCol[y + 1] + 1, col[y] + 1), prevCol[y] + (s[x] == d[y] ? 0 : 1));
		col.swap(prevCol);
	}
	return 1.0f - (FLOAT)prevCol[ld] / MAX(ls, ld);
}

FLOAT jaroWinkler(VECTOR<INT32> &s, VECTOR<INT32> &d)
{
	int i, j, l;
	int m = 0, t = 0;
	int sl = s.size();
	int al = d.size();
	VECTOR<INT32> sflags(sl), aflags(al);
	int range = MAX(0, MAX(sl, al) / 2 - 1);
	FLOAT dw;

	if (!sl || !al)
		return 0.0;

	for (i = 0; i < al; i++)
		aflags[i] = 0;

	for (i = 0; i < sl; i++)
		sflags[i] = 0;

	/* calculate matching characters */
	for (i = 0; i < al; i++)
	{
		for (j = MAX(i - range, 0), l = MIN(i + range + 1, sl); j < l; j++)
		{
			if (d[i] == s[j] && !sflags[j])
			{
				sflags[j] = 1;
				aflags[i] = 1;
				m++;
				break;
			}
		}
	}

	if (!m)
		return 0.0;

	/* calculate character transpositions */
	l = 0;
	for (i = 0; i < al; i++)
	{
		if (aflags[i] == 1)
		{
			for (j = l; j < sl; j++)
			{
				if (sflags[j] == 1)
				{
					l = j + 1;
					break;
				}
			}
			if (d[i] != s[j])
				t++;
		}
	}
	t /= 2;

	/* Jaro distance */
	dw = (((FLOAT)m / sl) + ((FLOAT)m / al) + ((FLOAT)(m - t) / m)) / 3.0;

	/* calculate common string prefix up to 4 chars */
	l = 0;
	for (i = 0; i < MIN(MIN(sl, al), 4); i++)
	if (s[i] == d[i])
		l++;

	/* Jaro-Winkler distance */
	dw = dw + (l * 0.1F * (1 - dw));	
	dw = 1.0f - dw;

	return dw;
}

FLOAT longestCommonSubstring(VECTOR<INT32>& str1, VECTOR<INT32>& str2)
{
	if (str1.empty() || str2.empty())
		return 0;
	int *curr = new int[str2.size()];
	int *prev = new int[str2.size()];
	int *swap = nullptr;
	int maxSubstr = 0;

	for (int i = 0; i<str1.size(); ++i)
	{
		for (int j = 0; j<str2.size(); ++j)
			if (str1[i] != str2[j])
				curr[j] = 0;
			else
			{
				if (i == 0 || j == 0)
					curr[j] = 1;
				else
					curr[j] = 1 + prev[j - 1];
				if (maxSubstr < curr[j])
					maxSubstr = curr[j];
			}
		swap = curr;
		curr = prev;
		prev = swap;
	}
	delete[] curr;
	delete[] prev;

	return 2.0F * maxSubstr / (str1.size() + str2.size());
}

FLOAT longestCommonPrefix(VECTOR<INT32>& str1, VECTOR<INT32>& str2)
{
	int i = 0;
	while (i < str1.size() && i < str2.size() && str1[i] == str2[i])
		i++;
	return 2.0F * i / (str1.size() + str2.size());
}

FLOAT jaccard(VECTOR<INT32> &s, VECTOR<INT32> &d)
{
	return (FLOAT)Intersect(s, d).size() / Union(s, d).size();
}

FLOAT tfidfCosine(DataSource *dSrc, DataSource *dDes, SPOEntry *src, SPOEntry *des, VECTOR<INT32> &s, VECTOR<INT32> &d)
{
	FLOAT dLR = 0;
	FLOAT dL = 0;
	FLOAT dR = 0;
	SET<INT32> L(s.begin(), s.end());
	SET<INT32> R(d.begin(), d.end());
	SET<INT32> I(L.size());
	for (INT32 k : R)
	if (Contains(L, k))
		I.insert(k);
	FLOAT temp = 0;
	if (I.size() > 0)
	{
		SET<INT32> U(L.begin(), L.end());
		U.insert(R.begin(), R.end());
		for (INT32 x : U)
		{
			FLOAT wL = 0;
			FLOAT wR = 0;
			if (Contains(L, x))
			{
				wL = src->TF.find(x)->second * dSrc->IDF.find(x)->second;
				dL += wL * wL;
			}
			if (Contains(R, x))
			{
				wR = des->TF.find(x)->second * dDes->IDF.find(x)->second;
				dR += wR * wR;
			}
			if (wL != 0 && wR != 0)
				dLR += wL * wR;
		}

		temp = dLR / sqrt(dL*dR);
	}
	return temp;
}

FLOAT modifiedBM25(DataSource *dSrc, DataSource *dDes, SPOEntry *src, SPOEntry *des, VECTOR<INT32> &s, VECTOR<INT32> &d)
{
	FLOAT max = 0;
	FLOAT b = 0.75f;
	FLOAT k = 1.2f;
	FLOAT nt = k + 1;
	FLOAT dt = k * (1 - b);
	FLOAT wd = 0;
	FLOAT temp = (FLOAT)Intersect(s, d).size() / Union(s, d).size();
	if (temp != 0)
	{
		FLOAT dbm = 0;
		INT32 px, py = -1;
		for (INT32 x = 0; x < s.size(); x++)
		for (INT32 y = 0; y < d.size(); y++)
		if (s[x] == d[y])
		{
			if (py == -1)
			{
				px = x;
				py = y;
			}
			FLOAT diff = 1.0f / (1.0f + abs(x - y - px + py));
			INT32 w = s[x];
			FLOAT idfDes = dDes->IDF.find(w)->second;
			FLOAT tfDes = des->TF[w];
			//FLOAT bm = idfDes * tfDes * (k + 1) / (tfDes + k * (1 - b) * wD);
			FLOAT bm = idfDes * tfDes * nt / (tfDes + dt);
		
			dbm += diff * bm;
		}

		temp *= dbm;
	}
	return temp;
}


FLOAT jaccardBM25(DataSource *dSrc, DataSource *dDes, SPOEntry *src, SPOEntry *des, VECTOR<INT32> &s, VECTOR<INT32> &d)
{
	FLOAT max = 0;
	FLOAT b = 0.75f;
	FLOAT k = 1.2f;
	FLOAT nt = k + 1;
	FLOAT dt = k * (1 - b);
	FLOAT wd = 0;
	FLOAT temp = (FLOAT)Intersect(s, d).size() / Union(s, d).size();
	if (temp != 0)
	{
		FLOAT dbm = 0;
		for (INT32 x = 0; x < s.size(); x++)
			for (INT32 y = 0; y < d.size(); y++)
				if (s[x] == d[y])
				{
					INT32 w = s[x];
					FLOAT idfDes = dDes->IDF.find(w)->second;
					FLOAT tfDes = des->TF[w];
					//FLOAT bm = idfDes * tfDes * (k + 1) / (tfDes + k * (1 - b) * wD);
					FLOAT bm = idfDes * tfDes * nt / (tfDes + dt);
					dbm += bm;
				}
		temp *= dbm;
	}
	return temp;
}
