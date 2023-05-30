//======================================================================================
//	Ed Kurlyak 2023 Textured Sphere Mesh Software
//======================================================================================

#ifndef _MESHMANAGER_
#define _MESHMANAGER_

#include <windows.h>
#include <math.h>
#include <stdio.h>

#include <vfw.h> //для DrawDibDraw
#pragma comment(lib, "Vfw32.lib") //для DrawDibDraw

#define PI 3.14159265358979f
#define TWOPI 6.28318530717958f

#define BITS_PER_PIXEL	32

typedef float matrix4x4[4][4];

struct tex_coord2
{
	int tu, tv = 0;
};

struct vector3
{
	vector3() {};
	vector3(float ix, float iy, float iz) : x(ix), y(iy), z(iz) {};

	union
	{
		float Mat[3];
		struct {
			float x,y,z;
		};
	};

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

	void Init_MeshManager(HWND hWnd);
	void Update_MeshManager();
	void Draw_MeshManager();

private:

	void Create_Backbuffer();
	void Clear_Backbuffer();
	void Present_Backbuffer();
	void Delete_Backbuffer();

	void Read_BMP_File(const char* TexFileName);

	void Vec3_Mat4x4_Mul(vector3& VecOut, vector3& Vec, matrix4x4 Mat);

	void Draw_Textured_Triangle(vector3 Vec1, tex_coord2 Tex1,
		vector3 Vec2, tex_coord2 Tex2,
		vector3 Vec3, tex_coord2 Tex3);
	void Draw_Textured_Poly(int y1, int y2);

	HWND m_hWnd = NULL;

	UCHAR* m_Res = NULL;
	UINT m_TextureWidth = 0;
	UINT m_TextureHeight = 0;

	UINT m_VertCount = 0;
	UINT m_TriangleCount = 0;

	BITMAPINFOHEADER m_Bih = { 0 };
	HDRAWDIB m_hDD = NULL;
	LPBYTE m_Data = NULL;
	LPBYTE m_DataTemp = NULL;

	UINT m_ViewWidth = 0;
	UINT m_ViewHeight = 0;
	
	vector3 *m_VertBuff = NULL;
	vector3 *m_VertBuffTransformed = NULL;
	tex_coord2 *m_TexCoord = NULL;
	DWORD *m_IndexBuff = NULL;

	float m_dxdyl = 0.0f, m_dudyl = 0.0f,
		m_dvdyl = 0.0f, m_dzdyl = 0.0f,
		m_dxdyr = 0.0f, m_dudyr = 0.0f,
		m_dvdyr = 0.0f, m_dzdyr = 0.0f;
	float m_ul = 0.0f, m_ur = 0.0f,
		m_vl = 0.0f, m_vr = 0.0f,
		m_zl = 0.0f, m_zr = 0.0f,
		m_xl = 0.0f, m_xr = 0.0f;
};

#endif