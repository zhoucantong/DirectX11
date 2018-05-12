#include "stdafx.h"
#include "MyObject.h"


MyObject::MyObject()
{
}


MyObject::~MyObject()
{
}

map<string, MyObject>::iterator ObjectManager::begin()
{
	return Objects.begin();
}

map<string, MyObject>::iterator ObjectManager::end()
{
	return Objects.end();
}
string ObjectManager::ConvertKey(UINT nKey)
{
	char cKey[KEYSIZE] = { 0 };
	sprintf_s(cKey, "%08X", nKey);
	string strKey(cKey);
	return strKey;
}
void ObjectManager::push_back(const string& ObjName, const MyObject& Object)
{
	string strKey = ConvertKey(ObjectNames.size());
	Objects.insert(map<string, MyObject>::value_type(strKey, Object));
	ObjectNames.push_back(ObjName);
}

map<string, MyObject>::iterator ObjectManager::FindObject(const string& ObjName)
{
	UINT uKey = ObjectNames.size();
	for (size_t nIndex =0;nIndex< ObjectNames.size();++nIndex)
	{
		if (ObjName == ObjectNames[nIndex])
		{
			uKey = nIndex;
			break;
		}
	}
	string strKey = ConvertKey(uKey);
	return Objects.find(strKey);
}