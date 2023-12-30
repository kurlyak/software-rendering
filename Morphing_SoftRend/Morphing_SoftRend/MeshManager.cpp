//======================================================================================
// Ed Kurlyak 2023 Morphing Animation Software
//======================================================================================

#include "MeshManager.h"

CMeshManager::CMeshManager()
{
};

CMeshManager::~CMeshManager()
{
	Delete_BackBuffer();

	delete [] CubeVertsArray;
	delete [] SphereVertsArray;
	delete [] PyramidVertsArray;
	delete [] ResultVertsArray;

	delete [] m_VertBuffTransformed;

	delete [] ColorArray;
	delete [] IndicesArray;
}

vector3 CMeshManager::Vec3_Mat4x4_Mul(vector3& VecIn, matrix4x4 MatIn)
{
	vector3 VecOut;

	VecOut.x =	VecIn.x * MatIn.Mat[M00] +
				VecIn.y * MatIn.Mat[M10] +
				VecIn.z * MatIn.Mat[M20] +
						MatIn.Mat[M30];

	VecOut.y =	VecIn.x * MatIn.Mat[M01] +
				VecIn.y * MatIn.Mat[M11] +
				VecIn.z * MatIn.Mat[M21] +
						MatIn.Mat[M31];

	VecOut.z =	VecIn.x * MatIn.Mat[M02] +
				VecIn.y * MatIn.Mat[M12] +
				VecIn.z * MatIn.Mat[M22] +
						MatIn.Mat[M32];

	return VecOut;
}

void CMeshManager::Build_Side(vector3 * VertBuff, unsigned int * Indices)
{
	float YVal = (m_NumCellsPerRow * m_CellSpacing) / 2.0f;

	float Width = m_NumCellsPerRow * m_CellSpacing;
	float Depth = m_NumCellsPerCol * m_CellSpacing;

	float HalfWidth = 0.5f * Width;
	float HalfDepth = 0.5f * Depth;

	for (UINT i = 0; i < m_NumVertsPerCol; ++i)
	{
		float z = HalfDepth - i * m_CellSpacing;

		for (UINT j = 0; j < m_NumVertsPerRow; ++j)
		{
			float x = -HalfWidth + j * m_CellSpacing;

			float y = YVal;

			int Index = i * m_NumVertsPerRow + j;

			VertBuff[Index].x = x;
			VertBuff[Index].y = y;
			VertBuff[Index].z = z;
		}
	}

	int Index = 0;

	for (UINT i = 0; i < m_NumCellsPerCol; ++i)
	{
		for (UINT j = 0; j < m_NumCellsPerRow; ++j)
		{
			Indices[Index] = i * m_NumVertsPerRow + j;
			Indices[Index + 1] = i * m_NumVertsPerRow + j + 1;
			Indices[Index + 2] = (i + 1) * m_NumVertsPerRow + j;

			Indices[Index + 3] = (i + 1) * m_NumVertsPerRow + j;
			Indices[Index + 4] = i * m_NumVertsPerRow + j + 1;
			Indices[Index + 5] = (i + 1) * m_NumVertsPerRow + j + 1;

			Index += 6;
		}
	}
}

matrix4x4 CMeshManager::MatrixRotationX(float Angle)
{
	matrix4x4 MatRotateX = {
		1, 0, 0, 0,
        0, cosf(Angle), sinf(Angle), 0,
        0, -sinf(Angle), cosf(Angle), 0,
        0, 0, 0, 1};

	return MatRotateX;
}

matrix4x4 CMeshManager::MatrixRotationZ(float Angle)
{
	matrix4x4 MatRotateZ = {
		cosf(Angle), sinf(Angle), 0, 0,
		-sinf(Angle), cosf(Angle), 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1};

	return MatRotateZ;
}

