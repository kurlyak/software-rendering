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

void CMeshManager::Mat4x4_Mat4x4_Mul(matrix4x4 MatOut, matrix4x4 Mat1, matrix4x4 Mat2)
{
	//row1 * col1
	MatOut[M00] = Mat1[M00] * Mat2[M00] + Mat1[M01] * Mat2[M10] + Mat1[M02] * Mat2[M20] + Mat1[M03] * Mat2[M30];
	//row1 * col2
	MatOut[M01] = Mat1[M00] * Mat2[M01] + Mat1[M01] * Mat2[M11] + Mat1[M02] * Mat2[M21] + Mat1[M03] * Mat2[M31];
	//row1 * col3
	MatOut[M02] = Mat1[M00] * Mat2[M02] + Mat1[M01] * Mat2[M12] + Mat1[M02] * Mat2[M22] + Mat1[M03] * Mat2[M32];
	//row1 * col4
	MatOut[M03] = Mat1[M00] * Mat2[M03] + Mat1[M01] * Mat2[M13] + Mat1[M02] * Mat2[M23] + Mat1[M03] * Mat2[M33];

	//row2 * col1
	MatOut[M10] = Mat1[M10] * Mat2[M00] + Mat1[M11] * Mat2[M10] + Mat1[M12] * Mat2[M20] + Mat1[M13] * Mat2[M30];
	//row2 * col2
	MatOut[M11] = Mat1[M10] * Mat2[M01] + Mat1[M11] * Mat2[M11] + Mat1[M12] * Mat2[M21] + Mat1[M13] * Mat2[M31];
	//row2 * col3
	MatOut[M12] = Mat1[M10] * Mat2[M02] + Mat1[M11] * Mat2[M12] + Mat1[M12] * Mat2[M22] + Mat1[M13] * Mat2[M32];
	//row2 * col4
	MatOut[M13] = Mat1[M10] * Mat2[M03] + Mat1[M11] * Mat2[M13] + Mat1[M12] * Mat2[M23] + Mat1[M13] * Mat2[M33];

	//row3 * col1
	MatOut[M20] = Mat1[M20] * Mat2[M00] + Mat1[M21] * Mat2[M10] + Mat1[M22] * Mat2[M20] + Mat1[M23] * Mat2[M30];
	//row3 * col2
	MatOut[M21] = Mat1[M20] * Mat2[M01] + Mat1[M21] * Mat2[M11] + Mat1[M22] * Mat2[M21] + Mat1[M23] * Mat2[M31];
	//row3 * col3
	MatOut[M22] = Mat1[M20] * Mat2[M02] + Mat1[M21] * Mat2[M12] + Mat1[M22] * Mat2[M22] + Mat1[M23] * Mat2[M32];
	//row3 * col4
	MatOut[M23] = Mat1[M20] * Mat2[M03] + Mat1[M21] * Mat2[M13] + Mat1[M22] * Mat2[M23] + Mat1[M23] * Mat2[M33];

	//row4 * col1
	MatOut[M30] = Mat1[M30] * Mat2[M00] + Mat1[M31] * Mat2[M10] + Mat1[M32] * Mat2[M20] + Mat1[M33] * Mat2[M30];
	//row4 * col2
	MatOut[M31] = Mat1[M30] * Mat2[M01] + Mat1[M31] * Mat2[M11] + Mat1[M32] * Mat2[M21] + Mat1[M33] * Mat2[M31];
	//row4 * col3
	MatOut[M32] = Mat1[M30] * Mat2[M02] + Mat1[M31] * Mat2[M12] + Mat1[M32] * Mat2[M22] + Mat1[M33] * Mat2[M32];
	//row4 * col4
	MatOut[M33] = Mat1[M30] * Mat2[M03] + Mat1[M31] * Mat2[M13] + Mat1[M32] * Mat2[M23] + Mat1[M33] * Mat2[M33];
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

void CMeshManager::MatrixRotationX(matrix4x4 MatOut, float Angle)
{
	matrix4x4 MatRotateX = {
		1, 0, 0, 0,
        0, cosf(Angle), sinf(Angle), 0,
        0, -sinf(Angle), cosf(Angle), 0,
        0, 0, 0, 1};

	memcpy(MatOut, MatRotateX, sizeof(matrix4x4));
}

void CMeshManager::MatrixRotationZ(matrix4x4 MatOut, float Angle)
{
	matrix4x4 MatRotateZ = {
		cosf(Angle), sinf(Angle), 0, 0,
		-sinf(Angle), cosf(Angle), 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1};

	memcpy(MatOut, MatRotateZ, sizeof(matrix4x4));
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
	MatrixRotationX(MxRotate, PI);
	for (UINT i = 0; i < NumVert; i++)
	{
		Vec3_Mat4x4_Mul(VecTemp, VertBuffTop[i], MxRotate);
		VertBuffBottom[i] = vector3(VecTemp.x, VecTemp.y, VecTemp.z );
	}

	//передняя сторона
	MatrixRotationX(MxRotate, -PI/2.0f);
	for (UINT i = 0; i < NumVert; i++)
	{
		Vec3_Mat4x4_Mul(VecTemp, VertBuffTop[i], MxRotate);
		VertBuffFront[i] = vector3(VecTemp.x, VecTemp.y, VecTemp.z );
	}

	//задняя сторона
	MatrixRotationX(MxRotate, PI/2.0f);
	for (UINT i = 0; i < NumVert; i++)
	{
		Vec3_Mat4x4_Mul(VecTemp, VertBuffTop[i], MxRotate);
		VertBuffBack[i] = vector3(VecTemp.x, VecTemp.y, VecTemp.z );
	}

	//левая сторона
	MatrixRotationZ(MxRotate, PI/2.0f);
	for (UINT i = 0; i < NumVert; i++)
	{
		Vec3_Mat4x4_Mul(VecTemp, VertBuffTop[i], MxRotate);
		VertBuffLeft[i] = vector3(VecTemp.x, VecTemp.y, VecTemp.z );
	}

	//правая сторона
	MatrixRotationZ(MxRotate, -PI/2.0f);
	for (UINT i = 0; i < NumVert; i++)
	{
		Vec3_Mat4x4_Mul(VecTemp, VertBuffTop[i], MxRotate);
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

void CMeshManager::Update(vector3 * Vec1, vector3 * Vec2, float Scalar)
{
	for( UINT i=0; i < m_VertCount; i++ )
    {
		vector3 VecSrcP = Vec1[i];
		vector3 VecDstP = Vec2[i];

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
    if(Angle > PI*2)
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
		matrix4x4 MatTemp1, MatTemp2;
		Mat4x4_Mat4x4_Mul(MatTemp1, MatRotateX, MatRotateY);
		Mat4x4_Mat4x4_Mul(MatTemp2, MatTemp1, MatRotateZ);
		Mat4x4_Mat4x4_Mul(MatTemp1, MatTemp2, MatWorld);
		Mat4x4_Mat4x4_Mul(MatTemp2, MatTemp1, MatProj);

		vector3 Vec1;
		Vec3_Mat4x4_Mul(Vec1, ResultVertsArray[i], MatTemp2);

		Vec1.x = Vec1.x / Vec1.z;
		Vec1.y = Vec1.y / Vec1.z;

		vector3 Vec2;
		Vec3_Mat4x4_Mul(Vec2, Vec1, MatScreen);

		m_VertBuffTransformed[i] = Vec2;
	}
}

void CMeshManager::Draw_Color_Poly(int y1, int y2, color_rgb Color)
{
	for ( int yi = y1; yi < y2; yi++ )
	{
		int x1 = (int) m_xl;
		int x2 = (int) m_xr;

		for (int xi=x1; xi<x2; xi++)
		{
			int indx =  yi * 4 * m_ViewWidth + xi * 4;
			
			m_Data[indx] = Color.b; // blue
			m_Data[indx + 1] = Color.g; // green
			m_Data[indx + 2] = Color.r; // red
			m_Data[indx + 3] = 0; 
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
		vector3 Vec1 = m_VertBuffTransformed[IndicesArray[i * 3]];
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
	
	DWORD m_dwSize = Rc.right * (BITS_PER_PIXEL >> 3) * Rc.bottom;

	m_Data = (LPBYTE)malloc(m_dwSize*sizeof(BYTE));

	memset(&m_Bih, 0, sizeof(BITMAPINFOHEADER));
	m_Bih.biSize = sizeof(BITMAPINFOHEADER);
	m_Bih.biWidth = Rc.right;
	m_Bih.biHeight = Rc.bottom;
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

void CMeshManager::Present_BackBuffer()
{ 
	//правильно было бы перевернуть backbuffer
	HDC hDC = GetDC(m_hWnd);
	DrawDibDraw(m_hDD, hDC, 0, 0, m_ViewWidth, m_ViewHeight, &m_Bih, m_Data, 0, 0, m_ViewWidth, m_ViewHeight, 0);
	ReleaseDC(m_hWnd, hDC);
}


