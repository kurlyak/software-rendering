//======================================================================================
//	Ed Kurlyak 2023 Wire Cube
//======================================================================================

#define _WIN32_WINNT 0x0500
#include <windows.h>
#include <conio.h>
#include <math.h>

#define PI 3.14159265358979f
#define PI2 (PI * 2.0f)

#define NUM_VERTICES 8
#define NUM_LINES 12

HWND g_hWnd;

struct vector3
{
	float x, y, z;
};

enum { A, B, C, D, E, F, G, H };

	/*
		CUBE VERTICES

		FONT SIDE	BACK SIDE
		C - D		G - H
		|	|		|	|
		A - B		E - F
	*/

vector3 g_VertBuff[NUM_VERTICES] = {
	-4.0, -4.0, -4.0,	//A
	 4.0, -4.0, -4.0,	//B
	-4.0,  4.0, -4.0,	//C
	 4.0,  4.0, -4.0,	//D

	-4.0, -4.0,  4.0,	//E
	 4.0, -4.0,  4.0,	//F
	-4.0,  4.0,  4.0,	//G
	 4.0,  4.0,  4.0 };	//H

vector3 g_VertBuffTransformed[NUM_VERTICES];

unsigned int g_IndexBuff[NUM_LINES * 2] = {
	A, B,
	B, D,
	D, C,
	C, A,

	E, F,
	F, H,
	H, G,
	G, E,

	A, E,
	B, F,
	D, H,
	C, G };

//matrix offset
enum {	M00, M01, M02, M03,
		M10, M11, M12, M13,
		M20, M21, M22, M23,
		M30, M31, M32, M33	};

typedef float matrix4x4[16];

vector3 Vec3_Mat4x4_Mul(vector3 & VecIn, matrix4x4 MatIn)
{
	vector3 VecOut;

	VecOut.x =	VecIn.x * MatIn[M00] +
				VecIn.y * MatIn[M10] +
				VecIn.z * MatIn[M20] +
					    MatIn[M30];

	VecOut.y =	VecIn.x * MatIn[M01] +
				VecIn.y * MatIn[M11] +
				VecIn.z * MatIn[M21] +
					    MatIn[M31];

	VecOut.z =	VecIn.x * MatIn[M02] +
				VecIn.y * MatIn[M12] +
				VecIn.z * MatIn[M22] +
					    MatIn[M32];

	return VecOut;
}

float Vec3_Dot(vector3& VecIn1, vector3& VecIn2)
{
	return VecIn1.x * VecIn2.x + VecIn1.y * VecIn2.y + VecIn1.z * VecIn2.z;
}

vector3 Vec3_Normalize(vector3& VecIn)
{
	vector3 VecOut;

	float Len = sqrtf((VecIn.x * VecIn.x) + (VecIn.y * VecIn.y) + (VecIn.z * VecIn.z));

	VecOut.x = VecIn.x / Len;
	VecOut.y = VecIn.y / Len;
	VecOut.z = VecIn.z / Len;

	return VecOut;
};

vector3 Vec3_Cross(vector3& VecIn1, vector3& VecIn2)
{
	vector3 VecOut;

	VecOut.x = VecIn1.y * VecIn2.z - VecIn1.z * VecIn2.y;
	VecOut.y = VecIn1.z * VecIn2.x - VecIn1.x * VecIn2.z;
	VecOut.z = VecIn1.x * VecIn2.y - VecIn1.y * VecIn2.x;

	return VecOut;
}

vector3 Vec3_Subtract(vector3& VecIn1, vector3& VecIn2)
{
	vector3 VecOut;

	VecOut.x = VecIn1.x - VecIn2.x;
	VecOut.y = VecIn1.y - VecIn2.y;
	VecOut.z = VecIn1.z - VecIn2.z;

	return VecOut;
}

