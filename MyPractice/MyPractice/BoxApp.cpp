#include "stdafx.h"
#include "BoxApp.h"
#include "DirectXMath.h"
#include "D3Dcompiler.h"
#include "MyObject.h"



BoxApp::BoxApp(HINSTANCE hInstance)
	: D3DApp(hInstance), mVB(0), mIB(0), mFX(0), mTech(0),
	mfxWorldViewProj(0), mInputLayout(0),
	mTheta(1.5f*MathHelper::Pi), mPhi(0.25f*MathHelper::Pi), mRadius(20.0f)
{
	mMainWndCaption = L"Box Demo";
	mLastMousePos.x = 0;
	mLastMousePos.y = 0;
	XMMATRIX I = XMMatrixIdentity();
	/*XMStoreFloat4x4(&mWorld, I);
	XMStoreFloat4x4(&mView, I);
	XMStoreFloat4x4(&mProj, I);*/

//	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&mGridWorld, I);
	ObjectTransform gridWorld(string("grid"), mGridWorld);
	DrawList.push_back(gridWorld);
	XMMATRIX boxScale = XMMatrixScaling(2.0f, 1.0f, 2.0f);
	XMMATRIX boxOffset = XMMatrixTranslation(0.0f, 0.5f, 0.0f);
	XMStoreFloat4x4(&mBoxWorld, XMMatrixMultiply(boxScale, boxOffset));
	ObjectTransform BoxWorld(string("box"), mBoxWorld);
	DrawList.push_back(BoxWorld);
	XMMATRIX centerSphereScale = XMMatrixScaling(2.0f, 2.0f, 2.0f);
	XMMATRIX centerSphereOffset = XMMatrixTranslation(0.0f, 2.0f, 0.0f);
	XMStoreFloat4x4(&mCenterSphere, XMMatrixMultiply(centerSphereScale, centerSphereOffset));
	ObjectTransform sphereWorld(string("sphere"), mCenterSphere);
	DrawList.push_back(sphereWorld);
	// We create 5 rows of 2 cylinders and spheres per row.
	for (int i = 0; i < 5; ++i)
	{
		XMStoreFloat4x4(&mCylWorld[i * 2 + 0],
			XMMatrixTranslation(-5.0f, 1.5f, -10.0f + i * 5.0f));
		XMStoreFloat4x4(&mCylWorld[i * 2 + 1],
			XMMatrixTranslation(+5.0f, 1.5f, -10.0f + i * 5.0f));
		XMStoreFloat4x4(&mSphereWorld[i * 2 + 0],
			XMMatrixTranslation(-5.0f, 3.5f, -10.0f + i * 5.0f));
		XMStoreFloat4x4(&mSphereWorld[i * 2 + 1],
			XMMatrixTranslation(+5.0f, 3.5f, -10.0f + i * 5.0f));
		ObjectTransform World1(string("cylinder"), mCylWorld[i * 2 + 0]);
		DrawList.push_back(World1);
		ObjectTransform World2(string("cylinder"), mCylWorld[i * 2 + 1]);
		DrawList.push_back(World2);
		ObjectTransform World3(string("sphere"), mSphereWorld[i * 2 + 0]);
		DrawList.push_back(World3);
		ObjectTransform World4(string("sphere"), mSphereWorld[i * 2 + 1]);
		DrawList.push_back(World4);
	}
	CurrentIndexOffset = 0;
	CurrentVertexOffset = 0;
}
BoxApp::~BoxApp()
{
	ReleaseCOM(mVB);
	ReleaseCOM(mIB);
	ReleaseCOM(mFX);
	ReleaseCOM(mInputLayout);
}
bool BoxApp::Init()
{
	if (!D3DApp::Init())
		return false;
	BuildGeometryBuffers();
	BuildFX();
	BuildVertexLayout();
	return true;
}
void BoxApp::OnResize()
{
	D3DApp::OnResize();
	// The window resized, so update the aspect ratio and recomputed
	// the projection matrix.
	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f*MathHelper::Pi,
		AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&mProj, P);
}
void BoxApp::UpdateScene(float dt)
{
	// Convert Spherical to Cartesian coordinates.
	float x = mRadius * sinf(mPhi)*cosf(mTheta);
	float z = mRadius * sinf(mPhi)*sinf(mTheta);
	float y = mRadius * cosf(mPhi);
	// Build the view matrix.
	XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&mView, V);
}
void BoxApp::DrawScene()
{
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView,
		reinterpret_cast<const float*>(&Colors::Blue));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView,
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	md3dImmediateContext->IASetInputLayout(mInputLayout);
	md3dImmediateContext->IASetPrimitiveTopology(
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	md3dImmediateContext->RSSetState(mWireframeRS);
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	md3dImmediateContext->IASetVertexBuffers(0, 1, &mVB, &stride, &offset);
	md3dImmediateContext->IASetIndexBuffer(mIB, DXGI_FORMAT_R32_UINT, 0);
	// Set constants
	XMMATRIX view = XMLoadFloat4x4(&mView);
	XMMATRIX proj = XMLoadFloat4x4(&mProj);
	XMMATRIX viewProj = view * proj;
	D3DX11_TECHNIQUE_DESC techDesc;
	mTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		// Draw the grid.
		XMMATRIX world = XMLoadFloat4x4(&mGridWorld);
		mfxWorldViewProj->SetMatrix(
			reinterpret_cast<float*>(&(world*viewProj)));
		mTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		auto FindIter = DrawObjects.find(string("grid"));
		if (FindIter != DrawObjects.end())
		{
			md3dImmediateContext->DrawIndexed(
				FindIter->second.IndexCount, FindIter->second.mIndexOffset, FindIter->second.mVertexOffset);
		}
		
		/*for (auto iter  = DrawList.begin();iter!= DrawList.end();++iter)
		{
			XMMATRIX world = XMLoadFloat4x4(&iter->WorldTransform);
			mfxWorldViewProj->SetMatrix(
				reinterpret_cast<float*>(&(world*viewProj)));
			mTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
			auto FindIter = DrawObjects.find(iter->GeometryName);
			if (FindIter != DrawObjects.end())
			{
				md3dImmediateContext->DrawIndexed(
					FindIter->second.IndexCount, FindIter->second.mIndexOffset, FindIter->second.mVertexOffset);
			}
			break;
		}
*/
		//// Draw the box.
		//world = XMLoadFloat4x4(&mBoxWorld);
		//mfxWorldViewProj->SetMatrix(
		//	reinterpret_cast<float*>(&(world*viewProj)));
		//mTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);

		//FindIter = DrawObjects.find(string("box"));
		//if (FindIter != DrawObjects.end())
		//{
		//	md3dImmediateContext->DrawIndexed(
		//		FindIter->second.IndexCount, FindIter->second.mIndexOffset, FindIter->second.mVertexOffset);
		//}
		//
		//// Draw center sphere.
		//world = XMLoadFloat4x4(&mCenterSphere);
		//mfxWorldViewProj->SetMatrix(
		//	reinterpret_cast<float*>(&(world*viewProj)));
		//mTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		//md3dImmediateContext->DrawIndexed(
		//	mSphereIndexCount, mSphereIndexOffset, mSphereVertexOffset);
		//// Draw the cylinders.
		//for (int i = 0; i < 10; ++i)
		//{
		//	world = XMLoadFloat4x4(&mCylWorld[i]);
		//	mfxWorldViewProj->SetMatrix(
		//		reinterpret_cast<float*>(&(world*viewProj)));
		//	mTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		//	md3dImmediateContext->DrawIndexed(mCylinderIndexCount,
		//		mCylinderIndexOffset, mCylinderVertexOffset);
		//}
		//// Draw the spheres.
		//for (int i = 0; i < 10; ++i)
		//{
		//	world = XMLoadFloat4x4(&mSphereWorld[i]);
		//	mfxWorldViewProj->SetMatrix(
		//		reinterpret_cast<float*>(&(world*viewProj)));
		//	mTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		//	md3dImmediateContext->DrawIndexed(mSphereIndexCount,
		//		mSphereIndexOffset, mSphereVertexOffset);
		//}
	}
	HR(mSwapChain->Present(0, 0));
}
void BoxApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;
	SetCapture(mhMainWnd);
}
void BoxApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}
void BoxApp::OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = XMConvertToRadians(
			0.25f*static_cast<float>(x - mLastMousePos.x));
		float dy = XMConvertToRadians(
			0.25f*static_cast<float>(y - mLastMousePos.y));
		// Update angles based on input to orbit camera around box.
		mTheta += dx;
		mPhi += dy;
		// Restrict the angle mPhi.
		mPhi = MathHelper::Clamp(mPhi, 0.1f, MathHelper::Pi - 0.1f);
	}
	else if ((btnState & MK_RBUTTON) != 0)
	{
		// Make each pixel correspond to 0.005 unit in the scene.
		float dx = 0.005f*static_cast<float>(x - mLastMousePos.x);
		float dy = 0.005f*static_cast<float>(y - mLastMousePos.y);
		// Update the camera radius based on input.
		mRadius += dx - dy;
		// Restrict the radius.
		mRadius = MathHelper::Clamp(mRadius, 3.0f, 15.0f);
	}
	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

