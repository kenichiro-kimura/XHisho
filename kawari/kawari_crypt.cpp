//---------------------------------------------------------------------------
//
// "������" for ����ʳ��β����ʳ��β���
// �Ź沽/���沽
//
//      Programed by NAKAUE.T (Meister)
//
//  2001.03.15  Phase 0.42    �Ź沽����
//
//---------------------------------------------------------------------------
#include "kawari_crypt.h"
#include "base64.h"
//---------------------------------------------------------------------------
// �Ź沽�������ǧ����
bool CheckCrypt(const string& encodedstr)
{
	string id=encodedstr.substr(0,9);
	return(id=="!KAWA0000");
}
//---------------------------------------------------------------------------
// ɸ��Ź沽
string EncryptString(const string& str)
{
	string encodedstr;

	for(unsigned int i=0;i<str.size();i++) {
		encodedstr+=str[i]^0xcc;
	}

	return(string("!KAWA0000")+EncodeBase64(encodedstr));
}
//---------------------------------------------------------------------------
// ɸ�����沽
string DecryptString(const string& encodedstr)
{
	string str=DecodeBase64(encodedstr.substr(9));

	string decodedstr;
	for(unsigned int i=0;i<str.size();i++) {
		decodedstr+=str[i]^0xcc;
	}

	return(decodedstr);
}
//---------------------------------------------------------------------------