void Draw_Cube()
{
	RECT Rc;
	GetClientRect(g_hWnd, &Rc);

	static float Angle = 0.0;

	matrix4x4 MatRotateY = {
		cosf(Angle),	0.0,	-sinf(Angle),	0.0,
		0.0,			1.0,	0.0,			0.0,
		sinf(Angle),	0.0,	cosf(Angle),	0.0,
		0.0,			0.0,	0.0,			1.0 };

	Angle = Angle + PI / 100.0f;
	if (Angle > PI2)
		Angle = 0.0f;

	//MATRIX VIEW CALCULATION
	vector3 VecRight = { 1.0f, 0.0f, 0.0 };
	vector3 VecUp = { 0.0f, 1.0f, 0.0f }; 
	vector3 VecCamPos = { 0.0f, 10.0f, 0.0f };
	vector3 VecModelPos = { 0.0, 0.0, 15.0f };
	vector3 VecLook;
	VecLook = Vec3_Subtract(VecModelPos, VecCamPos);

	VecLook = Vec3_Normalize(VecLook);

	VecUp = Vec3_Cross(VecLook, VecRight);
	VecUp = Vec3_Normalize(VecUp);
	VecRight = Vec3_Cross(VecUp, VecLook);
	VecRight = Vec3_Normalize(VecRight);

	float xp = -Vec3_Dot(VecCamPos, VecRight);
	float yp = -Vec3_Dot(VecCamPos, VecUp);
	float zp = -Vec3_Dot(VecCamPos, VecLook);

	matrix4x4 MatView = {
		VecRight.x,		VecUp.x,	VecLook.x,		0.0,
		VecRight.y,		VecUp.y,	VecLook.y,		0.0,
		VecRight.z,		VecUp.z,	VecLook.z,		0.0,
		xp,				yp,			zp,				1.0 };

	//MATRIX PROJECTION CALCULATION
	float fFov = PI / 2.0f; // FOV 90 degree
	float Aspect = (float)Rc.right / Rc.bottom;
	float ZFar = 100.0f;
	float ZNear = 1.0f;

	float h, w, Q;

	w = (1.0f / tanf(fFov * 0.5f)) / Aspect;
	h = 1.0f / tanf(fFov * 0.5f);
	Q = ZFar / (ZFar - ZNear);

	/*
	//полный расчет матрицы проекции
	matrix4x4 MatProj = {
		w, 0, 0, 0,
		0, h, 0, 0,
		0, 0, Q, 1,
		0, 0, -Q * ZNear, 0 };
	*/

	matrix4x4 MatProj = {
		w, 0, 0, 0,
		0, h, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1 };

	//MATRIX WORLD
	matrix4x4 MatWorld = {
		1.0f, 0.0, 0.0, 0.0,
		0.0, 1.0f, 0.0, 0.0,
		0.0, 0.0, 1.0f, 0.0,
		VecModelPos.x, VecModelPos.y, VecModelPos.z, 1.0f }; 

	//SCREEN MATRIX
	float Alpha = 0.5f * Rc.right;
	float Beta  = 0.5f * Rc.bottom;
	
	matrix4x4 MatScreen = {
		Alpha,  0,      0,    0, 
		0,      -Beta,  0,    0, 
		0,		0,		1,    0,
		Alpha,  Beta,	0,    1 };

	for (int i = 0; i < NUM_VERTICES; i++)
	{
		vector3 VecTemp = Vec3_Mat4x4_Mul(g_VertBuff[i], MatRotateY);
		VecTemp = Vec3_Mat4x4_Mul(VecTemp, MatWorld);
		VecTemp = Vec3_Mat4x4_Mul(VecTemp, MatView);
		VecTemp = Vec3_Mat4x4_Mul(VecTemp, MatProj);

		VecTemp.x = VecTemp.x / VecTemp.z;
		VecTemp.y = VecTemp.y / VecTemp.z;

		VecTemp = Vec3_Mat4x4_Mul(VecTemp, MatScreen);
		
		g_VertBuffTransformed[i] = VecTemp;
	}

	HDC hDC = GetDC(g_hWnd);

	HBRUSH hBrush = CreateSolidBrush(RGB(0, 0, 0));
	HBRUSH hOldBrush = (HBRUSH)SelectObject(hDC, hBrush);

	Rectangle(hDC, 0, 0, Rc.right, Rc.bottom);

	HPEN hPen = CreatePen(PS_SOLID, 4, RGB(255, 255, 127));
	HPEN hOldPen = (HPEN)SelectObject(hDC, hPen);

	for (int i = 0; i < NUM_LINES; i++)
	{
		vector3 Vec1 = g_VertBuffTransformed[g_IndexBuff[i * 2 + 0]];
		vector3 Vec2 = g_VertBuffTransformed[g_IndexBuff[i * 2 + 1]];

		MoveToEx(hDC, (int)Vec1.x, (int)Vec1.y, NULL);
		LineTo(hDC, (int)Vec2.x, (int)Vec2.y);
	}

	SelectObject(hDC, hOldBrush);
	DeleteObject(hBrush);

	SelectObject(hDC, hOldPen);
	DeleteObject(hPen);

	ReleaseDC(g_hWnd, hDC);

	Sleep(25);
}

int main()
{
	//код приведен для примера и не
	//претендует на производительность
	g_hWnd = GetConsoleWindow();

	while (!_kbhit())
	{
		Draw_Cube();
	}

	return 0;
}