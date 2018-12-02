#include "Utility.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <time.h>

using namespace std;

ofstream LOGSTREAM;
mutex LOGMT;
INT64 loopState = 0;
INT64 finalState = 0;
INT64 startTime = 0;
INT64 previousElapsed = 0;

#define CLRLN LOGSTR(cout, "\r                                                                               \r")

INT32 Loop(INT64 state)
{
	if (finalState == 0) //reset
	{
		finalState = state;
		loopState = 0;
		previousElapsed = 0;
		CLRLN;
		LOGSTRSAFE(cout, setprecision(2) << fixed << "\r0/" << finalState << " (0%) | " << PrintTime(0) << " | " << PrintTime(0));
		startTime = TickCount();
		return 0;
	}

	if (state == 1) //simple call: Loop()
		state = loopState + 1;

	//output consideration
	FLOAT prepercent = (INT32)((FLOAT)loopState * 100 / finalState / 0.01F) * 0.01;
	FLOAT percent = (INT32)((FLOAT)state * 100 / finalState / 0.01F) * 0.01;
	INT64 elapsed = TickCount() - startTime;
	if (percent > prepercent && elapsed - previousElapsed >= 1)
	{
		INT64 remain = (INT64)(elapsed * ((FLOAT)finalState / state - 1.0));
		LOGSTRSAFE(cout, setprecision(2) << fixed << "\r" << state << "/" << finalState << " (" << percent << "%) | " << PrintTime(elapsed) << " | " << PrintTime(remain));
		previousElapsed = elapsed;
	}

	//update
	loopState = state;
	if (loopState == finalState)
	{
		CLRLN;
		LOGSTRSAFE(cout, setprecision(2) << fixed << "\r" << state << "/" << finalState << " (100%) | " << PrintTime(elapsed) << " | " << PrintTime(0) << endl);
		finalState = 0; //reset 
		return 1;
	}
	return 0;
}

INT64 TickCount() //return time in seconds
{
#ifdef WIN
	return time(0);
#else
	timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts); //BSD
	return ts.tv_sec;
#endif
}

STRING Now()
{
	INT64 t = time(0);   // get time now
	auto now = localtime(&t);
	INT8 s[16];
	sprintf(s, "%04d%02d%02d%02d%02d%02d", now->tm_year + 1900, now->tm_mon + 1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);
	return STRING(s);
}

STRING PrintTime(INT64 timeInSeconds)
{
	INT32 seconds = timeInSeconds % 60; timeInSeconds /= 60;
	INT32 minutes = timeInSeconds % 60; timeInSeconds /= 60;
	INT32 hours = timeInSeconds % 24; timeInSeconds /= 24;
	INT32 days = timeInSeconds;
	INT8 s[16];
	sprintf(s, "%d.%02d:%02d:%02d", days, hours, minutes, seconds);
	return STRING(s);
}


SET<INT32> Intersect(VECTOR<INT32> &A, VECTOR<INT32> &B)
{
	SET<INT32> T(A.begin(), A.end());
	SET<INT32> C(T.size());
	for (INT32 b : B)
	if (Contains(T,b))
		C.insert(b);
	return C;
}

SET<INT32> Union(VECTOR<INT32> &A, VECTOR<INT32> &B)
{
	SET<INT32> C(A.size() + B.size());
	C.insert(A.begin(), A.end());
	C.insert(B.begin(), B.end());
	return C;
}

