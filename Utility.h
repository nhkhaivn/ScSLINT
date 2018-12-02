#pragma once

#include "Core.h"
#include <iostream>
#include <fstream>
#include <mutex>

//Log.cpp  =============================================================================================================
extern mutex LOGMT;
extern ofstream LOGSTREAM;
#define LOGALL(MESSAGE) {cout << MESSAGE << flush; LOGSTREAM << MESSAGE << flush;}
#define LOGSTR(STREAM, MESSAGE) { STREAM << MESSAGE << flush; }
#define LOGALLSAFE(MESSAGE) {LOGMT.lock(); cout << MESSAGE << flush; LOGSTREAM << MESSAGE << flush; LOGMT.unlock();}
#define LOGSTRSAFE(STREAM, MESSAGE) {LOGMT.lock(); STREAM << MESSAGE << flush; LOGMT.unlock();}

INT64 TickCount();
STRING Now();
STRING PrintTime(INT64 timeInSeconds);
INT32 Loop(INT64 state = 1);

VECTOR<INT32> ToUTF32(STRING utf8);
SET<INT32> Intersect(VECTOR<INT32> &A, VECTOR<INT32> &B);
SET<INT32> Union(VECTOR<INT32> &A, VECTOR<INT32> &B);