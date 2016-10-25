#include "OBJLoader.h"
#include <CommCtrl.h>
#include <sstream>
#include <iomanip>



using namespace std;

OBJLoader::OBJLoader()
	:
	m_vertexCount( 0 ),
	m_textureCount( 0 ), 
	m_normalCount( 0 ), 
	m_faceCount( 0 )
{
}

OBJLoader::~OBJLoader()
{
}

bool OBJLoader::Load( 
	HWND TextboxProgress,
	const std::wstring &FilenameLoad, 
	const std::wstring &FilenameSave )
{
	m_TextboxProgress = TextboxProgress;

	// Counts vertex data and sets loader type (Triangle or Quadrangle loader)
	bool result = Preprocess( FilenameLoad );
	if ( !result ) return false;

	// Read in the data from the OBJ file
	result = m_loader->LoadData();
	if ( !result ) return false;

	// Write to custom binary file layout
	/*if ( m_hasTexCoord )
	{
		if ( m_hasNormals )
		{
			WriteVertTexCoordNormalToBinaryFile( 
				FilenameSave, vertices, texcoords, normals, faces );
		}
		else
		{
			WriteVertTexCoordToBinaryFile( FilenameSave, vertices, texcoords, faces );
		}
	}
	else if ( m_hasNormals )
	{
		WriteVertNormalToBinaryFile( FilenameSave, vertices, normals, faces );
	}
	else
	{
		WriteVertToBinaryFile( FilenameSave, vertices, faces );
	}*/

	return result;
}

// Just messing around with constexpr, but this struct is used to help
// test first two bytes of a string instead of testing each char separately.
struct CStr
{
	static constexpr char vType[] = { 'v', ' ', 'v', 't', 'v', 'n', 'f', ' ' };

	template<int Idx>
	struct GetPair
	{
		static constexpr short result = vType[ Idx ] | ( vType[ Idx + 1 ] << 8 );
	};
	struct GetVertexStr
	{
		static constexpr short result = GetPair<0>::result;
	};
	struct GetVertexTexCoordStr
	{
		static constexpr short result = GetPair<2>::result;
	};
	struct GetVertexNormalStr
	{
		static constexpr short result = GetPair<4>::result;
	};
	struct GetFaceStr
	{
		static constexpr short result = GetPair<6>::result;
	};
};

bool OBJLoader::Preprocess( const std::wstring & Filename )
{
	constexpr auto maxLineSize = 255u;

	auto file = std::ifstream( Filename );
	if ( !file.is_open() ) return false;

	auto buffer = make_unique<char[]>( maxLineSize );
	bool polyCountDetermined = false;

	// This loop only checks the first two bytes of each line, thus skipping
	// the rest of the line avoiding having to read every byte of the file.
	while ( !file.eof() )
	{
		file.getline( buffer.get(), maxLineSize );
		const auto iBuffer = reinterpret_cast< short* >( buffer.get() );

		switch ( *iBuffer )
		{
			case CStr::GetVertexStr::result:
				++m_vertexCount;
				break;
			case CStr::GetVertexTexCoordStr::result:
				++m_textureCount;
				break;
			case CStr::GetVertexNormalStr::result:
				++m_normalCount;
				break;
			case CStr::GetFaceStr::result:
			{
				// If already determined whether triangle, quadrangle or other
				// skip this part and just count the faces.  May need to revisit
				// this if files have mixed tris, quads and higher n-gons.
				if ( !polyCountDetermined )
				{
					const auto len = strlen( buffer.get() );
					unsigned wsCount = 0;
					for ( int i = 0; i < len; ++i )
					{
						if ( buffer[ i ] == ' ' )
						{
							++wsCount;
						}
					}
					if ( wsCount == 3 )
					{
						m_isTriangle = true;
						polyCountDetermined = true;
					}
					else if ( wsCount == 4 )
					{
						m_isQuadrangle = true;
						polyCountDetermined = true;
					}
				}

				++m_faceCount;
				break;
			}
		}
	}

	file.close();

	if ( m_isTriangle )
	{
		m_loader.reset( new ObjTriangleLoader( *this, Filename ) );
	}
	else if ( m_isQuadrangle )
	{
		m_loader.reset( new ObjQuadLoader( *this, Filename ) );
	}
	m_hasTexCoord = ( m_textureCount > 0 ) ? true : false;
	m_hasNormals = ( m_normalCount > 0 ) ? true : false;

	return true;
}

