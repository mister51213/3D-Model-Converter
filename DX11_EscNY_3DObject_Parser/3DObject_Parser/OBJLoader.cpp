#include "OBJLoader.h"
#include <CommCtrl.h>

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
	bool result = ReadFileCounts( FilenameLoad );
	if( !result )
	{
		return result;
	}

	// Initialize the vector of data structures.
	vector<XMFLOAT3> vertices( m_vertexCount ), normals( m_normalCount );
	vector<XMFLOAT2> texcoords( m_textureCount );
	vector<FaceType> faces( m_faceCount );

	// Read in the data from the OBJ file
	ReadDataFromFile( FilenameLoad, vertices, texcoords, normals, faces );

	// Write to custom text file layout
	//WriteToTextFile( FilenameSave, vertices, texcoords, normals, faces );

	// Write to custom binary file layout
	WriteToBinaryFile( FilenameSave, vertices, texcoords, normals, faces );

	return result;
}

bool OBJLoader::ReadFileCounts( const std::wstring &Filename )
{
	// Open the file.
	ifstream file( Filename, std::ios::ate );
	auto filesize = file.tellg().seekpos();
	file.seekg( std::ios::beg );

	if( m_TextboxProgress )
	{
		SendMessage(
			m_TextboxProgress, PBM_SETRANGE32, 0, MAKELPARAM(0, filesize )
		);
		SetWindowText( m_TextboxProgress, L"Counting vertices..." );
	}


	// Check if it was successful in opening the file.
	bool result = file.good();
	if( result )
	{
		char input = 0;
		int count = 0;
		// Read from the file and continue to read until the end of the file is reached.
		while( count < filesize )
		{
			if( m_TextboxProgress )
			{
				UpdateProgress( count );
			}

			file.get( input );
			++count;
			switch( input )
			{
				// If the line starts with 'v' then check next character.
				case 'v':
					file.get( input );
					++count;
					switch( input )
					{
						// If ' ' then count vertex
						case ' ':
							++m_vertexCount;
							break;
							// If 't' then count texture coordinates
						case 't':
							++m_textureCount;
							break;
							// If 'n' then count normal
						case 'n':
							++m_normalCount;
							break;
					}
					break;
					// If the line starts with 'f' then increment the face count.
				case 'f':
					file.get( input );
					++count;
					if( input == ' ' )
					{
						++m_faceCount;
					}
					break;
			}			
		}

		// Close the file.
		file.close();

		if( m_faceCount <= 0 )
		{
			result = false;
		}
	}

	return result;
}

