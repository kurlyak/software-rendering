//================================
//	Ed Kurlyak 2023 Color Cube
//================================

#include "MeshManager.h"

CMeshManager::CMeshManager()
{
};

CMeshManager::~CMeshManager()
{
	Delete_BackBuffer();

	delete [] m_VertBuff;
	delete [] m_VertBuffTransformed;
	delete [] m_IndexBuff;

}

void CMeshManager::Mat4x4_Mat4x4_Mul(matrix4x4 MatOut, matrix4x4 Mat1, matrix4x4 Mat2)
{
	//row1 * col1
	MatOut[M00] = Mat1[M00]*Mat2[M00] + Mat1[M01]*Mat2[M10] + Mat1[M02]*Mat2[M20] + Mat1[M03]*Mat2[M30];
	//row1 * col2
	MatOut[M01] = Mat1[M00]*Mat2[M01] + Mat1[M01]*Mat2[M11] + Mat1[M02]*Mat2[M21] + Mat1[M03]*Mat2[M31];
	//row1 * col3
	MatOut[M02] = Mat1[M00]*Mat2[M02] + Mat1[M01]*Mat2[M12] + Mat1[M02]*Mat2[M22] + Mat1[M03]*Mat2[M32];
	//row1 * col4
	MatOut[M03] = Mat1[M00]*Mat2[M03] + Mat1[M01]*Mat2[M13] + Mat1[M02]*Mat2[M23] + Mat1[M03]*Mat2[M33];

	//row2 * col1
	MatOut[M10] = Mat1[M10]*Mat2[M00] + Mat1[M11]*Mat2[M10] + Mat1[M12]*Mat2[M20] + Mat1[M13]*Mat2[M30];
	//row2 * col2
	MatOut[M11] = Mat1[M10]*Mat2[M01] + Mat1[M11]*Mat2[M11] + Mat1[M12]*Mat2[M21] + Mat1[M13]*Mat2[M31];
	//row2 * col3
	MatOut[M12] = Mat1[M10]*Mat2[M02] + Mat1[M11]*Mat2[M12] + Mat1[M12]*Mat2[M22] + Mat1[M13]*Mat2[M32];
	//row2 * col4
	MatOut[M13] = Mat1[M10]*Mat2[M03] + Mat1[M11]*Mat2[M13] + Mat1[M12]*Mat2[M23] + Mat1[M13]*Mat2[M33];

	//row3 * col1
	MatOut[M20] = Mat1[M20]*Mat2[M00] + Mat1[M21]*Mat2[M10] + Mat1[M22]*Mat2[M20] + Mat1[M23]*Mat2[M30];
	//row3 * col2
	MatOut[M21] = Mat1[M20]*Mat2[M01] + Mat1[M21]*Mat2[M11] + Mat1[M22]*Mat2[M21] + Mat1[M23]*Mat2[M31];
	//row3 * col3
	MatOut[M22] = Mat1[M20]*Mat2[M02] + Mat1[M21]*Mat2[M12] + Mat1[M22]*Mat2[M22] + Mat1[M23]*Mat2[M32];
	//row3 * col4
	MatOut[M23] = Mat1[M20]*Mat2[M03] + Mat1[M21]*Mat2[M13] + Mat1[M22]*Mat2[M23] + Mat1[M23]*Mat2[M33];
	
	//row4 * col1
	MatOut[M30] = Mat1[M30]*Mat2[M00] + Mat1[M31]*Mat2[M10] + Mat1[M32]*Mat2[M20] + Mat1[M33]*Mat2[M30];
	//row4 * col2
	MatOut[M31] = Mat1[M30]*Mat2[M01] + Mat1[M31]*Mat2[M11] + Mat1[M32]*Mat2[M21] + Mat1[M33]*Mat2[M31];
	//row4 * col3
	MatOut[M32] = Mat1[M30]*Mat2[M02] + Mat1[M31]*Mat2[M12] + Mat1[M32]*Mat2[M22] + Mat1[M33]*Mat2[M32];
	//row4 * col4
	MatOut[M33] = Mat1[M30]*Mat2[M03] + Mat1[M31]*Mat2[M13] + Mat1[M32]*Mat2[M23] + Mat1[M33]*Mat2[M33];
}