bool OBJLoader::WriteToTextFile(
	const std::wstring &Filename,
	const std::vector<XMFLOAT3>& Vertices,
	const std::vector<XMFLOAT2>& TexCoords,
	const std::vector<XMFLOAT3>& Normals,
	const std::vector<FaceType>& Faces )
{
	bool result = true;

	// Converting/Writing to file

	// Open the output file.
	ofstream file( Filename);

	// Write out the file header that our model format uses.
	file << "Vertex Count: " << ( m_faceCount * 3 ) << endl;
	file << endl;
	file << "Data:" << endl;
	file << endl;

	int vIndex = 0, tIndex = 0, nIndex = 0;
	// Now loop through all the faces and output the three vertices for each face.
	for( int i = 0; i < m_faceCount; i++ )
	{
		vIndex = Faces[ i ].vIndex1 - 1;
		tIndex = Faces[ i ].tIndex1 - 1;
		nIndex = Faces[ i ].nIndex1 - 1;

		file << Vertices[ vIndex ].x << ' ' << Vertices[ vIndex ].y << ' ' << Vertices[ vIndex ].z << ' '
			<< TexCoords[ tIndex ].x << ' ' << TexCoords[ tIndex ].y << ' '
			<< Normals[ nIndex ].x << ' ' << Normals[ nIndex ].y << ' ' << Normals[ nIndex ].z << endl;

		vIndex = Faces[ i ].vIndex2 - 1;
		tIndex = Faces[ i ].tIndex2 - 1;
		nIndex = Faces[ i ].nIndex2 - 1;

		file << Vertices[ vIndex ].x << ' ' << Vertices[ vIndex ].y << ' ' << Vertices[ vIndex ].z << ' '
			<< TexCoords[ tIndex ].x << ' ' << TexCoords[ tIndex ].y << ' '
			<< Normals[ nIndex ].x << ' ' << Normals[ nIndex ].y << ' ' << Normals[ nIndex ].z << endl;

		vIndex = Faces[ i ].vIndex3 - 1;
		tIndex = Faces[ i ].tIndex3 - 1;
		nIndex = Faces[ i ].nIndex3 - 1;

		file << Vertices[ vIndex ].x << ' ' << Vertices[ vIndex ].y << ' ' << Vertices[ vIndex ].z << ' '
			<< TexCoords[ tIndex ].x << ' ' << TexCoords[ tIndex ].y << ' '
			<< Normals[ nIndex ].x << ' ' << Normals[ nIndex ].y << ' ' << Normals[ nIndex ].z << endl;
	}

	// Close the output file.
	file.close();

	return result;
}

bool OBJLoader::WriteVertToBinaryFile( const std::wstring & Filename, const std::vector<XMFLOAT3>& Vertices, const std::vector<FaceType>& Faces )
{
	return false;
}

bool OBJLoader::WriteVertTexCoordToBinaryFile( const std::wstring & Filename, const std::vector<XMFLOAT3>& Vertices, const std::vector<XMFLOAT2>& TexCoords, const std::vector<FaceType>& Faces )
{
	return false;
}

