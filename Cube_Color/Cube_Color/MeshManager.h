//=============================================
//	Ed Kurlyak 2023 Color Interpolation
//=============================================

#ifndef _MESHMANAGER_
#define _MESHMANAGER_

#include <windows.h>
#include <math.h>
#include <stdio.h>

#include <vfw.h> //��� DrawDibDraw
#pragma comment(lib, "Vfw32.lib") //��� DrawDibDraw

#define PI 3.14159265358979f

#define BITS_PER_PIXEL	32

#define SWAP(a,b,Temp) {Temp=a; a=b; b=Temp;}

enum { A, B, C, D, E, F, G, H };

struct color_rgb
{
	int r = 0;
	int g = 0;
	int b = 0;
};

//matrix offset
enum {
	M00, M01, M02, M03,
	M10, M11, M12, M13,
	M20, M21, M22, M23,
	M30, M31, M32, M33
};

typedef float matrix4x4[16];

struct vector3
{
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
	
	vector3 & operator = (const vector3 & Vec)
	{
		x = Vec.x;
		y = Vec.y;
		z = Vec.z;

		return *this;
	}
};

class CMeshManager
{
public:

	CMeshManager();
	~CMeshManager();

	void Init_Cube(HWND hWnd);
	void Calculate_Cube();
	void Draw_Cube();

private:

	void Build_BackBuffer();
	void Clear_BackBuffer();
	void Present_BackBuffer();
	void Delete_BackBuffer();
	
	void Mat4x4_Mat4x4_Mul(matrix4x4 MatOut, matrix4x4 Mat1, matrix4x4 Mat2);
	void Vec3_Mat4x4_Mul(vector3& VecOut, vector3& Vec, matrix4x4 Mat);

	void Draw_Color_Triangle(float x1,float y1,
					   float x2,float y2,
					   float x3,float y3,
					   color_rgb Color1,
					   color_rgb Color2,
					   color_rgb Color3);
	void Draw_Color_Poly(int y1, int y2);

	HWND m_hWnd = NULL;
	
	UINT m_VertCount = 0;
	UINT m_TriangleCount = 0;

	UINT m_ViewWidth = 0;
	UINT m_ViewHeight = 0;

	vector3 *m_VertBuff = NULL;
	vector3 *m_VertBuffTransformed = NULL;
	DWORD *m_IndexBuff = NULL;
	color_rgb *m_ColorArray = NULL;

	BITMAPINFOHEADER m_Bih;
	HDRAWDIB m_hDD = NULL;
	LPBYTE m_Data = NULL;

	float m_dredl = 0.0f, m_dgreenl = 0.0f, m_dbluel = 0.0f,
		m_dredr = 0.0f, m_dgreenr = 0.0f, m_dbluer = 0.0f,
		m_redl = 0.0f, m_greenl = 0.0f, m_bluel = 0.0f,
		m_redr = 0.0f, m_greenr = 0.0f, m_bluer = 0.0f,
		m_dxl = 0.0f, m_dxr = 0.0f, m_xl = 0.0f, m_xr = 0.0f;
};

#endif