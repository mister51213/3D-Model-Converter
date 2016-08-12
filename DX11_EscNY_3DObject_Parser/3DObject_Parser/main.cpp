#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <DirectXMath.h>
using namespace std;
using namespace DirectX;

/*
When examining the file you can ignore every line unless it 
starts with a "V", "VT", "VN", or "F". 
*/

//////////////
// TYPEDEFS //
//////////////
typedef struct
{
	float x, y, z;
}VertexType;

typedef struct
{
	int vIndex1, vIndex2, vIndex3;
	int tIndex1, tIndex2, tIndex3;
	int nIndex1, nIndex2, nIndex3;
}FaceType;

struct VertexPositionUVNormalType
{
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT2 uv;
    DirectX::XMFLOAT3 normal;
};

/////////////////////////
// FUNCTION PROTOTYPES //
/////////////////////////
void GetModelFilename(char*);
bool ReadFileCounts(char*, int&, int&, int&, int&);
bool LoadDataStructures(char*, int, int, int, int);
bool LoadDataBinary(char*, int, int, int, int);
// TODO: custom craft this function to do the same thing but load into binary
// -> Will produce TWO files, a custom txt and a binary!
//bool LoadIntoBinary(char*, int, int, int, int); - can be integrated into above function
// TODO: reverse this function to load obj into binary float array
/*vector<VertexPositionUVNormalType> vertList(vertexCount);
file.read(reinterpret_cast<char*>(vertList.data()), sizeof(VertexPositionUVNormalType)*vertexCount);*/

int main()
{
    bool result;
    char filename[256];
    int vertexCount, textureCount, normalCount, faceCount;
    char garbage;

    // Read in the name of the model file.
    GetModelFilename(filename);

    result = ReadFileCounts(filename, vertexCount, textureCount, normalCount, faceCount);
    if (!result)
    {
        return -1;
    }

    // Display the counts to the screen for information purposes.
    cout << endl;
    cout << "Vertices: " << vertexCount << endl;
    cout << "UVs:      " << textureCount << endl;
    cout << "Normals:  " << normalCount << endl;
    cout << "Faces:    " << faceCount << endl;

    // Now read the data from the file into the data structures and then output it in our model format.
    result = LoadDataStructures(filename, vertexCount, textureCount, normalCount, faceCount);
    if (!result)
    {
        return -1;
    }
    // Now do the same for binary output file
    result = LoadDataBinary(filename, vertexCount, textureCount, normalCount, faceCount);
    if (!result)
    {
        return -1;
    }

    // Notify the user the model has been converted.
    cout << "\nFile has been converted." << endl;
    cout << "\nDo you wish to exit (y/n)? ";
    cin >> garbage;

    return 0;
}

void GetModelFilename(char* filename)
{
	bool done;
	ifstream fin;

	// Loop until we have a file name.
	done = false;
	while(!done)
	{
		// Ask the user for the filename.
		cout << "Enter model filename: ";

		// Read in the filename.
		cin >> filename;

		// Attempt to open the file.
		fin.open(filename);

		if(fin.good())
		{
			// If the file exists and there are no problems then exit since we have the file name.
			done = true;
		}
		else
		{
			// If the file does not exist or there was an issue opening it then notify the user and repeat the process.
			fin.clear();
			cout << endl;
			cout << "File " << filename << " could not be opened." << endl << endl;
		}
	}

	return;
}

bool ReadFileCounts(char* filename, int& vertexCount, int& textureCount, int& normalCount, int& faceCount)
{
	ifstream fin;
	char input;

	// Initialize the counts.
	vertexCount = 0;
	textureCount = 0;
	normalCount = 0;
	faceCount = 0;

	// Open the file.
	fin.open(filename);

	// Check if it was successful in opening the file.
	if(fin.fail() == true)
	{
		return false;
	}

	// Read from the file and continue to read until the end of the file is reached.
	fin.get(input);
	while(!fin.eof())
	{
		// If the line starts with 'v' then count either the vertex, the texture coordinates, or the normal vector.
		if(input == 'v')
		{
			fin.get(input);
			if(input == ' ') { vertexCount++; }
			if(input == 't') { textureCount++; }
			if(input == 'n') { normalCount++; }
		}

		// If the line starts with 'f' then increment the face count.
		if(input == 'f')
		{
			fin.get(input);
			if(input == ' ') { faceCount++; }
		}
		
		// Otherwise read in the remainder of the line (iterate to the end)
		while(input != '\n' && !fin.eof())
		{
			fin.get(input);
		}

		// Start reading the beginning of the next line.
		fin.get(input);
	}

	// Close the file.
	fin.close();

	return true;
}