bool OBJLoader::WriteVertNormalToBinaryFile(
	const std::wstring & Filename, 
	const std::vector<XMFLOAT3>& Vertices, 
	const std::vector<XMFLOAT3>& Normals, 
	const std::vector<FaceType>& Faces )
{
	int vertCount = m_faceCount * 3;
	if ( m_TextboxProgress )
	{
		SendMessage(
			m_TextboxProgress, PBM_SETRANGE32, 0, MAKELPARAM( 0, vertCount )
		);
		SetWindowText( m_TextboxProgress, L"Writing file..." );
	}

	vector<XMFLOAT3> v( vertCount ), vn( vertCount );
	// i is for index of faces, j is for index of vertexFormats list
	for ( int i = 0, j = 0; i < m_faceCount; i++, j += 3 )
	{
		// Sometimes faces indices will be negative, so multiplying by -1 to make them positive, if negative.
		auto vIndex = Faces[ i ].vIndex1 > 0 ? Faces[ i ].vIndex1 - 1 : -( Faces[ i ].vIndex1 + 1 );
		auto nIndex = Faces[ i ].nIndex1 > 0 ? Faces[ i ].nIndex1 - 1 : -( Faces[ i ].nIndex1 + 1 );

		int k = j;
		if ( m_TextboxProgress )
		{
			UpdateProgress( k );
		}

		v[ k ] = { Vertices[ vIndex ].x, Vertices[ vIndex ].y, Vertices[ vIndex ].z };
		vn[ k ] = { Normals[ nIndex ].x, Normals[ nIndex ].y, Normals[ nIndex ].z };

		// Sometimes faces indices will be negative, so multiplying by -1 to make them positive, if negative.
		vIndex = Faces[ i ].vIndex2 > 0 ? Faces[ i ].vIndex2 - 1 : -( Faces[ i ].vIndex2 + 1 );
		nIndex = Faces[ i ].nIndex2 > 0 ? Faces[ i ].nIndex2 - 1 : -( Faces[ i ].nIndex2 + 1 );

		++k;
		v[ k ] = { Vertices[ vIndex ].x, Vertices[ vIndex ].y, Vertices[ vIndex ].z };
		vn[ k ] = { Normals[ nIndex ].x, Normals[ nIndex ].y, Normals[ nIndex ].z };

		// Sometimes faces indices will be negative, so multiplying by -1 to make them positive, if negative.
		vIndex = Faces[ i ].vIndex3 > 0 ? Faces[ i ].vIndex3 - 1 : -( Faces[ i ].vIndex3 + 1 );
		nIndex = Faces[ i ].nIndex3 > 0 ? Faces[ i ].nIndex3 - 1 : -( Faces[ i ].nIndex3 + 1 );

		++k;
		v[ k ] = { Vertices[ vIndex ].x, Vertices[ vIndex ].y, Vertices[ vIndex ].z };
		vn[ k ] = { Normals[ nIndex ].x, Normals[ nIndex ].y, Normals[ nIndex ].z };
	}

	const auto pvBuffer = reinterpret_cast<char*>( v.data() );
	const auto pvnBuffer = reinterpret_cast<char*>( vn.data() );
	const auto float3BufferSize = sizeof( XMFLOAT3 ) * vertCount;

	const auto pDataBuffer = reinterpret_cast<char*>( &vertCount );
	std::streamsize bufferSize = static_cast<streamsize>( sizeof( int ) );
	// Open the output file.
	ofstream file( Filename, std::ios::binary );
	if ( file.fail() )
	{
		return false;
	}

	const char texCount = 0;
	// Write vertex count into first 32 bits
	file.write( pDataBuffer, bufferSize );
	file.write( &texCount, bufferSize );
	file.write( pDataBuffer, bufferSize );


	file.write( pvBuffer, float3BufferSize );
	file.write( pvnBuffer, float3BufferSize );

	// Close the output file.
	file.close();


	return true;
}

