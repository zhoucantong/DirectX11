#pragma once
#include "GeometryGenerator.h"
#include <string>
#include <vector>
#include <map>
using namespace std;

class MyObject
{
public:
	MyObject();
	virtual ~MyObject();

public:
	XMMATRIX Scale;
	XMMATRIX Offset;
	GeometryGenerator::MeshData Data;

	size_t mIndexOffset;
	size_t mVertexOffset;
	size_t IndexCount;
};

class ObjectTransform
{
public:
	std::string GeometryName;
	XMFLOAT4X4 WorldTransform;
public:
	ObjectTransform(const std::string& Name, const XMFLOAT4X4& transform)
	{
		GeometryName = Name;
		WorldTransform = transform;
	}
};

#define KEYSIZE (64)
class ObjectManager
{
public:
	map<string, MyObject>::iterator begin();
	map<string, MyObject>::iterator end();
	void push_back(const string& ObjName, const MyObject& Object);
	map<string, MyObject>::iterator FindObject(const string& ObjName);
private:
	string ConvertKey(UINT nKey);
	map<string, MyObject> Objects;
	vector<string> ObjectNames;
};