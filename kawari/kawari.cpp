//---------------------------------------------------------------------------
//
// "�ؘa��" for ����ȊO�̉����ȊO�̉���
// �b��shiori.dll
//
//      Programed by NAKAUE.T (Meister)
//
//  2001.02.03  Phase 0.3     �m�[�R�����g
//  2001.02.11  Phase 0.31    �ċA��`����
//  2001.02.23  Phase 0.4     ���_�푈�I��F�O
//                            �`���炩�ɂ����艺�����A�߂��͌J��Ԃ��܂���`
//                                                                �o�[�W����
//                            kawari.ini����
//                            ���������t�@�C��
//                            sentence.txt�p�~
//  2001.02.27  Phase 0.41    �T���S�����Ȃ������o�O���(getword�֘A)
//  2001.03.15  Phase 0.42    unload�\��
//                            ���M���O�@�\
//                            �Í����t�@�C���Ή�
//                            �����G���g���Ή�
//                            ���[�J���ϐ��Ή�
//
//---------------------------------------------------------------------------
#include <string>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
using namespace std;
//---------------------------------------------------------------------------
#include "kawari.h"
#include "kawari_crypt.h"
//---------------------------------------------------------------------------
// 0����num-1�܂ł̗����𔭐�
inline int Random(int num)
{
	return((int)((double(rand())/RAND_MAX)*num));
}
//---------------------------------------------------------------------------
// ������̑O��̋󔒂���菜��
string StringTrim(string str)
{
	string::size_type linetop=str.find_first_not_of(" \t\r\n");
	string::size_type lineend=str.find_last_not_of(" \t\r\n");
	if(linetop==string::npos) return(string(""));

	return(str.substr(linetop,lineend-linetop+1));
}
//---------------------------------------------------------------------------
// 2�o�C�g������1�o�C�g�ڂ�?
inline bool iskanji1st(char c)
{
#if 1
	// SJIS
	// 0x00-0x7f ASCII
	// 0x80-0x9f,0xe0-0xfc ������S�p1�o�C�g��
	// 0xa0-0xdf �����锼�p�J�i
//	if((0<=c)||((-96<=c)&&(c<=-33))) return(false);
//	return(true);
	return((unsigned char)((c^0x20)-0xa1)<=0x3b);
	// ���Ȃ݂�2�o�C�g�ڂ�0x40-0xfc
#else
	// EUC
	return(c<0);
#endif
}
//---------------------------------------------------------------------------
// ${}�ň͂܂ꂽ�����_�����[�h�w����f�R�[�h����
// �f�R�[�h���ʂɃ����_�����[�h�w�肪�����Ă���ꍇ�ɂ͍ċA����
string TNS_KawariANI::AIStringDecode(string &orgsen,int level)
{
	if(level<=0) {
		if(LogFS) (*LogFS) << "AIStringDecode:Stack Over Flow" << endl;
		return(orgsen);
	}
	string retstr;
	vector<string> localdict;

	unsigned int bp=0;
	while(bp<orgsen.size()) {
		if(orgsen[bp]=='$') {
			bp++;
			if(bp>=orgsen.size()) break;

			if(orgsen[bp]=='{') bp++;

			string key;
			while((bp<orgsen.size())&&(orgsen[bp]!='}')) {
				if(iskanji1st(orgsen[bp])) {
					if((bp+1)<orgsen.size()) {
						key+=orgsen[bp++];
						key+=orgsen[bp++];
					}
				} else {
					key+=orgsen[bp++];
				}
			}
			if(bp<orgsen.size()) bp++;
			if(key.size()>0) {
				if(!isdigit(key[0])) {
					if(WordDictionary.count(key)>0) {
						string subsen=*(WordDictionary[key][Random(WordDictionary[key].size())]);
						localdict.push_back(AIStringDecode(subsen,level-1));
						retstr+=localdict.back();
					} else {
						retstr+=key;
					}
				} else {
					int dictnum=atoi(key.c_str());
					if((0<=dictnum)&&(dictnum<(int)localdict.size())) retstr+=localdict[dictnum];
				}
			}
		} else if(iskanji1st(orgsen[bp])) {
			retstr+=orgsen[bp++];
			retstr+=orgsen[bp++];
		} else {
			retstr+=orgsen[bp++];
		}
	}

	return(retstr);
}
//---------------------------------------------------------------------------
// �����_������Ԃ�
string TNS_KawariANI::GetAIStringRandom(void)
{
	if(LogFS) (*LogFS) << "GetAIStringRandom:" << endl;

	string key("sentence");

	string aistr;

	if(WordDictionary.count(key)>0) {
		string &orgsen=*(WordDictionary[key][Random(WordDictionary[key].size())]);
		aistr=AIStringDecode(orgsen);
	} else {
		aistr="sentence������܂���";
	}

	if(LogFS) (*LogFS) << "GetAIStringRandom(output):" << aistr << endl;

	return(aistr);
}
//---------------------------------------------------------------------------
// ��ʂɈ�v���郉���_���P���Ԃ�
string TNS_KawariANI::GetWord(string wordtype)
{
	// wordtype�ɂ́u\ms�v�̂悤�ȕ����񂪓n�����

	if(LogFS) (*LogFS) << "GetWord:" << wordtype << endl;

	string key("compatible-");
	key+=wordtype.substr(1);

	string aistr;

	if(WordDictionary.count(key)>0) {
		string &orgsen=*(WordDictionary[key][Random(WordDictionary[key].size())]);
		aistr=AIStringDecode(orgsen);
	} else {
		aistr=key+"��������܂���";
	}

	if(LogFS) (*LogFS) << "GetWord(output):" << aistr << endl;

	return(aistr);
}
//---------------------------------------------------------------------------
// ��ʁudms�v�Ɉ�v���郉���_���P���Ԃ�
string TNS_KawariANI::GetDMS(void)
{
	if(LogFS) (*LogFS) << "GetDMS:" << endl;

	return(GetWord("\\dms"));
}
//---------------------------------------------------------------------------
// GetMatchWord�ŕԂ����P��Ɋ�Â��A�����_������Ԃ�
string TNS_KawariANI::GetAIStringFromTargetWord(string targetword)
{
	if(LogFS) (*LogFS) << "GetAIStringFromTargetWord:" << targetword << endl;

//	string aistr=targetword+"���āA���l�����H";
	string aistr=GetAIStringRandom();	// �b��[�u

	if(LogFS) (*LogFS) << "GetAIStringFromTargetWord(output):" << aistr << endl;

	return(aistr);
}
//---------------------------------------------------------------------------
// ���GetAIStringRandom�ŕԂ���������A�Y������P���{���o��
string TNS_KawariANI::GetMatchWord(string sentence)
{
	if(LogFS) (*LogFS) << "GetMatchWord:" << sentence << endl;

	string aistr="�ق��ق�\1\\ms\r\n";

	return(aistr);
}
//---------------------------------------------------------------------------
void TNS_KawariANI::GetAIState(int state[6])
{
	if(LogFS) (*LogFS) << "GetAIState:" << endl;

#if 0
	state[0]=WordDictionary.size()*10;
	state[1]=1;
	state[2]=1;
	state[3]=1;
	state[4]=1;
	state[5]=1;
#else
	// GetAIState�̋���������s�\�Ȃ̂Ŏg���̃���
	// GetMatchWord�Ɗ֘A���Ă���̂�?
	state[0]=0;
	state[1]=0;
	state[2]=0;
	state[3]=0;
	state[4]=0;
	state[5]=0;
#endif
}
//---------------------------------------------------------------------------
bool TNS_KawariANI::Load(string datapath)
{
	// ���̃^�C�~���O�ŗ������������܂�
	srand((unsigned int)time(NULL));

	// �ǂݍ��ނׂ������t�@�C���ꗗ
	vector<string> dictfiles;

	// ini�t�@�C���̓ǂݍ���
	ifstream ifs;
	ifs.open((datapath+"kawari.ini").c_str());
	if(ifs.is_open()) {
		string buff;
		while(getline(ifs,buff)) {
			buff=StringTrim(buff);
			if((buff.size()>0)&&(buff[0]!='#')) {
				string key,value;
				string::size_type pos=buff.find(':');
				if(pos==string::npos) {
					key=buff;
					value="";
				} else {
					key=StringTrim(buff.substr(0,pos));
					value=StringTrim(buff.substr(pos+1));
				}

				if(key=="randomseed") {
					srand((unsigned int)atoi(key.c_str()));
				} else if(key=="dict") {
					dictfiles.push_back(value);
				} else if(key=="debug") {
					if(value=="log") {
						if(!LogFS) LogFS=new ofstream((datapath+"kawari.log").c_str());
					}
				}
			}
		}
		ifs.close();
	} else {
		// �ǂݍ��߂Ȃ������|���ӎu�\��
		// ���݂̏����[�U�[�ւ̒񎦕��@�͂��ꂵ���Ȃ�
		WordDictionary[string("sentence")].push_back(new string("kawari.ini���ǂ߂܂���B"));
	}


	// �����t�@�C���ǂݍ���
	for(unsigned int i=0;i<dictfiles.size();i++) {
		ifstream ifs;
		ifs.open((datapath+dictfiles[i]).c_str());

		if(ifs.is_open()) {
			if(LogFS) (*LogFS) << "Load:" << (datapath+dictfiles[i]) << endl;
			string buff;
			while(getline(ifs,buff)) {
				buff=StringTrim(buff);
				if((buff.size()>0)&&(buff[0]=='!')) {
					if(CheckCrypt(buff)) {
						buff=DecryptString(buff);
					} else {
						buff="";
					}
				}

				if((buff.size()>0)&&(buff[0]!='#')&&(buff[0]!=':')) {
					string key,value;
					string::size_type pos=buff.find(':');
					if(pos==string::npos) continue;

					key=StringTrim(buff.substr(0,pos));
					buff=buff.substr(pos+1);
					if(key.size()>0) {
						if(key[0]=='@') key=dictfiles[i]+key;

						while(buff.size()>0) {
							pos=buff.find(',');
							string word;
							if(pos!=string::npos) {
								word=StringTrim(buff.substr(0,pos));
								buff=buff.substr(pos+1);
							} else {
								word=StringTrim(buff);
								buff.erase();
							}

							if(word.size()>0) {
								string word2;
								for(unsigned int bp=0;bp<word.size();) {
									if((word[bp]=='$')&&((bp+3)<word.size())) {
										word2+=word[bp++];
										if(word[bp]=='{') bp++;
										word2+='{';

										while((bp<word.size())&&(word[bp]!='}')) {
											if(word[bp]!='@') {
												word2+=word[bp++];
											} else {
												word2+=dictfiles[i];
												word2+=word[bp++];
											}
										}
										bp++;
										word2+='}';
									} else {
										word2+=word[bp++];
									}
								}
								WordDictionary[key].push_back(new string(word2));
							}
						}
					}
				}
			}
			ifs.close();
		}
	}

	return(true);
}
//---------------------------------------------------------------------------
bool TNS_KawariANI::Unload(void)
{
	if(LogFS) (*LogFS) << "Unload:" << endl;

	return(true);
}
//---------------------------------------------------------------------------
bool TNS_KawariANI::SetUpNameTable(string& _sakuraname,string& _keroname,string& _friendname)
{
	if(LogFS) (*LogFS) << "SetUpNameTable:"
	 << _sakuraname << "," << _keroname << "," << _friendname << endl;

	WordDictionary["myname"].push_back(new string(_sakuraname));
	WordDictionary["unyuname"].push_back(new string(_keroname));
	WordDictionary["sakuraname"].push_back(new string(_sakuraname));
	WordDictionary["keroname"].push_back(new string(_keroname));
	WordDictionary["friendname"].push_back(new string(_friendname));

	return(true);
}
//---------------------------------------------------------------------------
string TNS_KawariANI::GetResponse(string sentence)
{
	if(LogFS) (*LogFS) << "GetResponse:" << sentence << endl;

	string aistr="���߂�Ȃ����A����������(GetResponse)�Ȃ�Č����΂������킩��Ȃ��́B";

	return(aistr);
}
//---------------------------------------------------------------------------
string TNS_KawariANI::GetImpression(string sentence)
{
	if(LogFS) (*LogFS) << "GetImpression:" << sentence << endl;

	string aistr="���߂�Ȃ����A����������(GetImpression)�Ȃ�Č����΂������킩��Ȃ��́B";

	return(aistr);
}
//---------------------------------------------------------------------------
// �ȉ��̃G���g���͓��ʈ��������
//
// myname : �����̖��O
// unyuname : ���ɂイ�̖��O
// friendname : �F�l�̖��O
//
// �ȉ���getword,getdms�p
// compatible-ms  : ����-�l
// compatible-mz  : ����-���@��
// compatible-mc  : ����-�Ж�
// compatible-mh  : ����-�X��
// compatible-mt  : ����-�Z
// compatible-me  : ����-�H��
// compatible-mp  : ����-�n���݂����Ȃ���
// compatible-m   : ����-�����
// compatible-dms : �u�`�Ɂ`����`�v�I�ȁA�i���������A�����ꂽ���߂̖���
//
