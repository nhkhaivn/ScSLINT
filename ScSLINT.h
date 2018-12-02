#pragma once

#include "Resolution.h"
#include "Learning.h"

//Framework functions

VOID ScSLINT(SystemParameters params); //All steps
VOID GenerateAlignment(SystemParameters params);
VOID GenerateBlock(SystemParameters params);
VOID CreateDefaultConfig(SystemParameters params);
VOID Match(SystemParameters params);
VOID Filter(SystemParameters params);
VOID Filter(SystemParameters params, INT32 topK);
VOID Save(STRING strFile, VECTOR<pair<INT64, FLOAT>> *links); //Save result (only id) as tsv
VECTOR<pair<INT64, FLOAT>> LoadResult(STRING strFile); //Load saved result



