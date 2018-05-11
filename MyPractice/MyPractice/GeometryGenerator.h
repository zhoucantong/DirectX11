#pragma once
class GeometryGenerator
{
public:
	GeometryGenerator();
	~GeometryGenerator();
public:
	struct Vertex
	{
		Vertex() {}
		Vertex(const XMFLOAT3& p,
			const XMFLOAT3& n,
			const XMFLOAT3& t,
			const XMFLOAT2& uv)
			: Position(p), Normal(n), TangentU(t), TexC(uv) {}
		Vertex(
			float px, float py, float pz,
			float nx, float ny, float nz,
			float tx, float ty, float tz,
			float u, float v)
			: Position(px, py, pz), Normal(nx, ny, nz),
			TangentU(tx, ty, tz), TexC(u, v) {}
		XMFLOAT3 Position;
		XMFLOAT3 Normal;
		XMFLOAT3 TangentU;
		XMFLOAT2 TexC;
	};
	struct MeshData
	{
		std::vector<Vertex> Vertices;
		std::vector<UINT> Indices;
	};

public:

	void CreateBox(float width, float height, float depth, MeshData& meshData);
	void CreateGrid(float width, float depth,
		UINT m, UINT n, MeshData& meshData);
	void CreateCylinder(float bottomRadius, float topRadius, float height, UINT sliceCount, UINT
		stackCount, MeshData& meshData);
	void BuildCylinderBottomCap(float bottomRadius,
		float topRadius, float height, UINT sliceCount,
		UINT stackCount, MeshData& meshData);
	void BuildCylinderTopCap(float bottomRadius,
		float topRadius, float height, UINT sliceCount,
		UINT stackCount, MeshData& meshData);
	void CreateSphere(float radius, UINT numSubdivisions, MeshData& meshData);

private:
	void Subdivide(MeshData& meshData);
};