void CMeshManager::Vec3_Mat4x4_Mul(vector3& VecOut, vector3& Vec, matrix4x4 Mat)
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

float CMeshManager::Vec3_Dot(vector3& Vec1, vector3& Vec2)
{
	return Vec1.x * Vec2.x + Vec1.y * Vec2.y + Vec1.z * Vec2.z;
}

void CMeshManager::Vec3_Normalize(vector3& VecOut, vector3& Vec)
{
	float Len = sqrtf((Vec.x * Vec.x) + (Vec.y * Vec.y) + (Vec.z * Vec.z));

	VecOut.x = Vec.x / Len;
	VecOut.y = Vec.y / Len;
	VecOut.z = Vec.z / Len;
};

void CMeshManager::Vec3_Cross(vector3& VecOut, vector3& Vec1, vector3& Vec2)
{
	VecOut.x = Vec1.y * Vec2.z - Vec1.z * Vec2.y;
	VecOut.y = Vec1.z * Vec2.x - Vec1.x * Vec2.z;
	VecOut.z = Vec1.x * Vec2.y - Vec1.y * Vec2.x;
}

void CMeshManager::Init_Cube(HWND hWnd)
{
	m_hWnd = hWnd;

	Create_BackBuffer();

	m_VertCount = 8;
	m_TriangleCount = 12;
	
	/*
		CUBE VERTICES

		FONT SIDE	BACK SIDE
		C - D		G - H
		|	|		|	|
		A - B		E - F
	*/

	vector3 vert_buff_temp[8] = {
		-5.0f,  -5.0f, -5.0f,	// A
		 5.0f,  -5.0f, -5.0f,	// B
		-5.0f,   5.0f, -5.0f,	// C
		 5.0f,   5.0f, -5.0f,	// D
				
		-5.0f,  -5.0f,  5.0f,	// E
		 5.0f,  -5.0f,  5.0f,	// F
		-5.0f,   5.0f,  5.0f,	// G
		 5.0f,   5.0f,  5.0f };	// H
	
	DWORD index_buff_temp[36] = {
		//front face	
		A, C, D,
		A, D, B,
	
		//left face
		E, G, C,
		E, C, A,

		//back face
		G, E, F,
		G, F, H,

		//right face
		B, D, H,
		B, H, F,

		//top face
		C, G, H,
		C, H, D,

		//bottom face
		E, A, B,
		E, B, F };

	for (UINT i = 0; i < m_TriangleCount; i++)
	{
		m_ColorArray[i].r = rand() % 256;
		m_ColorArray[i].g = rand() % 256;
		m_ColorArray[i].b = rand() % 256;
	}

	m_VertBuff = new vector3[8];
	m_VertBuffTransformed = new vector3[8];
	m_IndexBuff = new UINT[36];

	memcpy(m_VertBuff, vert_buff_temp, 8 * sizeof(vector3));
	memcpy(m_IndexBuff, index_buff_temp, 36 * sizeof(DWORD));
}

