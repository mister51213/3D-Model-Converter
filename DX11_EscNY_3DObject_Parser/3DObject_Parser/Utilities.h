#pragma once

#include <vector>
#include <DirectXMath.h>
#include <wrl.h>
#include <d3d11.h>

using namespace DirectX;

// Permit me to be a little lazy :)
// This is just an alias to shorten the amount of typing
template<class T>
using comptr = Microsoft::WRL::ComPtr<T>;

// Helper macros to decrease the amount of typing and space
#define RETURN_IF_FALSE(Result)\
if( !( Result ) ) \
{ \
	ShowCursor(TRUE);\
	return false; \
}\
else int a = 0
#define RETURN_MESSAGE_IF_FALSE(Result, Message)\
if(!(Result))\
{\
	ShowCursor(TRUE);\
	MessageBox(nullptr, Message, L"Error!", MB_OK);\
	return false;\
}\
else int a = 0
#define RETURN_IF_FAILED(Result) RETURN_IF_FALSE( SUCCEEDED( ( Result ) ) )

////////////////////////////////////////////////////
// Vector Math Operator Overloads
///////////////////////////////////////////////////

// Keep operator overloads out of namespace so we can access them
// without having to declare: using namespace
inline XMFLOAT3 operator+( const XMFLOAT3 &V1, const XMFLOAT3& V2 )
{
	return XMFLOAT3( V1.x + V2.x, V1.y + V2.y, V1.z + V2.z );
}
inline XMFLOAT3 operator-( const XMFLOAT3 &V1, const XMFLOAT3& V2 )
{
	return XMFLOAT3( V1.x - V2.x, V1.y - V2.y, V1.z - V2.z );
}
inline XMFLOAT3 operator*( const XMFLOAT3 &V, const float S )
{
	return XMFLOAT3( V.x * S, V.y * S, V.z * S );
}
inline XMFLOAT3 operator/( const XMFLOAT3 &V, const float S )
{
	const float recipScalar = 1.f / S;
	return XMFLOAT3( V.x * recipScalar, V.y * recipScalar, V.z * recipScalar );
}
inline XMFLOAT3 &operator+= ( XMFLOAT3& V1, const XMFLOAT3& V2 )
{
	V1 = V2 + V1;
	return V1;
}
inline XMFLOAT3 &operator-= ( XMFLOAT3& V1, const XMFLOAT3& V2 )
{
	V1 = V2 - V1;
	return V1;
}
inline XMFLOAT3 &operator*= ( XMFLOAT3 &V, const float S )
{
	V = V * S;
	return V;
}
inline XMFLOAT3 &operator/= ( XMFLOAT3 &V, const float S )
{
	V = V / S;
	return V;
}

// PI is equal to 180 degrees
constexpr float PI = 3.141592654f;

// TwoPI is equal to 360 degrees
constexpr float TwoPI = 2.f * PI;

// PIDivFour is equal to 45 degrees
constexpr float PIDivFour = PI / 4.f;

// PIDivTwo is equal to 90 degrees
constexpr float PIDivTwo = PI / 2.f;

// Radian is 0.01745329238f radians, multiply degrees by radian to convert 
// degrees to radians for rotation functions
constexpr float radian = PI / 180.f;

// Calculates the cross-product of two float3 vectors
inline XMFLOAT3 CrossProduct( const XMFLOAT3 &V1, const XMFLOAT3 &V2 )
{
	return
	{
		( V1.y * V2.z ) - ( V1.z * V2.y ),
		( V1.z * V2.x ) - ( V1.x * V2.z ),
		( V1.x * V2.y ) - ( V1.y * V2.x )
	};
}

// Calculates the dot-product of two float3 vectors
inline float DotProduct( const XMFLOAT3 &V1, const XMFLOAT3 &V2 )
{
	return ( V1.x * V2.x ) + ( V1.y * V2.y ) + ( V1.z * V2.z );
}

// Calculates the magnitude of a float3 vector
inline float Magnitude( const XMFLOAT3 &V )
{
	// The dot-product of an angle with itself is the same as
	// the magnitude of the vector squared, to get the magnitude
	// return the square root of the result.
	return sqrtf( DotProduct( V, V ) );
}