bool LoadDataStructures(char* filename, int vertexCount, int textureCount, int normalCount, int faceCount)
{
    /******************************************************************************
    *  Following code is same for both binary and txt output formats              *
    *  v   v   v   v   v   v   v   v   v   v   v   v   v   v   v   v   v   v   v  *
    *******************************************************************************/
    // VertexType is same as an XMFLOAT3
	VertexType *vertices, *texcoords, *normals;
    // FaceType is 3 sets of 3 ints = 9 total (for vertex, texture, and normal)
	FaceType *faces;

	ifstream fin; // used to INPUT the file
	int vertexIndex, texcoordIndex, normalIndex, faceIndex, vIndex, tIndex, nIndex;
	char input, input2;
	ofstream fout; // used to OUTPUT the file

	// Initialize the four data structures.
	vertices = new VertexType[vertexCount];
	if(!vertices)
	{
		return false;
	}

	texcoords = new VertexType[textureCount];
	if(!texcoords)
	{
		return false;
	}

	normals = new VertexType[normalCount];
	if(!normals)
	{
		return false;
	}

	faces = new FaceType[faceCount];
	if(!faces)
	{
		return false;
	}

	// Initialize the indexes.
	vertexIndex = 0;
	texcoordIndex = 0;
	normalIndex = 0;
	faceIndex = 0;

	// Open the file.
	fin.open(filename);

	// Check if it was successful in opening the file.
	if(fin.fail() == true)
	{
		return false;
	}

	// Read in the vertices, texture coordinates, and normals into the data structures.
	// Important: Also convert to left hand coordinate system since Maya uses right hand coordinate system.
	fin.get(input);
	while(!fin.eof())
	{
		if(input == 'v')
		{
			fin.get(input);

			// Read in the vertices.
			if(input == ' ') 
			{ 
				fin >> vertices[vertexIndex].x >> vertices[vertexIndex].y >> vertices[vertexIndex].z;

				// Invert the Z vertex to change to left hand system.
				vertices[vertexIndex].z = vertices[vertexIndex].z * -1.0f;
				vertexIndex++; 
			}

            // TODO: for loading the binary, we should manually skip the z value of teach texcoord
			// Read in the texture uv coordinates.
			if(input == 't') 
			{ 
				fin >> texcoords[texcoordIndex].x >> texcoords[texcoordIndex].y;

				// Invert the V texture coordinates to left hand system.
				texcoords[texcoordIndex].y = 1.0f - texcoords[texcoordIndex].y;
				texcoordIndex++; 
			}

			// Read in the normals.
			if(input == 'n') 
			{ 
				fin >> normals[normalIndex].x >> normals[normalIndex].y >> normals[normalIndex].z;

				// Invert the Z normal to change to left hand system.
				normals[normalIndex].z = normals[normalIndex].z * -1.0f;
				normalIndex++; 
			}
		}

		// Read in the faces.
		if(input == 'f') 
		{
			fin.get(input);
			if(input == ' ')
			{
				// Read the face data in backwards to convert it to a left hand system from right hand system.
				fin >> faces[faceIndex].vIndex3 >> input2 >> faces[faceIndex].tIndex3 >> input2 >> faces[faceIndex].nIndex3
				    >> faces[faceIndex].vIndex2 >> input2 >> faces[faceIndex].tIndex2 >> input2 >> faces[faceIndex].nIndex2
				    >> faces[faceIndex].vIndex1 >> input2 >> faces[faceIndex].tIndex1 >> input2 >> faces[faceIndex].nIndex1;
				faceIndex++;
			}
		}

		// Read in the remainder of the line.
		while(input != '\n' && !fin.eof())
		{
			fin.get(input);
		}

		// Start reading the beginning of the next line.
		fin.get(input);
	}

	// Close the file.
	fin.close();

    /******************************************************************************
    *  ^   ^   ^   ^   ^   ^   ^   ^    ^   ^   ^   ^   ^   ^   ^   ^   ^   ^  ^  *
    *  Code up to here should be the same for both binary and txt output formats  *
    *******************************************************************************/

	// Open the output file.
	fout.open("model.txt");
	
	// Write out the file header that our model format uses.
	fout << "Vertex Count: " << (faceCount * 3) << endl;
	fout << endl;
	fout << "Data:" << endl;
	fout << endl;

	// Now loop through all the faces and output the three vertices for each face.
	for(int i = 0; i < faceIndex; i++)
	{
		vIndex = faces[i].vIndex1 - 1;
		tIndex = faces[i].tIndex1 - 1;
		nIndex = faces[i].nIndex1 - 1;

		fout << vertices[vIndex].x << ' ' << vertices[vIndex].y << ' ' << vertices[vIndex].z << ' '
		     << texcoords[tIndex].x << ' ' << texcoords[tIndex].y << ' '
		     << normals[nIndex].x << ' ' << normals[nIndex].y << ' ' << normals[nIndex].z << endl;

		vIndex = faces[i].vIndex2 - 1;
		tIndex = faces[i].tIndex2 - 1;
		nIndex = faces[i].nIndex2 - 1;

		fout << vertices[vIndex].x << ' ' << vertices[vIndex].y << ' ' << vertices[vIndex].z << ' '
		     << texcoords[tIndex].x << ' ' << texcoords[tIndex].y << ' '
		     << normals[nIndex].x << ' ' << normals[nIndex].y << ' ' << normals[nIndex].z << endl;

		vIndex = faces[i].vIndex3 - 1;
		tIndex = faces[i].tIndex3 - 1;
		nIndex = faces[i].nIndex3 - 1;

		fout << vertices[vIndex].x << ' ' << vertices[vIndex].y << ' ' << vertices[vIndex].z << ' '
		     << texcoords[tIndex].x << ' ' << texcoords[tIndex].y << ' '
		     << normals[nIndex].x << ' ' << normals[nIndex].y << ' ' << normals[nIndex].z << endl;
	}

	// Close the output file.
	fout.close();

	// Release the four data structures.
	if(vertices)
	{
		delete [] vertices;
		vertices = 0;
	}
	if(texcoords)
	{
		delete [] texcoords;
		texcoords = 0;
	}
	if(normals)
	{
		delete [] normals;
		normals = 0;
	}
	if(faces)
	{
		delete [] faces;
		faces = 0;
	}

	return true;
}

    /******************************************************************************
    *****************************BINARY VERSION************************************
    *******************************************************************************/
