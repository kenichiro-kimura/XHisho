#include <string>
#include <iostream>
#include <cstdlib>
using namespace std;
#include <unistd.h>		// sleep
#include "kawari.h"
//#include "nisesakura_sakura.h"	// sakuraスクリプト

extern "C" char* RandomMessage(char* kawari_dir)
{
	string sakuraname="sakura";
	string keroname="unyuu";
	string friendname="sirone";
	string datapath="";

	TNS_KawariANI NS_Shiori;

	if(kawari_dir == NULL || strlen(kawari_dir) < 1){
	  datapath = "./";
	} else {
	  if(*kawari_dir == '/' || *kawari_dir == '.'){
	    datapath = (string)kawari_dir;
	  } else {
	    datapath = (string)getenv("HOME") + "/" + (string)kawari_dir;
	  }
	}

	datapath += "/";
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