// Calcualtes the length between two float3 point vectors
inline float Length( const XMFLOAT3 &V1, const XMFLOAT3 &V2 )
{
	// Length between two points is the magnitude of the vector
	// that starts at point1 and goes to point2
	return Magnitude( V2 - V1 );
}

// Normalizes a float3 vector
inline XMFLOAT3 Normalize( const XMFLOAT3 &V )
{
	// Optimization
	// Division is the slowest basic math operation, so getting
	// the recipricol we can use multiplication instead, which is faster
	float recipricalLength = 1.0f / Magnitude( V );
	return V * recipricalLength;
}

inline XMVECTOR ConvertToRadians( const XMVECTOR& angleInDegrees )
{
	// Use constexpr radian which is the result of PI / 180.f calculated
	// at compile time.
	return angleInDegrees * XMVectorReplicate( radian );
}

inline XMFLOAT3 ConvertToRadians( const XMFLOAT3& angleInDegrees )
{
	// Use constexpr radian which is the result of PI / 180.f calculated
	// at compile time.
	return{ angleInDegrees * radian };
}

/////////////////////////////
// CLAMP FUNCTION
// (for clamping angles)
/////////////////////////////
// NOTE: The intent may have been for angles, but this can be 
// used for clamping in any context, it's a pretty generic function
template<typename T>
	static T Clamp(const T& x, const T& low, const T& high)
	{
		return x < low ? low : (x > high ? high : x); 
	}

////////////////////////////////////////////////////////////////
// STRUCTS CONTAINING MODEL INFO for gameplay and rendering
// TODO: make it global; see which is faster
////////////////////////////////////////////////////////////////
struct ModelSpecs_W
{
	XMFLOAT3 position, orientation, scale;
};

struct ModelSpecs_L
{
	/*ModelSpecs_L()
		:
		center( 0.f, 0.f, 0.f ),
		size( 5.f, 5.f, 5.f ),
		orientation( 0.f, 0.f, 0.f )
	{}*/
	ModelSpecs_L( 
		const XMFLOAT3 &Center = { 0.f, 0.f, 0.f },
		const XMFLOAT3 &Orientation = { 0.f,0.f,0.f },
   		const XMFLOAT3 &Size = { 5.f, 5.f, 5.f })
		:
		center( Center ),
    	orientation( Orientation ),
        size( Size )
	{}

	XMFLOAT3 center;
	XMFLOAT3 orientation;
   	XMFLOAT3 size;
};

enum eModType { CUBE, CUBE_TEXTURED, PLANE, SPHERE, POLYGON, CUSTOM_MESH };

enum eTexture
{
	AsphaltFresh = 0,
	AsphaltTGA = 1,
	AsphaltOld = 2,
	Water = 3,
	SharkSkin = 4
};


////////////////////////////////////////////////////////////////
// FUNCTION FOR GETTING WORLD MATRIX
// TODO: make it global; see which is faster
////////////////////////////////////////////////////////////////

inline DirectX::XMMATRIX GetWorldMatrix( ModelSpecs_W modSpecs
/*const XMFLOAT3& translate, const XMFLOAT3& rotate, const XMFLOAT3& scale*/ )
{
	DirectX::XMMATRIX trans = XMMatrixTranslation(/*translate.x, translate.y, translate.z*/modSpecs.position.x, modSpecs.position.y, modSpecs.position.z );
	auto rotation = ConvertToRadians( modSpecs.orientation );
	DirectX::XMMATRIX rot = XMMatrixRotationRollPitchYaw(/*rotate.x, rotate.y, rotate.z*/rotation.x, rotation.y, rotation.z );
	DirectX::XMMATRIX scal = XMMatrixScaling(/*scale.x, scale.y, scale.z*/modSpecs.scale.x, modSpecs.scale.y, modSpecs.scale.z );

	//return rot*scal*trans;
	return XMMatrixTranspose( rot*scal*trans ); // Also transpose it in this step to make it easier for GPU to handle
												// TODO: remove transponse world matrix from shader
}

// TODO: Make this global inline in Utilities
// Transpose the matrices to prepare them for the shader.
inline DirectX::XMMATRIX Transpose( std::vector<XMMATRIX> & matrices )
{
	// TODO:  Figure out best way to do this; what's the fastest way,
	// pass a list of matrices, or what?????

	/*MatrixBufferType data;
	data.world = XMMatrixTranspose( worldMatrix );
	data.view = XMMatrixTranspose( viewMatrix );
	data.projection = XMMatrixTranspose( projectionMatrix );*/
	return XMMatrixIdentity();
}

