//======================================================================================
//	Ed Kurlyak 2023 Torus Lighting Software Rendering
//======================================================================================

#ifndef _MESHMANAGER_
#define _MESHMANAGER_

#include <windows.h>
#include <math.h>
#include <stdio.h>

#include <vfw.h> //для DrawDibDraw
#pragma comment(lib, "Vfw32.lib") //для DrawDibDraw

#define PI 3.14159265358979f
#define PI2 (PI * 2.0f)

#define SWAP(a,b,Temp) {Temp=a; a=b; b=Temp;}

#define BITS_PER_PIXEL	32

struct matrix4x4
{
	float Mat[4][4];
};

struct vector3
{
	union
	{
		float Vec[3];
		struct
		{
			float xv, yv, zv;
		};
	};

	float xs, ys;

	vector3() {};
	vector3(float ix, float iy, float iz)
	{
		xv = ix;
		yv = iy;
		zv = iz;
	};

	vector3 operator - (const vector3 & VecIn)
	{
		vector3 VecOut;
		
		VecOut.xv = xv - VecIn.xv;
		VecOut.yv = yv - VecIn.yv;
		VecOut.zv = zv - VecIn.zv;
		
		return VecOut;
	}
};

struct color_rgb
{
	color_rgb() {};
	~color_rgb() {};

	color_rgb(int ir, int ig, int ib): r(ir), g(ig), b(ib) {};

	int r = 0;
	int g = 0;
	int b = 0;

	color_rgb operator + (const color_rgb & ColorIn)
	{
		color_rgb ColorOut;
		
		ColorOut.r = r + ColorIn.r;
		ColorOut.g = g + ColorIn.g;
		ColorOut.b = b + ColorIn.b;
		
		return ColorOut;
	}

	color_rgb operator * (const float& Val)
	{
		color_rgb ColorOut;

		ColorOut.r = (int)(r * Val);
		ColorOut.g = (int)(g * Val);
		ColorOut.b = (int)(b * Val);

		return ColorOut;
	}
};

class CMeshManager
{
public:

	CMeshManager();
	~CMeshManager();

	void Init_MeshManager(HWND hWnd);
	void Update_MeshManager();
	void Draw_MeshManager();

private:
	
	void Build_BackBuffer();
	void Clear_BackBuffer();
	void Present_BackBuffer();
	void Delete_BackBuffer();
	
	matrix4x4 Mat4x4_Mat4x4_Mul(matrix4x4 &MatIn1, matrix4x4 &MatIn2);
	vector3 Vec3_Mat4x4_Mul(vector3& VecIn, matrix4x4 &MatIn);
	float Vec3_Dot(vector3& VecIn1, vector3& Vec2In);
	vector3 Vec3_Normalize(vector3& VecIn);
	float Vec3_Len(vector3& VecIn);

	void Draw_Color_Triangle(float x1,float y1, float z1,
					   float x2,float y2, float z2,
					   float x3,float y3, float z3,
					   color_rgb Color1,
					   color_rgb Color2,
					   color_rgb Color3);
	void Draw_Color_Poly(int y1, int y2);

	HWND m_hWnd = NULL;

	UINT m_VertCount = 0;
	UINT m_TriangleCount = 0;

	ULONG m_ViewWidth = 0;
	ULONG m_ViewHeight = 0;

	vector3* m_VertBuff = NULL;
	vector3 *m_VertBuffTransformed = NULL;
	UINT *m_IndexBuff = NULL;
	vector3 *m_NormalBuff = NULL;
	vector3 *m_NormalBuffTransformed = NULL;

	BITMAPINFOHEADER m_Bih = { 0 };
	HDRAWDIB m_hDD = NULL;
	LPBYTE m_Data = NULL;

	float m_dredl = 0.0f, m_dgreenl = 0.0f, m_dbluel = 0.0f,
		m_dredr = 0.0f, m_dgreenr = 0.0f, m_dbluer = 0.0f,
		m_redl = 0.0f, m_greenl = 0.0f, m_bluel = 0.0f,
		m_redr = 0.0f, m_greenr = 0.0f, m_bluer = 0.0f,
		m_dxl = 0.0f, m_dxr = 0.0f, m_xl = 0.0f, m_xr = 0.0f;

	float m_zl = 0.0f, m_zr = 0.0f,
		m_dzl = 0.0f, m_dzr = 0.0f;

	float **m_ZBuff = NULL;
};

#endif