bool OBJLoader::WriteVertTexCoordNormalToBinaryFile(
	const std::wstring &Filename,
	const std::vector<XMFLOAT3>& Vertices,
	const std::vector<XMFLOAT2>& TexCoords,
	const std::vector<XMFLOAT3>& Normals,
	const std::vector<FaceType>& Faces )
{
	// Write vertex count into first 32 bits
	int vertCount = m_faceCount * 3;
	if( m_TextboxProgress )
	{
		SendMessage(
			m_TextboxProgress, PBM_SETRANGE32, 0, MAKELPARAM( 0, vertCount )
		);
		SetWindowText( m_TextboxProgress, L"Writing file..." );
	}

	vector<XMFLOAT3> v( vertCount ), vn( vertCount );
	vector<XMFLOAT2> vt( vertCount );
	// i is for index of faces, j is for index of vertexFormats list
	for( int i = 0, j = 0; i < m_faceCount; i++, j += 3 )
	{
		// Sometimes faces indices will be negative, so multiplying by -1 to make them positive, if negative.
		auto vIndex = Faces[ i ].vIndex1 > 0 ? Faces[ i ].vIndex1 - 1 : -( Faces[ i ].vIndex1 + 1 );
		auto tIndex = Faces[ i ].tIndex1 > 0 ? Faces[ i ].tIndex1 - 1 : -( Faces[ i ].tIndex1 + 1 );
		auto nIndex = Faces[ i ].nIndex1 > 0 ? Faces[ i ].nIndex1 - 1 : -( Faces[ i ].nIndex1 + 1 );

		int k = j;
		if( m_TextboxProgress )
		{
			UpdateProgress( k );
		}

		v[ k ] = { Vertices[ vIndex ].x, Vertices[ vIndex ].y, Vertices[ vIndex ].z };
		vt[ k ] = { TexCoords[ tIndex ].x, TexCoords[ tIndex ].y };
		vn[ k ] = { Normals[ nIndex ].x, Normals[ nIndex ].y, Normals[ nIndex ].z };

		// Sometimes faces indices will be negative, so multiplying by -1 to make them positive, if negative.
		vIndex = Faces[ i ].vIndex2 > 0 ? Faces[ i ].vIndex2 - 1 : -( Faces[ i ].vIndex2 + 1 );
		tIndex = Faces[ i ].tIndex2 > 0 ? Faces[ i ].tIndex2 - 1 : -( Faces[ i ].tIndex2 + 1 );
		nIndex = Faces[ i ].nIndex2 > 0 ? Faces[ i ].nIndex2 - 1 : -( Faces[ i ].nIndex2 + 1 );

		++k;
		v[ k ] = { Vertices[ vIndex ].x, Vertices[ vIndex ].y, Vertices[ vIndex ].z };
		vt[ k ] = { TexCoords[ tIndex ].x, TexCoords[ tIndex ].y };
		vn[ k ] = { Normals[ nIndex ].x, Normals[ nIndex ].y, Normals[ nIndex ].z };

		// Sometimes faces indices will be negative, so multiplying by -1 to make them positive, if negative.
		vIndex = Faces[ i ].vIndex3 > 0 ? Faces[ i ].vIndex3 - 1 : -( Faces[ i ].vIndex3 + 1 );
		tIndex = Faces[ i ].tIndex3 > 0 ? Faces[ i ].tIndex3 - 1 : -( Faces[ i ].tIndex3 + 1 );
		nIndex = Faces[ i ].nIndex3 > 0 ? Faces[ i ].nIndex3 - 1 : -( Faces[ i ].nIndex3 + 1 );

		++k;
		v[ k ] = { Vertices[ vIndex ].x, Vertices[ vIndex ].y, Vertices[ vIndex ].z };
		vt[ k ] = { TexCoords[ tIndex ].x, TexCoords[ tIndex ].y };
		vn[ k ] = { Normals[ nIndex ].x, Normals[ nIndex ].y, Normals[ nIndex ].z };
	}

	const auto pvBuffer = reinterpret_cast<char*>( v.data() );
	const auto pvtBuffer = reinterpret_cast<char*>( vt.data() );
	const auto pvnBuffer = reinterpret_cast<char*>( vn.data() );
	const auto float3BufferSize = sizeof( XMFLOAT3 ) * vertCount;
	const auto float2BufferSize = sizeof( XMFLOAT2 ) * vertCount;

	const auto pDataBuffer = reinterpret_cast<char*>( &vertCount );
	std::streamsize bufferSize = static_cast<streamsize>( sizeof( int ) );
	// Open the output file.
	ofstream file( Filename, std::ios::binary );
	if ( file.fail() )
	{
		return false;
	}

	file.write( pDataBuffer, bufferSize );
	file.write( pDataBuffer, bufferSize );
	file.write( pDataBuffer, bufferSize );


	file.write( pvBuffer, float3BufferSize );
	file.write( pvtBuffer, float2BufferSize );
	file.write( pvnBuffer, float3BufferSize );

	// Close the output file.
	file.close();


	return true;

}

void OBJLoader::UpdateProgress( int CurrentVertex )
{
	if( m_TextboxProgress )
	{
		SendMessage(
			m_TextboxProgress, PBM_STEPIT, CurrentVertex, 0
		);
	}
}