bool OBJLoader::ReadDataFromFile(
	const std::wstring &Filename, 
	std::vector<XMFLOAT3>& Vertices, 
	std::vector<XMFLOAT2>& TexCoords, 
	std::vector<XMFLOAT3>& Normals, 
	std::vector<FaceType>& Faces )
{
	// used to INPUT the file
	char input, input2;

	// Initialize the indexes.
	int vertexIndex = 0;
	int texcoordIndex = 0;
	int normalIndex = 0;
	int faceIndex = 0;

	// Open the file.
	ifstream file( Filename, std::ios::ate );

	// Check if it was successful in opening the file.
	if( file.fail() == true )
	{
		return false;
	}

	// Read in the vertices, texture coordinates, and normals into the data structures.
	// Important: Also convert to left hand coordinate system since Maya uses right hand coordinate system.
	int filesize = file.tellg().seekpos();
	file.seekg( std::ios::beg );

	if( m_TextboxProgress )
	{
		SendMessage(
			m_TextboxProgress, PBM_SETRANGE32, 0, MAKELPARAM( 0, filesize )
		);
		SetWindowText( m_TextboxProgress, L"Reading file..." );
	}

	int count = 0;
	while( count < filesize )
	{
		if( m_TextboxProgress )
		{
			UpdateProgress( count );
		}
		file.get( input );
		++count;
		if( input == 'v' )
		{
			file.get( input );
			++count;

			// Read in the vertices.
			if( input == ' ' )
			{
				file >> Vertices[ vertexIndex ].x >> Vertices[ vertexIndex ].y >> Vertices[ vertexIndex ].z;

				// Invert the Z vertex to change to left hand system.
				Vertices[ vertexIndex ].z = Vertices[ vertexIndex ].z * -1.0f;
				vertexIndex++;
			}

			// TODO: for loading the binary, we should manually skip the z value of teach texcoord
			// Read in the texture uv coordinates.
			if( input == 't' )
			{
				file >> TexCoords[ texcoordIndex ].x >> TexCoords[ texcoordIndex ].y;

				// Invert the V texture coordinates to left hand system.
				TexCoords[ texcoordIndex ].y = 1.0f - TexCoords[ texcoordIndex ].y;
				texcoordIndex++;
			}

			// Read in the normals.
			if( input == 'n' )
			{
				file >> Normals[ normalIndex ].x >> Normals[ normalIndex ].y >> Normals[ normalIndex ].z;

				// Invert the Z normal to change to left hand system.
				Normals[ normalIndex ].z = Normals[ normalIndex ].z * -1.0f;
				normalIndex++;
			}
		}

		// Read in the faces.
		if( input == 'f' )
		{
			file.get( input );
			++count;
			if( input == ' ' )
			{
				// Read the face data in backwards to convert it to a left hand system from right hand system.
				file >> Faces[ faceIndex ].vIndex3 >> input2 >> Faces[ faceIndex ].tIndex3 >> input2 >> Faces[ faceIndex ].nIndex3
					>> Faces[ faceIndex ].vIndex2 >> input2 >> Faces[ faceIndex ].tIndex2 >> input2 >> Faces[ faceIndex ].nIndex2
					>> Faces[ faceIndex ].vIndex1 >> input2 >> Faces[ faceIndex ].tIndex1 >> input2 >> Faces[ faceIndex ].nIndex1;

				faceIndex++;
			}
		}
	}

	// Close the file.
	file.close();

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

bool OBJLoader::WriteToBinaryFile( 
	const std::wstring &Filename,
	const std::vector<XMFLOAT3>& Vertices,
	const std::vector<XMFLOAT2>& TexCoords,
	const std::vector<XMFLOAT3>& Normals,
	const std::vector<FaceType>& Faces )
{
	// Open the output file.
	ofstream file( Filename, std::ios::binary );
	if( file.fail() )
	{
		return false;
	}

	// Write vertex count into first 32 bits
	int vertCount = m_faceCount * 3;
	if( m_TextboxProgress )
	{
		SendMessage(
			m_TextboxProgress, PBM_SETRANGE32, 0, MAKELPARAM( 0, vertCount )
		);
		SetWindowText( m_TextboxProgress, L"Writing file..." );
	}

	char *pDataBuffer = reinterpret_cast<char*>( &vertCount );
	std::streamsize bufferSize = static_cast<streamsize>( sizeof( int ) );

	file.write( pDataBuffer, bufferSize );

	vector<VertexPositionUVNormalType> vertexFormats( vertCount );
	// i is for index of faces, j is for index of vertexFormats list
	for( int i = 0, j = 0; i < m_faceCount; i++, j += 3 )
	{
		// Sometimes faces indices will be negative, so multiplying by -1 to make them positive, if negative.
		int vIndex = Faces[ i ].vIndex1 > 0 ? Faces[ i ].vIndex1 - 1 : -( Faces[ i ].vIndex1 + 1 );
		int tIndex = Faces[ i ].tIndex1 > 0 ? Faces[ i ].tIndex1 - 1 : -( Faces[ i ].tIndex1 + 1 );
		int nIndex = Faces[ i ].nIndex1 > 0 ? Faces[ i ].nIndex1 - 1 : -( Faces[ i ].nIndex1 + 1 );

		int k = j;
		if( m_TextboxProgress )
		{
			UpdateProgress( k );
		}

		assert( k < vertexFormats.size() );
		vertexFormats[ k ].position = { Vertices[ vIndex ].x, Vertices[ vIndex ].y, Vertices[ vIndex ].z };
		vertexFormats[ k  ].uv = { TexCoords[ tIndex ].x, TexCoords[ tIndex ].y };
		vertexFormats[ k  ].normal = { Normals[ nIndex ].x, Normals[ nIndex ].y, Normals[ nIndex ].z };

		// Sometimes faces indices will be negative, so multiplying by -1 to make them positive, if negative.
		vIndex = Faces[ i ].vIndex2 > 0 ? Faces[ i ].vIndex2 - 1 : -( Faces[ i ].vIndex2 + 1 );
		tIndex = Faces[ i ].tIndex2 > 0 ? Faces[ i ].tIndex2 - 1 : -( Faces[ i ].tIndex2 + 1 );
		nIndex = Faces[ i ].nIndex2 > 0 ? Faces[ i ].nIndex2 - 1 : -( Faces[ i ].nIndex2 + 1 );

		++k;
		assert( k < vertexFormats.size() );
		vertexFormats[ k ].position = { Vertices[ vIndex ].x, Vertices[ vIndex ].y, Vertices[ vIndex ].z };
		vertexFormats[ k ].uv = { TexCoords[ tIndex ].x, TexCoords[ tIndex ].y };
		vertexFormats[ k ].normal = { Normals[ nIndex ].x, Normals[ nIndex ].y, Normals[ nIndex ].z };

		// Sometimes faces indices will be negative, so multiplying by -1 to make them positive, if negative.
		vIndex = Faces[ i ].vIndex3 > 0 ? Faces[ i ].vIndex3 - 1 : -( Faces[ i ].vIndex3 + 1 );
		tIndex = Faces[ i ].tIndex3 > 0 ? Faces[ i ].tIndex3 - 1 : -( Faces[ i ].tIndex3 + 1 );
		nIndex = Faces[ i ].nIndex3 > 0 ? Faces[ i ].nIndex3 - 1 : -( Faces[ i ].nIndex3 + 1 );

		++k;
		assert( k < vertexFormats.size() );
		vertexFormats[ k ].position = { Vertices[ vIndex ].x, Vertices[ vIndex ].y, Vertices[ vIndex ].z };
		vertexFormats[ k ].uv = { TexCoords[ tIndex ].x, TexCoords[ tIndex ].y };
		vertexFormats[ k ].normal = { Normals[ nIndex ].x, Normals[ nIndex ].y, Normals[ nIndex ].z };

	}

	pDataBuffer = reinterpret_cast<char*>( vertexFormats.data() );
	bufferSize = sizeof( VertexPositionUVNormalType )*vertCount;
	file.write( pDataBuffer, bufferSize );

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
