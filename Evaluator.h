#pragma once

#include "Core.h"

struct RPF
{
	INT64 Expected, Detected, Correct;
	FLOAT Recall, Precision, F1;
};

//Evaluator.cpp
SET<INT64> LoadReference(STRING strFile);
SET<INT64> AdaptToSplit(SET<INT64> *reference, SET<INT32> *instance);
RPF Evaluate(VECTOR<pair<INT64, FLOAT>> *detected, SET<INT64> *reference);
RPF Evaluate(STRING strBlockFile, SET<INT64> *reference);