bool OBJLoader::ObjTriangleLoader::LoadData()
{
	vector<XMFLOAT3> vertices( m_parent.m_vertexCount ), normals( m_parent.m_normalCount );
	vector<XMFLOAT2> texcoords( m_parent.m_textureCount );
	vector<FaceType> faces( m_parent.m_faceCount );
	auto file = std::ifstream( m_filename );

	const auto maxLineSize = 255u;	
	auto GetLine = [ &file, &maxLineSize ]()
	{
		std::unique_ptr<char[]> buffer( new char[ maxLineSize ] );
		file.getline( buffer.get(), maxLineSize );
		return buffer;
	};

	auto v_index = 0u;
	auto t_index = 0u;
	auto n_index = 0u;
	auto f_index = 0u;

	auto FillVertexPosition = [ &v_index, &file, &vertices, this ]( const int BuffLength )
	{
		file.seekg( -( BuffLength - 2l ), file.cur );
		
		vertices[ v_index ] = [ &file ]()
		{
			float x, y, z;
			file >> x >> y >> z;
			return XMFLOAT3( x, y, z );
		}( );
				
		const auto val = file.get();
		++v_index;
	};
	auto FillTexCoordinates = [ &t_index, &file, &texcoords, this ]( const int BuffLength )
	{
		file.seekg( -( BuffLength - 3l ), file.cur );
		file
			>> texcoords[ t_index ].x
			>> texcoords[ t_index ].y;
		const auto val = file.get();
		++t_index;
	};
	auto FillVertexNormals = [ &n_index, &file, &normals, this ]( const int BufferLength )
	{
		file.seekg( -( BufferLength - 3l ), file.cur );
		file
			>> normals[ n_index ].x
			>> normals[ n_index ].y
			>> normals[ n_index ].z;
		const auto val = file.get();
		++n_index;
	};

	// Lambda for filling [V]ertex, [T]extureCoord and [N]ormals data
	auto FillFaceVTN = [ &f_index, &file, &faces ]()
	{
		char c = 0;
		
		file
			>> faces[ f_index ].vIndex3 >> c >> faces[ f_index ].tIndex3 >> c >> faces[ f_index ].nIndex3
			>> faces[ f_index ].vIndex2 >> c >> faces[ f_index ].tIndex2 >> c >> faces[ f_index ].nIndex2
			>> faces[ f_index ].vIndex1 >> c >> faces[ f_index ].tIndex1 >> c >> faces[ f_index ].nIndex1;
	};
	// Lambda for filling [V]ertex and [T]extureCoord data
	auto FillFaceVT  = [ &f_index, &file, &faces ]()
	{
		char c = 0;
		file
			>> faces[ f_index ].vIndex3 >> c >> faces[ f_index ].tIndex3
			>> faces[ f_index ].vIndex2 >> c >> faces[ f_index ].tIndex2
			>> faces[ f_index ].vIndex1 >> c >> faces[ f_index ].tIndex1;
	};
	// Lambda for filling [V]ertex and [N]ormals data
	auto FillFaceVN  = [ &f_index, &file, &faces ]()
	{
		char c = 0;
		file
			>> faces[ f_index ].vIndex3 >> c >> c >> faces[ f_index ].nIndex3
			>> faces[ f_index ].vIndex2 >> c >> c >> faces[ f_index ].nIndex2
			>> faces[ f_index ].vIndex1 >> c >> c >> faces[ f_index ].nIndex1;
	};
	// Lambda for filling [V]ertex  data
	auto FillFaceV   = [ &f_index, &file, &faces ]()
	{
		char c = 0;
		file
			>> faces[ f_index ].vIndex3 >> c >> c
			>> faces[ f_index ].vIndex2 >> c >> c
			>> faces[ f_index ].vIndex1 >> c >> c;
	};
	// Lambda for sorting out which face data to collect
	auto FillFaces   = [ & ]( const int BufferLength )
	{
		// For some reason, the indices don't need to be offset like the
		// other float data for vertex, texture and normals
		file.seekg( -( BufferLength ), file.cur );
		if ( m_parent.m_hasTexCoord && m_parent.m_hasNormals )
		{
			FillFaceVTN();
		}
		else if ( m_parent.m_hasTexCoord && !m_parent.m_hasNormals )
		{
			FillFaceVT();
		}
		else if ( !m_parent.m_hasTexCoord && m_parent.m_hasNormals )
		{
			FillFaceVN();
		}
		else
		{
			FillFaceV();
		}
		const auto val = file.get();
		++f_index;
	};

	// This loop only checks the first two bytes of each line, thus skipping
	// the rest of the line avoiding having to read every byte of the file.
	while ( !file.eof() )
	{
		// Gets a line of data and stores in a unique_ptr<char[]>
		const auto buffer = GetLine();
		
		// Converts the char array to an short array and checks the first
		// two bytes for 'v ', 'vt', 'vn' and 'f '.
		const auto iBuffer = reinterpret_cast<const short* >( buffer.get() );
		const auto bufLen = static_cast<int>( strlen( buffer.get() ) );

		// Bytes are stored in little endian, so compare against
		// ' v', 'tv', 'nv' and ' f'
		switch ( *iBuffer )
		{
			case CStr::GetVertexStr::result:
				FillVertexPosition( bufLen );
				break;
			case CStr::GetVertexTexCoordStr::result:
				FillTexCoordinates( bufLen );
				break;
			case CStr::GetVertexNormalStr::result:
				FillVertexNormals( bufLen );
				break;
			case CStr::GetFaceStr::result:
				FillFaces( bufLen );
				break;
		}
	}

	// Moves local std::vectors to parent owned std::vectors
	m_parent.m_v = std::move( vertices );
	m_parent.m_vt = std::move( texcoords );
	m_parent.m_vn = std::move( normals );
	m_parent.m_f = std::move( faces );

	return true;
}

