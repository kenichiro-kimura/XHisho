//---------------------------------------------------------------------------
//
// "華和梨" for あれ以外の何か以外の何か
// 偽AI代用品
//
//      Programed by NAKAUE.T (Meister)
//
//  2001.02.03  Phase 0.3     ノーコメント
//  2001.02.11  Phase 0.31    再帰定義実装
//  2001.02.23  Phase 0.4     卒論戦争終戦祈念
//                            〜安らかにお眠り下さい、過ちは繰り返しません〜
//                                                                バージョン
//                            kawari.ini導入
//                            複数辞書ファイル
//                            sentence.txt廃止
//  2001.03.15  Phase 0.42    unload予約
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

	// 辞書
	map<string,vector<string*> > WordDictionary;

	// ログファイル出力用
	ofstream *LogFS;

	// ${}で囲まれたランダムワード指定をデコードする
	// デコード結果にランダムワード指定が入っている場合には再帰処理
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
