//---------------------------------------------------------------------------
//
// "������" for ����ʳ��β����ʳ��β���
// ��AI������
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
//  2001.03.15  Phase 0.42    unloadͽ��
//
//---------------------------------------------------------------------------
#ifndef KAWARI_H
#define KAWARI_H
//---------------------------------------------------------------------------
#include <string>
#include <vector>
#include <map>
#include <fstream>
using namespace std;
//---------------------------------------------------------------------------
class TNS_KawariANI {
private:

	string MyName;
	string UnyuName;
	string FriendName;

	// ����
	map<string,vector<string*> > WordDictionary;

	// ���ե����������
	ofstream *LogFS;

	// ${}�ǰϤޤ줿�������ɻ����ǥ����ɤ���
	// �ǥ����ɷ�̤˥������ɻ��꤬���äƤ�����ˤϺƵ�����
	string AIStringDecode(string &orgsen,int level=30);

public:

	TNS_KawariANI(void) : LogFS(NULL) {}

	~TNS_KawariANI()
	{
		if(LogFS) {
			(*LogFS) << "delete object." << endl;
			LogFS->close();
			delete LogFS;
		}
	}

	string GetAIStringRandom(void);
	string GetAIStringFromTargetWord(string targetword);
	string GetDMS(void);
	string GetWord(string wordtype);
	string GetMatchWord(string sentence);
	void GetAIState(int state[6]);
	bool Load(string datapath);
	bool Unload(void);
	bool SetUpNameTable(string& _sakuraname,string& _keroname,string& _friendname);

	string GetResponse(string sentence);
	string GetImpression(string sentence);

};
//---------------------------------------------------------------------------
#endif