void CMeshManager::Init_MeshManager(HWND hWnd)
{
	m_hWnd = hWnd;

	Create_BackBuffer();

	//************************************
	//MAKE CUBE
	//************************************

	m_NumCellsPerRow = 25;
	m_NumCellsPerCol = 25;
	m_CellSpacing = 2.0;

	m_NumVertsPerRow = m_NumCellsPerRow + 1;
	m_NumVertsPerCol = m_NumCellsPerCol + 1;

	//треугольников и вершин в одной стороне, сторон 6
	UINT NumTriangles = m_NumCellsPerRow * m_NumCellsPerCol * 2;
	UINT NumVert = m_NumVertsPerRow * m_NumVertsPerCol;

	//всего треугольников и вершин в кубе
	m_TriangleCount = m_NumCellsPerRow * m_NumCellsPerCol * 2 * 6;
	m_VertCount = m_NumVertsPerRow * m_NumVertsPerCol * 6;

	//общие массивы
	CubeVertsArray = new vector3[m_VertCount];
	m_VertBuffTransformed = new vector3[m_VertCount];
	ColorArray = new color_rgb[m_VertCount];
	IndicesArray = new unsigned int[m_TriangleCount * 3];

	unsigned int * Indices = new unsigned int[NumTriangles * 3];

	vector3 * VertBuffTop = new vector3[NumVert];
	vector3 * VertBuffBottom = new vector3[NumVert];
	vector3 * VertBuffFront = new vector3[NumVert];
	vector3 * VertBuffBack = new vector3[NumVert];
	vector3 * VertBuffLeft = new vector3[NumVert];
	vector3 * VertBuffRight = new vector3[NumVert];

	//строим верхнюю сторону куба и переворачиваем
	//ее создавая другие стороны
	Build_Side(VertBuffTop, Indices);

	matrix4x4 MxRotate;
	vector3 VecTemp;

	//нижняя сторона
	MxRotate = MatrixRotationX(PI);
	for (UINT i = 0; i < NumVert; i++)
	{
		VecTemp = Vec3_Mat4x4_Mul(VertBuffTop[i], MxRotate);
		VertBuffBottom[i] = vector3(VecTemp.x, VecTemp.y, VecTemp.z );
	}

	//передняя сторона
	MxRotate = MatrixRotationX(-PI/2.0f);
	for (UINT i = 0; i < NumVert; i++)
	{
		VecTemp = Vec3_Mat4x4_Mul(VertBuffTop[i], MxRotate);
		VertBuffFront[i] = vector3(VecTemp.x, VecTemp.y, VecTemp.z );
	}

	//задняя сторона
	MxRotate = MatrixRotationX(PI/2.0f);
	for (UINT i = 0; i < NumVert; i++)
	{
		VecTemp = Vec3_Mat4x4_Mul(VertBuffTop[i], MxRotate);
		VertBuffBack[i] = vector3(VecTemp.x, VecTemp.y, VecTemp.z );
	}

	//левая сторона
	MxRotate = MatrixRotationZ(PI/2.0f);
	for (UINT i = 0; i < NumVert; i++)
	{
		VecTemp = Vec3_Mat4x4_Mul(VertBuffTop[i], MxRotate);
		VertBuffLeft[i] = vector3(VecTemp.x, VecTemp.y, VecTemp.z );
	}

	//правая сторона
	MxRotate = MatrixRotationZ(-PI/2.0f);
	for (UINT i = 0; i < NumVert; i++)
	{
		VecTemp = Vec3_Mat4x4_Mul(VertBuffTop[i], MxRotate);
		VertBuffRight[i] = vector3(VecTemp.x, VecTemp.y, VecTemp.z );
	}

	int Index = 0;

	for (UINT i = 0; i < NumVert; i++)
	{
		CubeVertsArray[Index].x = VertBuffTop[i].x;
		CubeVertsArray[Index].y = VertBuffTop[i].y;
		CubeVertsArray[Index].z = VertBuffTop[i].z;

		ColorArray[Index] = color_rgb(0.811765f * 255.0f, 0.0f, 0.933333f * 255.0f);

		Index++;
	}

	for (UINT i = 0; i < NumVert; i++)
	{
		CubeVertsArray[Index].x = VertBuffBottom[i].x;
		CubeVertsArray[Index].y = VertBuffBottom[i].y;
		CubeVertsArray[Index].z = VertBuffBottom[i].z;

		ColorArray[Index] = color_rgb(0.396078f * 255.0f, 0.250980f * 255.0f, 0.0f);

		Index++;
	}

	for (UINT i = 0; i < NumVert; i++)
	{

		CubeVertsArray[Index].x = VertBuffFront[i].x;
		CubeVertsArray[Index].y = VertBuffFront[i].y;
		CubeVertsArray[Index].z = VertBuffFront[i].z;

		ColorArray[Index] = color_rgb(0.184314f * 255.0f, 0.274510f * 255.0f, 0.662745f * 255.0f);

		Index++;
	}

	for (UINT i = 0; i < NumVert; i++)
	{

		CubeVertsArray[Index].x = VertBuffBack[i].x;
		CubeVertsArray[Index].y = VertBuffBack[i].y;
		CubeVertsArray[Index].z = VertBuffBack[i].z;

		ColorArray[Index] = color_rgb(0.898039f * 255.0f, 0.980392f * 255.0f, 0.0f);

		Index++;
	}

	for (UINT i = 0; i < NumVert; i++)
	{

		CubeVertsArray[Index].x = VertBuffLeft[i].x;
		CubeVertsArray[Index].y = VertBuffLeft[i].y;
		CubeVertsArray[Index].z = VertBuffLeft[i].z;

		ColorArray[Index] = color_rgb(0.662745f * 255.0f, 0.078431f * 255.0f, 0.0f);

		Index++;
	}

	for (UINT i = 0; i < NumVert; i++)
	{

		CubeVertsArray[Index].x = VertBuffRight[i].x;
		CubeVertsArray[Index].y = VertBuffRight[i].y;
		CubeVertsArray[Index].z = VertBuffRight[i].z;

		ColorArray[Index] = color_rgb(0.070588f * 255.0f, 0.592157f * 255.0f, 0.0f);

		Index++;
	}

	delete [] VertBuffTop;
	delete [] VertBuffBottom;
	delete [] VertBuffFront;
	delete [] VertBuffBack;
	delete [] VertBuffLeft;
	delete [] VertBuffRight;

	Index = 0;

	for (UINT j =0; j < 6; j++)
	{
		for (UINT i = 0; i < NumTriangles * 3; i++)
		{
			IndicesArray[Index] = Indices[i] + NumVert * j;
			Index++;
		}
	}

	//************************************
	//MAKE SPHERE
	//************************************

	SphereVertsArray = new vector3[m_VertCount];
	//как делаем сферу
	//каждую вершину нормализуем и умножаем на радиус
	//радису сферы это от центра координат до стороны
	//(не до угла куба- до стороны)
	for( UINT i = 0; i < m_VertCount; i++)
	{
		//вычисляем длинну вектора до вершины
		float Len = sqrtf(CubeVertsArray[i].x * CubeVertsArray[i].x +
			CubeVertsArray[i].y * CubeVertsArray[i].y +
			CubeVertsArray[i].z * CubeVertsArray[i].z);
		
		//нормализуем и умножаем на радиус 25 
		//25 это от центра до одной стороны куба
		SphereVertsArray[i].x = CubeVertsArray[i].x / Len * 25.0f;
		SphereVertsArray[i].y = CubeVertsArray[i].y / Len * 25.0f;
		SphereVertsArray[i].z = CubeVertsArray[i].z / Len * 25.0f;
	}

	//************************************
	//MAKE PYRAMID
	//************************************

	PyramidVertsArray = new vector3[m_VertCount];

	//как делаем пирамиду
	//общая высота куба 50 (от центра вниз 25 и вверх 25)
	//мы y каждой вершины координату Y переводим от 0 до 50
	//затем вычисляем коофициент y / 50
	//коофициент будет от 0 до 1
	//затем x,z каждой вершины умножаем на этот коэфициент
	for( UINT i = 0; i < m_VertCount; i++)
	{

		float y = 0;

		//если y вершины меньше 0
		//т.е. вершина ниже центра координат
		//а центр куба в центре координат
		//переводим y вершины от 0 до 50
		if( CubeVertsArray[i].y < 0)
			y = 25.0f + fabsf(CubeVertsArray[i].y);

		//переводим y вершины от 0 до 50
		if( CubeVertsArray[i].y > 0)
			y = 25.0f - CubeVertsArray[i].y;

		//если вершина на уровне центра коорданат
		//то y = 25
		if( CubeVertsArray[i].y == 0)
			y = 25;

		//вычисляем коофициент
		float Fac = y / 50.0f;

		//умножаем x и z каждой вершины на этот коэфициент
		//что бы уменьшить значение x и z
		PyramidVertsArray[i].x = CubeVertsArray[i].x * Fac ;
		PyramidVertsArray[i].y = CubeVertsArray[i].y;
		PyramidVertsArray[i].z = CubeVertsArray[i].z * Fac;
	}

	//**********************
	//CREATE RESULT VERT BUFF
	//**********************

	ResultVertsArray = new vector3[m_VertCount];

}

