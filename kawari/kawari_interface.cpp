#include <string>
#include <iostream>
#include <cstdlib>
using namespace std;
#include <unistd.h>		// sleep
#include "kawari.h"
//#include "nisesakura_sakura.h"	// sakuraスクリプト

extern "C" char* RandomMessage(char* kawari_dir)
{
	static string sakuraname="sakura";
	static string keroname="unyuu";
	static string friendname="sirone";
	static string datapath="";

	static TNS_KawariANI NS_Shiori;
	static int virgine = 1;

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

	if(virgine){
	  if(!NS_Shiori.Load(datapath)) {
	    return NULL;
	  }
	  virgine = 0;


	  NS_Shiori.SetUpNameTable(sakuraname,keroname,friendname);

	}

	string aistr;
	char* r_str;
	aistr = NS_Shiori.GetAIStringRandom();

	r_str = (char*)malloc(aistr.size() + 1);
	strcpy(r_str,aistr.c_str());
	return r_str;
}

extern "C" char* DecodeEscapeMessage(const char* kawari_dir, const char* word){
  static string sakuraname="sakura";
  static string keroname="unyuu";
  static string friendname="sirone";
  static string datapath="";
  
  static TNS_KawariANI NS_Shiori;
  static int virgine = 1;

  string wordtype = word;

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
  
  if(virgine){
    if(!NS_Shiori.Load(datapath)) {
      return NULL;
    }
    virgine = 0;

    NS_Shiori.SetUpNameTable(sakuraname,keroname,friendname);

  }

  string aistr;
  char* r_str;
  aistr = NS_Shiori.GetWord(wordtype);

  r_str = (char*)malloc(aistr.size() + 1);
  strcpy(r_str,aistr.c_str());
  return r_str;
}