void BoxApp::BuildFX()
{
	DWORD shaderFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
	shaderFlags |= D3D10_SHADER_DEBUG;
	shaderFlags |= D3D10_SHADER_SKIP_OPTIMIZATION;
#endif
	ID3DBlob* compiledShader = 0;
	ID3DBlob* compilationMsgs = 0;
	HRESULT hr = D3DCompileFromFile(L"FX/color.fx", NULL, NULL, NULL, "fx_5_0", shaderFlags, 0, &compiledShader, &compilationMsgs);
	// D3DCompileFromFile(vsFilename,NULL,NULL,"TextureVertexShader","vs_5_0",D3D10_SHADER_ENABLE_STRICTNESS | D3D10_SHADER_DEBUG,0,&vertexShaderBuffer,&errorMessage);0, 0, &compiledShader, &compilationMsgs);
	// compilationMsgs can store errors or warnings.
	if (compilationMsgs != 0)
	{
		// MessageBoxA(0, (char*)compilationMsgs->GetBufferPointer(), 0, 0);
		ReleaseCOM(compilationMsgs);
	}
	// Even if there are no compilationMsgs, check to make sure there
	// were no other errors.
	if (FAILED(hr))
	{
		//DXTrace(__FILE__, (DWORD)__LINE__, hr,L"D3DX11CompileFromFile", true);
	}
	HR(D3DX11CreateEffectFromMemory(
		compiledShader->GetBufferPointer(),
		compiledShader->GetBufferSize(),
		0, md3dDevice, &mFX));
	// Done with compiled shader.
	ReleaseCOM(compiledShader);
	mTech = mFX->GetTechniqueByName("ColorTech");
	mfxWorldViewProj = mFX->GetVariableByName(
		"gWorldViewProj")->AsMatrix();
}
void BoxApp::BuildVertexLayout()
{
	// Create the vertex input layout.
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
		D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0,
	D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	// Create the input layout
	D3DX11_PASS_DESC passDesc;
	mTech->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(md3dDevice->CreateInputLayout(vertexDesc, 2,
		passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize, &mInputLayout));
}