// Common vertex buffer types and corresponding input element descriptions

// Position and Color 
struct VertexPositionColorType
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT4 color;
	static std::vector<D3D11_INPUT_ELEMENT_DESC> CreateLayoutDescriptions()
	{
		///////////////////////////////////////////////////////////////////////////////////
		// Create the layout of the VERTEX DATA that will be processed by the shader.    //
		// We indicate the usage of each element in the layout to the shader by labeling //
		// the first one POSITION and the second one COLOR.                              //
		///////////////////////////////////////////////////////////////////////////////////

		std::vector<D3D11_INPUT_ELEMENT_DESC> eDesc( 2 );
		eDesc[ 0 ].SemanticName = "POSITION";
		eDesc[ 0 ].SemanticIndex = 0;
		eDesc[ 0 ].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		eDesc[ 0 ].InputSlot = 0;
		eDesc[ 0 ].AlignedByteOffset = 0;
		eDesc[ 0 ].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		eDesc[ 0 ].InstanceDataStepRate = 0;

		eDesc[ 1 ].SemanticName = "COLOR";
		eDesc[ 1 ].SemanticIndex = 0;
		eDesc[ 1 ].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		eDesc[ 1 ].InputSlot = 0;
		eDesc[ 1 ].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		eDesc[ 1 ].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		eDesc[ 1 ].InstanceDataStepRate = 0;

		return eDesc;
	}
};

// Position and texture coordinates
struct VertexPositionUVType
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT2 uv;

	// Input layout descriptions for position and texture coordinates
	static std::vector<D3D11_INPUT_ELEMENT_DESC> CreateLayoutDescriptions()
	{
		std::vector<D3D11_INPUT_ELEMENT_DESC> eDesc( 2 );
		eDesc[ 0 ].SemanticName = "POSITION";
		eDesc[ 0 ].SemanticIndex = 0;
		eDesc[ 0 ].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		eDesc[ 0 ].InputSlot = 0;
		eDesc[ 0 ].AlignedByteOffset = 0;
		eDesc[ 0 ].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		eDesc[ 0 ].InstanceDataStepRate = 0;

		eDesc[ 1 ].SemanticName = "TEXCOORD";
		eDesc[ 1 ].SemanticIndex = 0;
		eDesc[ 1 ].Format = DXGI_FORMAT_R32G32_FLOAT;
		eDesc[ 1 ].InputSlot = 0;
		eDesc[ 1 ].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		eDesc[ 1 ].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		eDesc[ 1 ].InstanceDataStepRate = 0;

		return eDesc;
	}

};

// Position, normal and texture coordinates
struct VertexPositionUVNormalType
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT2 uv;
	DirectX::XMFLOAT3 normal;

	// Input layout descriptions for position, normal and texture coordinates
	static std::vector<D3D11_INPUT_ELEMENT_DESC> CreateLayoutDescriptions()
	{
		std::vector<D3D11_INPUT_ELEMENT_DESC> eDesc( 2 );
		eDesc[ 0 ].SemanticName = "POSITION";
		eDesc[ 0 ].SemanticIndex = 0;
		eDesc[ 0 ].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		eDesc[ 0 ].InputSlot = 0;
		eDesc[ 0 ].AlignedByteOffset = 0;
		eDesc[ 0 ].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		eDesc[ 0 ].InstanceDataStepRate = 0;

		eDesc[ 1 ].SemanticName = "TEXCOORD";
		eDesc[ 1 ].SemanticIndex = 0;
		eDesc[ 1 ].Format = DXGI_FORMAT_R32G32_FLOAT;
		eDesc[ 1 ].InputSlot = 0;
		eDesc[ 1 ].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		eDesc[ 1 ].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		eDesc[ 1 ].InstanceDataStepRate = 0;

		eDesc[ 2 ].SemanticName = "NORMAL";
		eDesc[ 2 ].SemanticIndex = 0;
		eDesc[ 2 ].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		eDesc[ 2 ].InputSlot = 0;
		eDesc[ 2 ].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		eDesc[ 2 ].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		eDesc[ 2 ].InstanceDataStepRate = 0;

		return eDesc;
	}
};