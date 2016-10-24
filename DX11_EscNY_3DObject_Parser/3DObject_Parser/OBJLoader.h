#pragma once

#include "Includes.h"
#include "Utilities.h"

struct VPos
{
	float x, y, z;
};
struct VTex
{
	float u, v;
};
struct VNorm
{
	float x, y, z;
};

struct FaceType
{
	int vIndex1, vIndex2, vIndex3;
	int tIndex1, tIndex2, tIndex3;
	int nIndex1, nIndex2, nIndex3;
};

struct FaceTypeQuad
{
	int vIndex1, vIndex2, vIndex3, vIndex4;
	int tIndex1, tIndex2, tIndex3, tIndex4;
	int nIndex1, nIndex2, nIndex3, nIndex4;
};


class OBJLoader
{
	class ObjLoaderBase
	{
	public:
		ObjLoaderBase( OBJLoader &Parent, const std::wstring &Filename )
			:
			m_filename( Filename ),
			m_parent( Parent )
		{}
		virtual ~ObjLoaderBase()
		{
		}

		virtual bool LoadData() = 0;

	protected:
		std::wstring m_filename;
		OBJLoader &m_parent;
	};
	class ObjTriangleLoader :public ObjLoaderBase
	{
	public:
		ObjTriangleLoader( OBJLoader &Parent, const std::wstring &Filename )
			:
			ObjLoaderBase( Parent, Filename )
		{}
		
		bool LoadData() override;
	};
	class ObjQuadLoader :public ObjLoaderBase
	{
	public:
		ObjQuadLoader( OBJLoader &Parent, const std::wstring &Filename )
			:
			ObjLoaderBase( Parent, Filename )
		{}

		bool LoadData() override;
	};

public:
	OBJLoader();
	~OBJLoader();

	bool Load(
		HWND TextboxProgress, 
		const std::wstring &FilenameLoad, 
		const std::wstring &FilenameSave );

	bool Preprocess( const std::wstring &Filename );
private:
	bool WriteToTextFile(
		const std::wstring &Filename,
		const std::vector<XMFLOAT3>& Vertices, 
		const std::vector<XMFLOAT2>& TexCoords, 
		const std::vector<XMFLOAT3>& Normals,
		const std::vector<FaceType>& Faces);
	bool WriteVertToBinaryFile(
		const std::wstring &Filename,
		const std::vector<XMFLOAT3>& Vertices,
		const std::vector<FaceType>& Faces);
	bool WriteVertTexCoordToBinaryFile(
		const std::wstring &Filename,
		const std::vector<XMFLOAT3>& Vertices,
		const std::vector<XMFLOAT2>& TexCoords,
		const std::vector<FaceType>& Faces );
	bool WriteVertNormalToBinaryFile(
		const std::wstring &Filename,
		const std::vector<XMFLOAT3>& Vertices,
		const std::vector<XMFLOAT3>& Normals,
		const std::vector<FaceType>& Faces );
	bool WriteVertTexCoordNormalToBinaryFile(
		const std::wstring &Filename,
		const std::vector<XMFLOAT3>& Vertices,
		const std::vector<XMFLOAT2>& TexCoords,
		const std::vector<XMFLOAT3>& Normals,
		const std::vector<FaceType>& Faces );

	bool WriteToBinaryFile(
		const std::wstring &Filename,
		const std::vector<XMFLOAT3>& Vertices,
		const std::vector<XMFLOAT2>& TexCoords,
		const std::vector<XMFLOAT3>& Normals,
		const std::vector<FaceType>& Faces );

	void UpdateProgress( int CurrentVertex );
private:
	std::vector<XMFLOAT3> m_v, m_vn;
	std::vector<XMFLOAT2> m_vt;
	std::vector<FaceType> m_f;

	int m_vertexCount, m_textureCount, m_normalCount, m_faceCount;
	HWND m_TextboxProgress;
	std::unique_ptr<ObjLoaderBase> m_loader;
	bool m_hasTexCoord = false, m_hasNormals = false;
	bool m_isTriangle = false, m_isQuadrangle = false;
};