bool LoadDataBinary(char* filename, int vertexCount, int textureCount, int normalCount, int faceCount)
{
    /******************************************************************************
    *  Following code is same for both binary and txt output formats              *
    *  v   v   v   v   v   v   v   v   v   v   v   v   v   v   v   v   v   v   v  *
    *******************************************************************************/
    // VertexType is same as an XMFLOAT3
	VertexType *vertices, *texcoords, *normals;
    // FaceType is 3 sets of 3 ints = 9 total (for vertex, texture, and normal)
	FaceType *faces;

	ifstream fin; // used to INPUT the file
	int vertexIndex, texcoordIndex, normalIndex, faceIndex, vIndex, tIndex, nIndex;
	char input, input2;
	ofstream fout; // used to OUTPUT the file

	// Initialize the four data structures.
	vertices = new VertexType[vertexCount];
	if(!vertices)
	{
		return false;
	}

	texcoords = new VertexType[textureCount];
	if(!texcoords)
	{
		return false;
	}

	normals = new VertexType[normalCount];
	if(!normals)
	{
		return false;
	}

	faces = new FaceType[faceCount];
	if(!faces)
	{
		return false;
	}

	// Initialize the indexes.
	vertexIndex = 0;
	texcoordIndex = 0;
	normalIndex = 0;
	faceIndex = 0;

	// Open the file.
	fin.open(filename);

	// Check if it was successful in opening the file.
	if(fin.fail() == true)
	{
		return false;
	}

	// Read in the vertices, texture coordinates, and normals into the data structures.
	// Important: Also convert to left hand coordinate system since Maya uses right hand coordinate system.
	fin.get(input);
	while(!fin.eof())
	{
		if(input == 'v')
		{
			fin.get(input);

			// Read in the vertices.
			if(input == ' ') 
			{ 
				fin >> vertices[vertexIndex].x >> vertices[vertexIndex].y >> vertices[vertexIndex].z;

				// Invert the Z vertex to change to left hand system.
				vertices[vertexIndex].z = vertices[vertexIndex].z * -1.0f;
				vertexIndex++; 
			}

            // TODO: for loading the binary, we should manually skip the z value of teach texcoord
			// Read in the texture uv coordinates.
			if(input == 't') 
			{ 
				fin >> texcoords[texcoordIndex].x >> texcoords[texcoordIndex].y;

				// Invert the V texture coordinates to left hand system.
				texcoords[texcoordIndex].y = 1.0f - texcoords[texcoordIndex].y;
				texcoordIndex++; 
			}

			// Read in the normals.
			if(input == 'n') 
			{ 
				fin >> normals[normalIndex].x >> normals[normalIndex].y >> normals[normalIndex].z;

				// Invert the Z normal to change to left hand system.
				normals[normalIndex].z = normals[normalIndex].z * -1.0f;
				normalIndex++; 
			}
		}

		// Read in the faces.
		if(input == 'f') 
		{
			fin.get(input);
			if(input == ' ')
			{
				// Read the face data in backwards to convert it to a left hand system from right hand system.
				fin >> faces[faceIndex].vIndex3 >> input2 >> faces[faceIndex].tIndex3 >> input2 >> faces[faceIndex].nIndex3
				    >> faces[faceIndex].vIndex2 >> input2 >> faces[faceIndex].tIndex2 >> input2 >> faces[faceIndex].nIndex2
				    >> faces[faceIndex].vIndex1 >> input2 >> faces[faceIndex].tIndex1 >> input2 >> faces[faceIndex].nIndex1;
				faceIndex++;
			}
		}

		// Read in the remainder of the line.
		while(input != '\n' && !fin.eof())
		{
			fin.get(input);
		}

		// Start reading the beginning of the next line.
		fin.get(input);
	}

	// Close the file.
	fin.close();
    /******************************************************************************
    *  ^   ^   ^   ^   ^   ^   ^   ^    ^   ^   ^   ^   ^   ^   ^   ^   ^   ^  ^  *
    *  Code up to here should be the same for both binary and txt output formats  *
    *******************************************************************************/
	// Open the output file.
	fout.open("model.BinaryMesh", std::ios::binary);
	
	// Write vertex count into first 32 bits
    int vertCount = faceCount * 3;
	fout.write(reinterpret_cast<char*>(&vertCount), sizeof(int));

    vector<VertexPositionUVNormalType> vertexFormats(vertCount);
    // i is for index of faces, j is for index of vertexFormats list
    	for(int i = 0, j = 0; i < faceCount; i++, j+=3)
	{
		vIndex = faces[i].vIndex1 - 1;
		tIndex = faces[i].tIndex1 - 1;
		nIndex = faces[i].nIndex1 - 1;

        vertexFormats[j].position = { vertices[vIndex].x, vertices[vIndex].y, vertices[vIndex].z };
        vertexFormats[j].uv = { texcoords[tIndex].x, texcoords[tIndex].y };
        vertexFormats[j].normal = { normals[nIndex].x, normals[nIndex].y, normals[nIndex].z };

		vIndex = faces[i].vIndex2 - 1;
		tIndex = faces[i].tIndex2 - 1;
		nIndex = faces[i].nIndex2 - 1;

        vertexFormats[j+1].position = { vertices[vIndex].x, vertices[vIndex].y, vertices[vIndex].z };
        vertexFormats[j+1].uv = { texcoords[tIndex].x, texcoords[tIndex].y };
        vertexFormats[j+1].normal = { normals[nIndex].x, normals[nIndex].y, normals[nIndex].z };

		vIndex = faces[i].vIndex3 - 1;
		tIndex = faces[i].tIndex3 - 1;
		nIndex = faces[i].nIndex3 - 1;

        vertexFormats[j+2].position = { vertices[vIndex].x, vertices[vIndex].y, vertices[vIndex].z };
        vertexFormats[j+2].uv = { texcoords[tIndex].x, texcoords[tIndex].y };
        vertexFormats[j+2].normal = { normals[nIndex].x, normals[nIndex].y, normals[nIndex].z };
	}

// OLD VERSION
	// Now loop through all the faces and output the three vertices for each face.
	//for(int i = 0; i < faceIndex; i++)
	//{
	//	vIndex = faces[i].vIndex1 - 1;
	//	tIndex = faces[i].tIndex1 - 1;
	//	nIndex = faces[i].nIndex1 - 1;
 //       // *Insertion operator doesn't work here
	//	fout << vertices[vIndex].x << vertices[vIndex].y << vertices[vIndex].z 
	//	     << texcoords[tIndex].x << texcoords[tIndex].y 
	//	     << normals[nIndex].x << normals[nIndex].y << normals[nIndex].z;
	//	vIndex = faces[i].vIndex2 - 1;
	//	tIndex = faces[i].tIndex2 - 1;
	//	nIndex = faces[i].nIndex2 - 1;
	//	fout << vertices[vIndex].x << vertices[vIndex].y << vertices[vIndex].z
	//	     << texcoords[tIndex].x << texcoords[tIndex].y
	//	     << normals[nIndex].x << normals[nIndex].y << normals[nIndex].z;
	//	vIndex = faces[i].vIndex3 - 1;
	//	tIndex = faces[i].tIndex3 - 1;
	//	nIndex = faces[i].nIndex3 - 1;
	//	fout << vertices[vIndex].x << vertices[vIndex].y << vertices[vIndex].z
	//	     << texcoords[tIndex].x << texcoords[tIndex].y
	//	     << normals[nIndex].x << normals[nIndex].y << normals[nIndex].z;
	//}

        fout.write(reinterpret_cast<char*>(vertexFormats.data()), sizeof(VertexPositionUVNormalType)*vertCount);

	// Close the output file.
	fout.close();

	// Release the four data structures.
	if(vertices)
	{
		delete [] vertices;
		vertices = 0;
	}
	if(texcoords)
	{
		delete [] texcoords;
		texcoords = 0;
	}
	if(normals)
	{
		delete [] normals;
		normals = 0;
	}
	if(faces)
	{
		delete [] faces;
		faces = 0;
	}

	return true;
}