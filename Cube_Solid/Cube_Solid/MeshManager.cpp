//======================================================================================
//	Ed Kurlyak 2023 Color Cube
//======================================================================================

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

vector3 CMeshManager::Vec3_Mat4x4_Mul(vector3& VecIn, matrix4x4 MatIn)
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

void CMeshManager::Init_MeshManager(HWND hWnd)
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

void CMeshManager::Update_MeshManager()
{
	static float Angle = 0.0f;;

	matrix4x4 MatRotateX={
		1, 0, 0, 0,
        0, cosf(Angle), sinf(Angle), 0,
        0, -sinf(Angle), cosf(Angle), 0,
        0, 0, 0, 1 };
	
	matrix4x4 MatRotateY={
		cosf(Angle), 0, -sinf(Angle), 0,
		0, 1, 0, 0,
		sinf(Angle), 0, cosf(Angle), 0,
		0, 0, 0, 1};

	matrix4x4 MatRotateZ={
		cosf(Angle), sinf(Angle), 0, 0,
		-sinf(Angle), cosf(Angle), 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1};
	
	Angle += PI / 100.0f;
    if(Angle> PI * 2.0f)
		Angle = 0;
	
	//при помощи этой матрицы можно
	//менять положение куба на сцене
	matrix4x4 MatWorld={
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
    matrix4x4 MatProj = {
		w, 0, 0, 0,
        0, h, 0, 0,
        0, 0, Q, 1,
        0, 0, -Q*ZNear, 0 };
	*/

	matrix4x4 MatProj={
		w, 0, 0, 0,
        0, h, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1 };

	float Alpha = 0.5f*m_ViewWidth;
	float Beta  = 0.5f*m_ViewHeight;
	
	matrix4x4 MatScreen = {
		Alpha,  0,	    0,    0, 
		0,      -Beta,  0,    0, 
		0,		0,		1,    0,
		Alpha,  Beta,	0,    1};

	for (UINT i = 0; i < m_VertCount; i++)
	{

		vector3 VecTemp = Vec3_Mat4x4_Mul(m_VertBuff[i], MatRotateY);
		VecTemp = Vec3_Mat4x4_Mul(VecTemp, MatRotateX);
		VecTemp = Vec3_Mat4x4_Mul(VecTemp, MatRotateZ);
		VecTemp = Vec3_Mat4x4_Mul(VecTemp, MatWorld);
		VecTemp = Vec3_Mat4x4_Mul(VecTemp, MatProj);

		VecTemp.x = VecTemp.x / VecTemp.z;
		VecTemp.y = VecTemp.y / VecTemp.z;

		VecTemp = Vec3_Mat4x4_Mul(VecTemp, MatScreen);

		m_VertBuffTransformed[i] = VecTemp;
	}
}

void CMeshManager::Draw_Color_Poly(int y1, int y2)
{
	
	for ( int y = y1; y < y2; y++ )
	{
		for (int x=(int)m_xl; x<(int)m_xr; x++)
		{
			int Index =  y * 4 * m_ViewWidth + x * 4;
			
			m_Data[Index + 0] = m_Color.b; // blue
			m_Data[Index + 1] = m_Color.g; // green
			m_Data[Index + 2] = m_Color.r; // red
			m_Data[Index + 3] = 0; 
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

	if (!Side) //если Side = 0 левая сторона длиннее
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

void CMeshManager::Draw_MeshManager()
{
	 Clear_BackBuffer();

    for (UINT i = 0; i < m_TriangleCount; i++)
	 {
		vector3 Vec1 = m_VertBuffTransformed[m_IndexBuff[i * 3 + 0]];
		vector3 Vec2 = m_VertBuffTransformed[m_IndexBuff[i * 3 + 1]];
		vector3 Vec3 = m_VertBuffTransformed[m_IndexBuff[i * 3 + 2]];

		float s = (Vec2.x - Vec1.x) * (Vec3.y - Vec1.y) - (Vec2.y - Vec1.y) * (Vec3.x - Vec1.x);

		if (s <= 0)
			continue;

		m_Color = m_ColorArray[i];

		Draw_Color_Triangle(Vec1.x, Vec1.y, Vec2.x, Vec2.y, Vec3.x, Vec3.y);
     }  

	Present_BackBuffer();
}

void CMeshManager::Create_BackBuffer()
{
	RECT Rc;
	GetClientRect(m_hWnd, &Rc);

	m_ViewWidth = Rc.right;
	m_ViewHeight = Rc.bottom;
	
	DWORD m_dwSize = m_ViewWidth * (BITS_PER_PIXEL >> 3) * m_ViewHeight;

	m_Data = (LPBYTE)malloc(m_dwSize*sizeof(BYTE));

	memset(&m_Bih, 0, sizeof(BITMAPINFOHEADER));
	m_Bih.biSize = sizeof(BITMAPINFOHEADER);
	m_Bih.biWidth = m_ViewWidth;
	m_Bih.biHeight = m_ViewHeight;
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
			int Index = i * 4 * m_ViewWidth + j * 4;

			m_Data[Index + 0] = (BYTE) (255.0 * 0.3f); // blue
			m_Data[Index + 1] = (BYTE) (255.0 * 0.125f); // green
			m_Data[Index + 2] = 0; // red
			m_Data[Index + 3] = 0; 
		}
	}
}

void CMeshManager::Present_BackBuffer( )
{ 
	//перед выводом правильно было бы
	//написать код и перевернуть изображение
	//но у нас просто куб и значения не имеет
	HDC hDC = GetDC(m_hWnd);
	DrawDibDraw(m_hDD, hDC, 0, 0, m_ViewWidth, m_ViewHeight, &m_Bih, m_Data, 0, 0, m_ViewWidth, m_ViewHeight, 0);
	ReleaseDC(m_hWnd, hDC);
}



