#pragma once

#include "Includes.h"
#include "Utilities.h"

struct FaceType
{
	int vIndex1, vIndex2, vIndex3;
	int tIndex1, tIndex2, tIndex3;
	int nIndex1, nIndex2, nIndex3;
};

class OBJLoader
{
public:
	OBJLoader();
	~OBJLoader();

	bool Load(
		HWND TextboxProgress, 
		const std::wstring &FilenameLoad, 
		const std::wstring &FilenameSave );

private:
	bool ReadFileCounts( const std::wstring &Filename );
	bool ReadDataFromFile( 
		const std::wstring &Filename, 
		std::vector<XMFLOAT3>& Vertices, 
		std::vector<XMFLOAT2>& TextureCoords, 
		std::vector<XMFLOAT3>& Normals, 
		std::vector<FaceType>& Faces );
	bool WriteToTextFile(
		const std::wstring &Filename,
		const std::vector<XMFLOAT3>& Vertices, 
		const std::vector<XMFLOAT2>& TexCoords, 
		const std::vector<XMFLOAT3>& Normals,
		const std::vector<FaceType>& Faces);
	bool WriteToBinaryFile( 
		const std::wstring &Filename,
		const std::vector<XMFLOAT3>& Vertices,
		const std::vector<XMFLOAT2>& TexCoords,
		const std::vector<XMFLOAT3>& Normals,
		const std::vector<FaceType>& Faces );

	void UpdateProgress( int CurrentVertex );
private:
	std::vector<VertexPositionUVNormalType> m_vertexList;
	int m_vertexCount, m_textureCount, m_normalCount, m_faceCount;
	HWND m_TextboxProgress;
};

