#include <string>
#include <iostream>
#include <cstdlib>
using namespace std;
#include <unistd.h>		// sleep
#include "kawari.h"
//#include "nisesakura_sakura.h"	// sakuraスクリプト

char* RandomMessage()
{
	int sleeptime=10;
	string sakuraname="sakura";
	string keroname="unyuu";
	string friendname="sirone";
	char* p_ptr;
	char _path[1024];
	string datapath="";

	TNS_KawariANI NS_Shiori;

	datapath = "./";

	string dllpath=datapath+"shiori.dll";

	if(!NS_Shiori.Load(datapath)) {
	  return NULL;
	}

	NS_Shiori.SetUpNameTable(sakuraname,keroname,friendname);

	string aistr;
	char* r_str;
	aistr = NS_Shiori.GetAIStringRandom();

	r_str = (char*)malloc(aistr.size() + 1);
	strcpy(r_str,aistr.c_str());
	return r_str;
}
