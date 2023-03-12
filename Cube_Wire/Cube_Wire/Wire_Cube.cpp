//================================
//	Ed Kurlyak 2023 Wire Cube
//================================

#define _WIN32_WINNT 0x0500
#include <windows.h>
#include <conio.h>
#include <math.h>

#define PI 3.14159265358979f

#define NUM_VERTICES 8
#define NUM_LINES 12

HWND g_hWnd;

struct vector3 {
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

void Vec3_Mat4x4_Mul(vector3 & VecOut, vector3 & Vec, matrix4x4 Mat)
{
	VecOut.x =	Vec.x * Mat[M00] +
				Vec.y * Mat[M10] +
				Vec.z * Mat[M20] +
					    Mat[M30];

	VecOut.y =	Vec.x * Mat[M01] +
				Vec.y * Mat[M11] +
				Vec.z * Mat[M21] +
					    Mat[M31];

	VecOut.z =	Vec.x * Mat[M02] +
				Vec.y * Mat[M12] +
				Vec.z * Mat[M22] +
					    Mat[M32];
}

float Vec3_Dot(vector3& Vec1, vector3& Vec2)
{
	return Vec1.x * Vec2.x + Vec1.y * Vec2.y + Vec1.z * Vec2.z;
}

void Vec3_Normalize(vector3& VecOut, vector3& Vec)
{
	float Len = sqrtf((Vec.x * Vec.x) + (Vec.y * Vec.y) + (Vec.z * Vec.z));

	VecOut.x = Vec.x / Len;
	VecOut.y = Vec.y / Len;
	VecOut.z = Vec.z / Len;
};

void Vec3_Cross(vector3& VecOut, vector3& Vec1, vector3& Vec2)
{
	VecOut.x = Vec1.y * Vec2.z - Vec1.z * Vec2.y;
	VecOut.y = Vec1.z * Vec2.x - Vec1.x * Vec2.z;
	VecOut.z = Vec1.x * Vec2.y - Vec1.y * Vec2.x;
}

void Vec3_Subtract(vector3& VecOut, vector3& Vec1, vector3& Vec2)
{
	VecOut.x = Vec1.x - Vec2.x;
	VecOut.y = Vec1.y - Vec2.y;
	VecOut.z = Vec1.z - Vec2.z;
}

void Draw_Cube()
{
	RECT rc;
	GetClientRect(g_hWnd, &rc);

	static float Angle = 0.0;

	matrix4x4 MxRotateY = {
		cosf(Angle),	0.0,	-sinf(Angle),	0.0,
		0.0,			1.0,	0.0,			0.0,
		sinf(Angle),	0.0,	cosf(Angle),	0.0,
		0.0,			0.0,	0.0,			1.0 };

	Angle = Angle + PI / 100.0f;
	if (Angle > PI * 2.0f)
		Angle = 0.0f;

	//MATRIX VIEW CALCULATION
	vector3 Right = { 1.0f, 0.0f, 0.0 };
	vector3 Up = { 0.0f, 1.0f, 0.0f }; 
	vector3 Pos = { 0.0f, 10.0f, 0.0f };
	vector3 ModelPos = { 0.0, 0.0, 15.0 };
	vector3 Look;
	Vec3_Subtract(Look, ModelPos, Pos);

	Vec3_Normalize(Look, Look);

	Vec3_Cross(Up, Look, Right);
	Vec3_Normalize(Up, Up);
	Vec3_Cross(Right, Up, Look);
	Vec3_Normalize(Right, Right);

	float xp = -Vec3_Dot(Pos, Right);
	float yp = -Vec3_Dot(Pos, Up);
	float zp = -Vec3_Dot(Pos, Look);

	matrix4x4 MxView = {
		Right.x,	Up.x,	Look.x,	0.0,
		Right.y,   Up.y,  Look.y,    0.0,
		Right.z,   Up.z,  Look.z,    0.0,
		xp,			yp,		zp,			1.0 };

	//MATRIX PROJECTION CALCULATION
	float fFov = PI / 2.0f; // FOV 90 degree
	float Aspect = (float)rc.right / rc.bottom;
	float ZFar = 100.0f;
	float ZNear = 1.0f;

	float h, w, Q;

	w = (1.0f / tanf(fFov * 0.5f)) / Aspect;
	h = 1.0f / tanf(fFov * 0.5f);
	Q = ZFar / (ZFar - ZNear);

	/*
	//полный расчет матрицы проекции
	matrix4x4 MxProj = {
		w, 0, 0, 0,
		0, h, 0, 0,
		0, 0, Q, 1,
		0, 0, -Q * ZNear, 0 };
	*/

	matrix4x4 MxProj = {
		w, 0, 0, 0,
		0, h, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1 };

	//MATRIX WORLD
	matrix4x4 MxWorld = {
		1.0f, 0.0, 0.0, 0.0,
		0.0, 1.0f, 0.0, 0.0,
		0.0, 0.0, 1.0f, 0.0,
		ModelPos.x, ModelPos.y, ModelPos.z, 1.0f }; 

	//SCREEN MATRIX
	float Alpha = 0.5f * rc.right;
	float Beta  = 0.5f * rc.bottom;
	
	matrix4x4 MxScreen = {
		Alpha,  0,      0,    0, 
		0,      -Beta,  0,    0, 
		0,		0,		1,    0,
		Alpha,  Beta,	0,    1 };

	for (int i = 0; i < NUM_VERTICES; i++)
	{
		vector3 Vec1, Vec2;
		Vec3_Mat4x4_Mul(Vec1, g_VertBuff[i], MxRotateY);
		Vec3_Mat4x4_Mul(Vec2, Vec1, MxWorld);
		Vec3_Mat4x4_Mul(Vec1, Vec2, MxView);
		Vec3_Mat4x4_Mul(Vec2, Vec1, MxProj);

		Vec2.x = Vec2.x / Vec2.z;
		Vec2.y = Vec2.y / Vec2.z;

		Vec3_Mat4x4_Mul(Vec1, Vec2, MxScreen);
		
		g_VertBuffTransformed[i] = Vec1;
	}

	HDC hDC = GetDC(g_hWnd);

	HBRUSH hBrush = CreateSolidBrush(RGB(0, 0, 0));
	HBRUSH hOldBrush = (HBRUSH)SelectObject(hDC, hBrush);

	Rectangle(hDC, 0, 0, rc.right, rc.bottom);

	HPEN hPen = CreatePen(PS_SOLID, 4, RGB(255, 255, 127));
	HPEN hOldPen = (HPEN)SelectObject(hDC, hPen);

	for (int i = 0; i < NUM_LINES; i++)
	{
		vector3 Vec1 = g_VertBuffTransformed[g_IndexBuff[i * 2]];
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