bool OBJLoader::ObjQuadLoader::LoadData()
{
	vector<XMFLOAT3> vertices( m_parent.m_vertexCount ), normals( m_parent.m_normalCount );
	vector<XMFLOAT2> texcoords( m_parent.m_textureCount );
	vector<FaceTypeQuad> faces( m_parent.m_faceCount );

	const auto maxLineSize = 255u;

	auto v_index = 0u;
	auto t_index = 0u;
	auto n_index = 0u;
	auto f_index = 0u;
	auto file = std::ifstream( m_filename );
	while ( !file.eof() )
	{
		const auto buffer = [ &file, &maxLineSize ]()
		{
			std::unique_ptr<char[]> buffer( new char[ maxLineSize ] );
			file.getline( buffer.get(), maxLineSize );
			return buffer;
		}( );

		const auto iBuffer = reinterpret_cast<const short* >( buffer.get() );
		const auto bufLen = static_cast<int>( strlen( buffer.get() ) );

		switch ( *iBuffer )
		{
			case CStr::GetVertexStr::result:
				file.seekg( -( bufLen - 2l ), file.cur );
				file
					>> vertices[ v_index ].x
					>> vertices[ v_index ].y
					>> vertices[ v_index ].z;
				++v_index;
				break;
			case CStr::GetVertexTexCoordStr::result:
				file.seekg( -( bufLen - 3l ), file.cur );
				file
					>> texcoords[ t_index ].x
					>> texcoords[ t_index ].y;
				++t_index;
				break;
			case CStr::GetVertexNormalStr::result:
				file.seekg( -( bufLen - 3l ), file.cur );
				file
					>> normals[ n_index ].x
					>> normals[ n_index ].y
					>> normals[ n_index ].z;
				++n_index;
				break;
			case CStr::GetFaceStr::result:
			{
				file.seekg( -( bufLen - 2l ), file.cur );
				if ( m_parent.m_hasTexCoord && m_parent.m_hasNormals )
				{
					char c = 0;
					file
						>> faces[ f_index ].vIndex4 >> c >> faces[ f_index ].tIndex4 >> c >> faces[ f_index ].nIndex4
						>> faces[ f_index ].vIndex3 >> c >> faces[ f_index ].tIndex3 >> c >> faces[ f_index ].nIndex3
						>> faces[ f_index ].vIndex2 >> c >> faces[ f_index ].tIndex2 >> c >> faces[ f_index ].nIndex2
						>> faces[ f_index ].vIndex1 >> c >> faces[ f_index ].tIndex1 >> c >> faces[ f_index ].nIndex1;

				}
				else if ( m_parent.m_hasTexCoord && !m_parent.m_hasNormals )
				{
					char c = 0;
					file
						>> faces[ f_index ].vIndex4 >> c >> faces[ f_index ].tIndex4
						>> faces[ f_index ].vIndex3 >> c >> faces[ f_index ].tIndex3
						>> faces[ f_index ].vIndex2 >> c >> faces[ f_index ].tIndex2
						>> faces[ f_index ].vIndex1 >> c >> faces[ f_index ].tIndex1;
				}
				else if ( !m_parent.m_hasTexCoord && m_parent.m_hasNormals )
				{
					char c = 0;
					file
						>> faces[ f_index ].vIndex4 >> c >> c >> faces[ f_index ].nIndex4
						>> faces[ f_index ].vIndex3 >> c >> c >> faces[ f_index ].nIndex3
						>> faces[ f_index ].vIndex2 >> c >> c >> faces[ f_index ].nIndex2
						>> faces[ f_index ].vIndex1 >> c >> c >> faces[ f_index ].nIndex1;
				}
				else
				{
					char c = 0;
					file
						>> faces[ f_index ].vIndex4 >> c >> c
						>> faces[ f_index ].vIndex3 >> c >> c
						>> faces[ f_index ].vIndex2 >> c >> c
						>> faces[ f_index ].vIndex1 >> c >> c;
				}
				++f_index;
				break;
			}
		}
	}
	return true;
}