void BoxApp::BuildGeometryBuffers()
{
	GeometryGenerator::MeshData box;
	GeometryGenerator::MeshData grid;
	GeometryGenerator::MeshData sphere;
	GeometryGenerator::MeshData cylinder;

	GeometryGenerator geoGen;
	geoGen.CreateBox(1.0f, 1.0f, 1.0f, box);
	geoGen.CreateGrid(20.0f, 30.0f, 60, 40, grid);
	geoGen.CreateSphere(0.5f, 20, sphere);
	geoGen.CreateCylinder(0.5f, 0.3f, 3.0f, 20, 20, cylinder);
	AddGeometry(("box"),box);
	AddGeometry(("grid"),grid);
	AddGeometry(("sphere"), sphere);
	AddGeometry(("cylinder"), cylinder);
	UINT totalVertexCount = 0;
	UINT totalIndexCount = 0;
	for (auto iter =  DrawObjects.begin();iter != DrawObjects.end(); ++iter)
	{
		totalVertexCount += iter->second.Data.Vertices.size();
		totalIndexCount += iter->second.IndexCount;
	}
	//
	// Extract the vertex elements we are interested in and pack the
	// vertices of all the meshes into one vertex buffer.
	//
	std::vector<Vertex> vertices(totalVertexCount);
	XMFLOAT4 black(0.0f, 0.0f, 0.0f, 1.0f);
	UINT k = 0;
	for (auto iter = DrawObjects.begin(); iter != DrawObjects.end(); ++iter)
	{
		for (auto VerticesIter = iter->second.Data.Vertices.begin(); VerticesIter != iter->second.Data.Vertices.end(); ++VerticesIter,++k)
		{
			vertices[k].Pos = VerticesIter->Position;
			vertices[k].Color = black;
		}
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex) * totalVertexCount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mVB));
	//
	// Pack the indices of all the meshes into one index buffer.
	//
	std::vector<UINT> indices;
	for (auto iter = DrawObjects.begin(); iter != DrawObjects.end(); ++iter)
	{
		indices.insert(indices.end(), iter->second.Data.Indices.begin(), iter->second.Data.Indices.end());
	}

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * totalIndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
	HR(md3dDevice->CreateBuffer(&ibd, &iinitData, &mIB));
}

void BoxApp::AddGeometry(const string& MeshName,const GeometryGenerator::MeshData& Mesh)
{
	DrawObjects;
	MyObject NewObject;
	NewObject.mIndexOffset = CurrentIndexOffset;
	NewObject.mVertexOffset = CurrentVertexOffset;
	NewObject.IndexCount = Mesh.Indices.size();
	NewObject.Data = Mesh;
	CurrentIndexOffset += Mesh.Indices.size();
	CurrentVertexOffset += Mesh.Vertices.size();
	DrawObjects.insert(map<string, MyObject>::value_type( MeshName,NewObject));
}
float BoxApp::GetHeight(float x, float z)const
{
	return 0.3f*(z*sinf(0.1f*x) + x * cosf(0.1f*z));
}