void CMeshManager::Calculate_Cube()
{
	static float Angle = 0.0f;;

	matrix4x4 MxRotateX={
		1, 0, 0, 0,
        0, cosf(Angle), sinf(Angle), 0,
        0, -sinf(Angle), cosf(Angle), 0,
        0, 0, 0, 1 };
	
	matrix4x4 MxRotateY={
		cosf(Angle), 0, -sinf(Angle), 0,
		0, 1, 0, 0,
		sinf(Angle), 0, cosf(Angle), 0,
		0, 0, 0, 1};

	matrix4x4 MxRotateZ={
		cosf(Angle), sinf(Angle), 0, 0,
		-sinf(Angle), cosf(Angle), 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1};
	
	Angle += PI / 100.0f;
    if(Angle> PI * 2.0f)
		Angle = 0;
	
	//при помощи этой матрицы можно
	//мен€ть положение куба на сцене
	matrix4x4 MxWorld={
		1, 0, 0, 0,
		0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 18, 1 };


	float Fov = PI / 2.0f; // FOV 90 degree
    float Aspect = (float) m_ViewWidth / m_ViewHeight;
	float ZFar = 100.0f;
	float ZNear = 0.1f;

	float h, w, Q;

    w = (1.0f/tanf(Fov*0.5f))/Aspect;  
    h = 1.0f/tanf(Fov*0.5f);   
    Q = ZFar/(ZFar - ZNear);

	/*
	//полный расчет матрицы проекции
    matrix4x4 MxProj = {
		w, 0, 0, 0,
        0, h, 0, 0,
        0, 0, Q, 1,
        0, 0, -Q*ZNear, 0 };
	*/

	matrix4x4 MxProj={
		w, 0, 0, 0,
        0, h, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1 };

	float Alpha = 0.5f*m_ViewWidth;
	float Beta  = 0.5f*m_ViewHeight;
	
	matrix4x4 MxScreen = {
		Alpha,  0,	    0,    0, 
		0,      -Beta,  0,    0, 
		0,		0,		1,    0,
		Alpha,  Beta,	0,    1};

	for (UINT i = 0; i < m_VertCount; i++)
	{
		matrix4x4 MxTemp1, MxTemp2;
		Mat4x4_Mat4x4_Mul(MxTemp1, MxRotateX, MxRotateY);
		Mat4x4_Mat4x4_Mul(MxTemp2, MxTemp1, MxRotateZ);
		Mat4x4_Mat4x4_Mul(MxTemp1, MxTemp2, MxWorld);
		Mat4x4_Mat4x4_Mul(MxTemp2, MxTemp1, MxProj);

		vector3 Vec1;
		Vec3_Mat4x4_Mul(Vec1, m_VertBuff[i], MxTemp2);

		Vec1.x = Vec1.x / Vec1.z;
		Vec1.y = Vec1.y / Vec1.z;

		vector3 Vec2;
		Vec3_Mat4x4_Mul(Vec2, Vec1, MxScreen);

		m_VertBuffTransformed[i] = Vec2;
	}
}

void CMeshManager::Draw_Color_Poly(int y1, int y2)
{
	
	for ( int yi = y1; yi < y2; yi++ )
	{
		for (int xi=(int)m_xl; xi<(int)m_xr; xi++)
		{
			int indx =  yi * 4 * m_ViewWidth + xi * 4;
			
			m_Data[indx] = m_Color.b; // blue
			m_Data[indx + 1] = m_Color.g; // green
			m_Data[indx + 2] = m_Color.r; // red
			m_Data[indx + 3] = 0; 
		}

		m_xl+=m_dxl;
		m_xr+=m_dxr;
	}
}

void CMeshManager::Draw_Color_Triangle(float x1,float y1,
					   float x2,float y2,
					   float x3,float y3)
{
	float Temp;
	int Side;

	if (y2 < y1)
	{
		SWAP(x2,x1,Temp);
		SWAP(y2,y1,Temp);
	}

	if (y3 < y1)
	{
		SWAP(x3,x1,Temp);
		SWAP(y3,y1,Temp);
		
	}

	if (y3 < y2)
	{
		
		SWAP(x3,x2,Temp);
		SWAP(y3,y2,Temp);
		
	}

	if (y2 > y1 && y3 > y2)
	{
		float dxdy1 = (x2 - x1) / (y2 - y1);
		float dxdy2 = (x3 - x1) / (y3 - y1);
		Side = dxdy2 > dxdy1;
	}

	if (y1 == y2)
		Side = x1 > x2;
	if (y2 == y3)
		Side = x3 > x2;

	if (!Side) //если Side = 0 лева€ сторона длиннее
	{
		m_xl = x1;
		m_dxl = (x3 - x1) / (y3 - y1);

		if ( y1 < y2) //треугольник с плоским низом x3 < x2
		{
			m_xr = x1;
			m_dxr = (x2 - x1) / (y2 - y1);

			Draw_Color_Poly((int)y1, (int)y2);	
		}

		if(y2 < y3) //треугольник с плоским верхом x1 < x2
		{
			m_xr = x2;
			m_dxr = (x3 - x2) / (y3 - y2);

			Draw_Color_Poly((int)y2, (int)y3);
		}
	}
	else
	{
		m_xr = x1;
		m_dxr = (x3 - x1) / (y3 - y1);
		
		if (y1 < y2) //треугольинк с плоским низом x3 > x2
		{
			m_xl = x1;
			m_dxl = (x2 - x1) / (y2 - y1);

			Draw_Color_Poly((int)y1, (int)y2);
	
		}
		if (y2 < y3) //треугольник с плоским верхом x1 > x2
		{
			m_xl = x2;
			m_dxl = (x3 - x2) / (y3 - y2);

			Draw_Color_Poly((int)y2, (int)y3);
		}
	}
}

