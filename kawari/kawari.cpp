//---------------------------------------------------------------------------
//
// "������" for ����ʳ��β����ʳ��β���
// ����shiori.dll
//
//      Programed by NAKAUE.T (Meister)
//
//  2001.02.03  Phase 0.3     �Ρ�������
//  2001.02.11  Phase 0.31    �Ƶ��������
//  2001.02.23  Phase 0.4     ´�����轪�ﵧǰ
//                            ���¤餫�ˤ�̲�겼����������Ϸ����֤��ޤ����
//                                                                �С������
//                            kawari.iniƳ��
//                            ʣ������ե�����
//                            sentence.txt�ѻ�
//  2001.02.27  Phase 0.41    �����������ʤ��ä��Х����(getword��Ϣ)
//  2001.03.15  Phase 0.42    unloadͽ��
//                            ���󥰵�ǽ
//                            �Ź沽�ե������б�
//                            ��������ȥ��б�
//                            �������ѿ��б�
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
// 0����num-1�ޤǤ������ȯ��
inline int Random(int num)
{
	return((int)((double(rand())/RAND_MAX)*num));
}
//---------------------------------------------------------------------------
// ʸ���������ζ���������
string StringTrim(string str)
{
	string::size_type linetop=str.find_first_not_of(" \t\r\n");
	string::size_type lineend=str.find_last_not_of(" \t\r\n");
	if(linetop==string::npos) return(string(""));

	return(str.substr(linetop,lineend-linetop+1));
}
//---------------------------------------------------------------------------
// 2�Х���ʸ����1�Х����ܤ�?
inline bool iskanji1st(char c)
{
#if 1
	// SJIS
	// 0x00-0x7f ASCII
	// 0x80-0x9f,0xe0-0xfc ����������1�Х�����
	// 0xa0-0xdf ������Ⱦ�ѥ���
//	if((0<=c)||((-96<=c)&&(c<=-33))) return(false);
//	return(true);
	return((unsigned char)((c^0x20)-0xa1)<=0x3b);
	// ���ʤߤ�2�Х����ܤ�0x40-0xfc
#else
	// EUC
	return(c<0);
#endif
}
//---------------------------------------------------------------------------
// ${}�ǰϤޤ줿�������ɻ����ǥ����ɤ���
// �ǥ����ɷ�̤˥������ɻ��꤬���äƤ�����ˤϺƵ�����
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
// ������ʸ���֤�
string TNS_KawariANI::GetAIStringRandom(void)
{
	if(LogFS) (*LogFS) << "GetAIStringRandom:" << endl;

	string key("sentence");

	string aistr;

	if(WordDictionary.count(key)>0) {
		string &orgsen=*(WordDictionary[key][Random(WordDictionary[key].size())]);
		aistr=AIStringDecode(orgsen);
	} else {
		aistr="sentence������ޤ���";
	}

	if(LogFS) (*LogFS) << "GetAIStringRandom(output):" << aistr << endl;

	return(aistr);
}
//---------------------------------------------------------------------------
// ���̤˰��פ��������ñ����֤�
string TNS_KawariANI::GetWord(string wordtype)
{
	// wordtype�ˤϡ�\ms�פΤ褦��ʸ�����Ϥ����

	if(LogFS) (*LogFS) << "GetWord:" << wordtype << endl;

	string key("compatible-");
	key+=wordtype.substr(1);

	string aistr;

	if(WordDictionary.count(key)>0) {
		string &orgsen=*(WordDictionary[key][Random(WordDictionary[key].size())]);
		aistr=AIStringDecode(orgsen);
	} else {
		aistr=key+"�����Ĥ���ޤ���";
	}

	if(LogFS) (*LogFS) << "GetWord(output):" << aistr << endl;

	return(aistr);
}
//---------------------------------------------------------------------------
// ���̡�dms�פ˰��פ��������ñ����֤�
string TNS_KawariANI::GetDMS(void)
{
	if(LogFS) (*LogFS) << "GetDMS:" << endl;

	return(GetWord("\\dms"));
}
//---------------------------------------------------------------------------
// GetMatchWord���֤���ñ��˴�Ť���������ʸ���֤�
string TNS_KawariANI::GetAIStringFromTargetWord(string targetword)
{
	if(LogFS) (*LogFS) << "GetAIStringFromTargetWord:" << targetword << endl;

//	string aistr=targetword+"�äơ���ͤ����";
	string aistr=GetAIStringRandom();	// ��������

	if(LogFS) (*LogFS) << "GetAIStringFromTargetWord(output):" << aistr << endl;

	return(aistr);
}
//---------------------------------------------------------------------------
// ���GetAIStringRandom���֤���ʸ���顢��������ñ����ܤ��Ф�
string TNS_KawariANI::GetMatchWord(string sentence)
{
	if(LogFS) (*LogFS) << "GetMatchWord:" << sentence << endl;

	string aistr="�ۤ��ۤ�\1\\ms\r\n";

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
	// GetAIState�ε�ư��������ǽ�ʤΤǻȤ��Υ��
	// GetMatchWord�ȴ�Ϣ���Ƥ���Τ�?
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
	// ���Υ����ߥ󥰤������������ޤ�
	srand((unsigned int)time(NULL));

	// �ɤ߹���٤�����ե��������
	vector<string> dictfiles;

	// ini�ե�������ɤ߹���
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
		// �ɤ߹���ʤ��ä��ݤ�ջ�ɽ��
		// ���ߤν�桼�����ؤ�����ˡ�Ϥ��줷���ʤ�
		WordDictionary[string("sentence")].push_back(new string("kawari.ini���ɤ�ޤ���"));
	}


	// ����ե������ɤ߹���
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
	WordDictionary["friendname"].push_back(new string(_friendname));

	return(true);
}
//---------------------------------------------------------------------------
string TNS_KawariANI::GetResponse(string sentence)
{
	if(LogFS) (*LogFS) << "GetResponse:" << sentence << endl;

	string aistr="�����ʤ���������������(GetResponse)�ʤ�Ƹ����Ф������狼��ʤ��Ρ�";

	return(aistr);
}
//---------------------------------------------------------------------------
string TNS_KawariANI::GetImpression(string sentence)
{
	if(LogFS) (*LogFS) << "GetImpression:" << sentence << endl;

	string aistr="�����ʤ���������������(GetImpression)�ʤ�Ƹ����Ф������狼��ʤ��Ρ�";

	return(aistr);
}
//---------------------------------------------------------------------------
// �ʲ��Υ���ȥ�����̰��������
//
// myname : ��ʬ��̾��
// unyuname : ���ˤ夦��̾��
// friendname : ͧ�ͤ�̾��
//
// �ʲ���getword,getdms��
// compatible-ms  : ̾��-��
// compatible-mz  : ̾��-̵��ʪ
// compatible-mc  : ̾��-��̾
// compatible-mh  : ̾��-Ź̾
// compatible-mt  : ̾��-��
// compatible-me  : ̾��-��ʪ
// compatible-mp  : ̾��-��̾�ߤ����ʤ��
// compatible-m   : ̾��-�����
// compatible-dms : �֡��ˡ��������Ū�ʡ��ʻ줬ʣ��Ϣ�뤵�줿Ĺ���̾��
//