void CMeshManager::Update(vector3 * VecIn1, vector3 * VecIn2, float Scalar)
{
	for( UINT i=0; i < m_VertCount; i++ )
    {
		vector3 VecSrcP = VecIn1[i];
		vector3 VecDstP = VecIn2[i];

		float InvScalar = 1.0f - Scalar;
		VecSrcP= VecSrcP * InvScalar;
		VecDstP= VecDstP * Scalar;

		ResultVertsArray[i] = VecSrcP + VecDstP;
	}
}

void CMeshManager::Update_MeshManager()
{
	float BlendWeight;

	static UINT CurrBlend = 0;

	static float Blend = 0.0f;
	Blend += 0.0075f;

	//считаем до 0 до 1 BlendWeight
	if(Blend < 1.0f)
	{
		BlendWeight = Blend;
	}
	else
	{
		//делаем паузу после перехода
		//анимации оставляем мешь в позиции интерполяции 1.0
		BlendWeight = 1.0;

		//если пауза закончилась т.е. больше 3.0
		//обнуляем BlendWeight
		//переключаем на следующую анимацию CurrBlend++
		if(Blend > 3.0f)
		{
			BlendWeight = 0.0;
			Blend = 0.0f;
			CurrBlend++;
		}

		//если закончились все анимации
		//начинаем сначала
		if(CurrBlend > 3)
			CurrBlend = 0;
	}

	//первая анимация куб-сфера
	if(CurrBlend == 0)
	{
		Update(CubeVertsArray, SphereVertsArray, BlendWeight);
	}
	//вторая анимация сфера-пирамида
	else if(CurrBlend == 1)
	{
		Update(SphereVertsArray, PyramidVertsArray, BlendWeight);
	}
	//третья анимация пирамида-сфера
	else if(CurrBlend == 2)
	{
		Update(PyramidVertsArray, SphereVertsArray, BlendWeight);
	}
	//четвертая анимация сфера-куб
	else if(CurrBlend == 3)
	{
		Update(SphereVertsArray, CubeVertsArray, BlendWeight);
	}

	//рисуем результирующий мешь после интерполяции

	static float Angle = 0.0f;;

	matrix4x4 MatRotateX = {
		1, 0, 0, 0,
        0, cosf(Angle), sinf(Angle), 0,
        0, -sinf(Angle), cosf(Angle), 0,
        0, 0, 0, 1 };
	
	matrix4x4 MatRotateY = {
		cosf(Angle), 0, -sinf(Angle), 0,
		0, 1, 0, 0,
		sinf(Angle), 0, cosf(Angle), 0,
		0, 0, 0, 1 };

	matrix4x4 MatRotateZ = {
		cosf(Angle), sinf(Angle), 0, 0,
		-sinf(Angle), cosf(Angle), 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1 };
	
	Angle+= PI / 150.0f;
    if(Angle > PI2)
		Angle = 0;
	
	//при помощи этой матрицы можно
	//перемещать модель по сцене
	matrix4x4 MatWorld={
		1, 0, 0, 0,
		0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 80, 1 };

	float Fov = PI/2.0f; // FOV 90 degree
    float Aspect = (float)m_ViewWidth / m_ViewHeight;
	float ZFar = 100.0f;
	float ZNear = 0.1f;

	float h, w, Q;

    w = (1.0f/tanf(Fov*0.5f))/Aspect;  
    h = 1.0f/tanf(Fov*0.5f);   
    Q = ZFar/(ZFar - ZNear);

	/*
	//полный расчет матрицы проекции
    matrix4x4 MatProj={
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

	float Alpha = 0.5f * m_ViewWidth;
	float Beta = 0.5f * m_ViewHeight;

	matrix4x4 MatScreen = {
		Alpha,  0,	    0,    0,
		0,      -Beta,  0,    0,
		0,		0,		1,    0,
		Alpha,  Beta,	0,    1 };

	for (UINT i = 0; i < m_VertCount; i++)
	{
		
		vector3 VecTemp = Vec3_Mat4x4_Mul(ResultVertsArray[i], MatRotateY);
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

void CMeshManager::Draw_Color_Poly(int y1, int y2, color_rgb Color)
{
	for ( int y = y1; y < y2; y++ )
	{
		for (int x= (int)m_xl; x< (int)m_xr; x++)
		{
			int Index =  y * 4 * m_ViewWidth + x * 4;
			
			m_Data[Index + 0] = Color.b; // blue
			m_Data[Index + 1] = Color.g; // green
			m_Data[Index + 2] = Color.r; // red
			m_Data[Index + 3] = 0; 
		}

		m_xl+=m_dxl;
		m_xr+=m_dxr;
	}
}

void CMeshManager::Draw_Color_Triangle(float x1,float y1,
					   float x2,float y2,
					   float x3,float y3,
					   color_rgb Color)
{
	float Temp;
	int Side = 0;

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

	int y1i = (int) floor (y1);
	int y2i = (int) floor (y2);
	int y3i = (int) floor (y3);

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

	float dyl, dyr;

	if (!Side)//если Side = 0 левая сторона длиннее
	{
		dyl = y3 - y1;
		m_dxl = (x3 - x1) / dyl;
		
		m_xl = x1;
		
		if ( y1 < y2) //треугольник с плоским низом x3 < x2
		{
			dyr = y2 - y1;
			m_dxr = (x2 - x1) / dyr;

			m_xr = x1;

			Draw_Color_Poly(y1i, y2i, Color);	
		}

		if(y2 < y3) //треугольник с плоским верхом x1 < x2
		{
			dyr = y3 - y2;
			m_dxr = (x3 - x2) / dyr;

			m_xr = x2;

			Draw_Color_Poly(y2i, y3i, Color);
		}
	}
	else
	{
		dyr = y3 - y1;
		m_dxr = (x3 - x1) / dyr;
		
		m_xr = x1;
		
		if (y1 < y2) //треугольинк с плоским низом x3 > x2
		{
			dyl = y2 - y1;
			m_dxl = (x2 - x1) / dyl;

			m_xl = x1;

			Draw_Color_Poly(y1i, y2i, Color);
	
		}
		if (y2 < y3) //треугольник с плоским верхом x1 > x2
		{
	
			dyl = y3 - y2;
			m_dxl = (x3 - x2) / dyl;
			
			m_xl = x2;

			Draw_Color_Poly(y2i, y3i, Color);
		}
	}
}

void CMeshManager::Draw_MeshManager()
{
	 Clear_BackBuffer();

    for (UINT i = 0; i < m_TriangleCount; i++)
	 {
		vector3 Vec1 = m_VertBuffTransformed[IndicesArray[i * 3 + 0]];
		vector3 Vec2 = m_VertBuffTransformed[IndicesArray[i * 3 + 1]];
		vector3 Vec3 = m_VertBuffTransformed[IndicesArray[i * 3 + 2]];

		//используем псевдоскалярное (косое) умножение векторов
		//для отбрасывания задних поверхностей
		float s = (Vec2.x - Vec1.x) * (Vec3.y - Vec1.y) - (Vec2.y - Vec1.y) * (Vec3.x - Vec1.x);

		if (s <= 0)
			continue;
		
		//цвета для всех 3х вершин одинаковые
		//поэтому берем только цвет 1й вершины
		color_rgb Color = ColorArray[IndicesArray[i * 3]];

		Draw_Color_Triangle(Vec1.x, Vec1.y, Vec2.x, Vec2.y, Vec3.x, Vec3.y, Color);
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

void CMeshManager::Present_BackBuffer()
{ 
	//правильно было бы перевернуть backbuffer
	HDC hDC = GetDC(m_hWnd);
	DrawDibDraw(m_hDD, hDC, 0, 0, m_ViewWidth, m_ViewHeight, &m_Bih, m_Data, 0, 0, m_ViewWidth, m_ViewHeight, 0);
	ReleaseDC(m_hWnd, hDC);
}