void CMeshManager::Draw_Cube()
{
	 Clear_BackBuffer();

    for (UINT i = 0; i < m_TriangleCount; i++)
	 {
		vector3 Vec1 = m_VertBuffTransformed[m_IndexBuff[i * 3]];
		vector3 Vec2 = m_VertBuffTransformed[m_IndexBuff[i * 3 + 1]];
		vector3 Vec3 = m_VertBuffTransformed[m_IndexBuff[i * 3 + 2]];
		
		//дл€ отбрасывани€ задних поверхностей
		//используем нормаль к треугольнику
		vector3 Edge1 = Vec2 - Vec1;
		vector3 Edge2 = Vec3 - Vec1;

		Vec3_Normalize(Edge1, Edge1);
		Vec3_Normalize(Edge2, Edge2);

		vector3 Cross;
		Vec3_Cross(Cross, Edge2, Edge1);
		Vec3_Normalize(Cross, Cross);

		vector3 Look = { 0.0f, 0.0f, -1.0f };

		float Dot = Vec3_Dot(Cross, Look);
		
		if(Dot <=0.0) //backface culling
			continue;

		m_Color = m_ColorArray[i];

		Draw_Color_Triangle(Vec1.x, Vec1.y, Vec2.x, Vec2.y, Vec3.x, Vec3.y);
     }  

	Present_BackBuffer();
}

void CMeshManager::Create_BackBuffer()
{
	RECT rc;
	GetClientRect(m_hWnd, &rc);

	m_ViewWidth = rc.right;
	m_ViewHeight = rc.bottom;
	
	DWORD m_dwSize = rc.right * (BITS_PER_PIXEL >> 3) * rc.bottom;

	m_Data = (LPBYTE)malloc(m_dwSize*sizeof(BYTE));

	memset(&m_Bih, 0, sizeof(BITMAPINFOHEADER));
	m_Bih.biSize = sizeof(BITMAPINFOHEADER);
	m_Bih.biWidth = rc.right;
	m_Bih.biHeight = rc.bottom;
	m_Bih.biPlanes = 1;
	m_Bih.biBitCount = BITS_PER_PIXEL;
	m_Bih.biCompression = BI_RGB;
	m_Bih.biSizeImage = m_dwSize;
	
	m_hDD = DrawDibOpen();
}

void CMeshManager::Delete_BackBuffer()
{
	DrawDibClose(m_hDD);

	free(m_Data);
	m_Data = NULL;
}

void CMeshManager::Clear_BackBuffer()
{
	for ( UINT i = 0; i <  m_ViewHeight; i++)
	{
		for ( UINT j = 0; j < m_ViewWidth; j++ )
		{
			int indx = i * 4 * m_ViewWidth + j * 4;

			m_Data[indx] = (BYTE) (255.0 * 0.3f); // blue
			m_Data[indx + 1] = (BYTE) (255.0 * 0.125f); // green
			m_Data[indx + 2] = 0; // red

			m_Data[indx + 3] = 0; 
		}
	}
}

void CMeshManager::Present_BackBuffer( )
{ 
	//перед выводом правильно было бы
	//написать код и перевернуть изображение
	//но у нас просто куб и значени€ не имеет
	HDC hDC = GetDC(m_hWnd);
	DrawDibDraw(m_hDD, hDC, 0, 0, m_ViewWidth, m_ViewHeight, &m_Bih, m_Data, 0, 0, m_ViewWidth, m_ViewHeight, 0);
	ReleaseDC(m_hWnd, hDC);
}



