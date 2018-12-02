#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

using namespace std;

//Basic types ============================================================================================================
#ifdef WIN
#define INT8 __int8
#define INT32 __int32
#define INT64 __int64
#define FLOAT float
#define DOUBLE double
#else
#define INT8 char
#define INT32 int32_t
#define INT64 int64_t
#define FLOAT float
#define DOUBLE double
#endif

#define VOID void 
#define STRING std::string
#define MAP std::unordered_map
#define SET std::unordered_set
#define VECTOR std::vector

#ifdef WIN
#define MKDIR STRING("md ")
#define SLASH "\\"
#define RM STRING("del ")
#define RN STRING("move ")
#define XMX	"10G"
#define CMD(X) system((X + " > nul" ).c_str())
#else
#define MKDIR STRING("mkdir ")
#define SLASH "/"
#define RM STRING("rm ")
#define RN STRING("mv ")
#define XMX "200G"
#define CMD(X) system((X + " > /dev/null").c_str())
#endif


#define MIN(A,B) (A<B?A:B)
#define MAX(A,B) (A>B?A:B)
#define LOG(A,B) (log(A)/log(B))
#define Contains(A,B) (A.find(B)!=A.end())
#define RefContains(A,B) (A->find(B)!=A->end())
