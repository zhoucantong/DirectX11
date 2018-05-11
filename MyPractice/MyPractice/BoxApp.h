#pragma once
#include "D3DApp.h"
#include <vector>
#include <map>
#include <list>
#include "GeometryGenerator.h"
using namespace std;
// #include "effects.h"
// #include "DirectXMath.h"
class MyObject;
class ObjectTransform;
class GeometryGenerator;
struct MeshData;
struct Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT4 Color;
};


class BoxApp :
	public D3DApp
{
public:
	BoxApp(HINSTANCE hInstance);
	~BoxApp();
	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();
	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);
private:
	void BuildGeometryBuffers();
	void AddGeometry(const string& MeshName,const GeometryGenerator::MeshData& Mesh);
	void BuildFX();
	void BuildVertexLayout();
	float GetHeight(float x, float z)const;
private:
	ID3D11Buffer * mVB;
	ID3D11Buffer* mIB;
	ID3DX11Effect* mFX;
	ID3DX11EffectTechnique* mTech;
	ID3DX11EffectMatrixVariable* mfxWorldViewProj;
	ID3D11InputLayout* mInputLayout;

	XMFLOAT4X4 mWorld;
	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;
	float mTheta;
	float mPhi;
	float mRadius;
	POINT mLastMousePos;

	//size_t mGridIndexCount;

private:
	// Define transformations from local spaces to world space.
	XMFLOAT4X4 mSphereWorld[10];
	XMFLOAT4X4 mCylWorld[10];
	XMFLOAT4X4 mBoxWorld;
	XMFLOAT4X4 mGridWorld;
	XMFLOAT4X4 mCenterSphere;

	map<string,MyObject> DrawObjects;// 物体对象
	list<ObjectTransform> DrawList;		// 需要绘制的几何体列表
	size_t CurrentIndexOffset;
	size_t CurrentVertexOffset;
};

