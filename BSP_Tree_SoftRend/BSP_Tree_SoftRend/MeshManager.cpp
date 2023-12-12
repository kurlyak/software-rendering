//======================================================================================
//	Ed Kurlyak 2023 BSP Tree Software Triangle Rasterization
//======================================================================================

#include "MeshManager.h"

//**********************************
//Конструктор класса CMeshManager
//**********************************
CMeshManager::CMeshManager()
{
}

//**********************************
//Деструктор класса CMeshManager
//**********************************
CMeshManager::~CMeshManager()
{
	Delete_BackBuffer();

	Delete_BSP(m_Root);

	if (m_LevelTile[0] != NULL)
	{
		delete [] m_LevelTile[0];
		m_LevelTile[0] = NULL;
	}

	if (m_LevelTile[1] != NULL)
	{
		delete[] m_LevelTile[1];
		m_LevelTile[1] = NULL;
	}

	if (m_LevelTile[2] != NULL)
	{
		delete[] m_LevelTile[2];
		m_LevelTile[2] = NULL;
	}

	if(m_LevelTile != NULL)
	{
		delete [] m_LevelTile;
		m_LevelTile = NULL;
	}

	if(m_Polygons.PolyList != NULL)
	{
		free(m_Polygons.PolyList);
		m_Polygons.PolyList = NULL;
	}

	if (m_TexInfo != NULL)
	{
		delete [] m_TexInfo;
		m_TexInfo = NULL;
	}
}

//*************************************************
//Функция нормализует вектор - приводит вектор к
//единичной длинне
//*************************************************
void CMeshManager::Vec4_Normalize(vector4& VecOut, vector4& Vec)
{
	float Len = sqrtf((Vec.x * Vec.x) + (Vec.y * Vec.y) + (Vec.z * Vec.z));

	VecOut.x = Vec.x / Len;
	VecOut.y = Vec.y / Len;
	VecOut.z = Vec.z / Len;
	VecOut.w = 1.0f;
}

//**************************************************************
//Функция возвращает матрицу поворота вокруг заданого вектора
//**************************************************************
matrix4x4 CMeshManager::Matrix_Rotation_Axis(vector4 Vec, float Angle)
{
	float x = Vec.x;
	float y = Vec.y;
	float z = Vec.z;
	
	float s = sinf(Angle);
	float c = cosf(Angle);
	float omc = (float)1.0f - c;

	float xomc = x * omc;
	float yomc = y * omc;
	float zomc = z * omc;

	float xxomc = x * xomc;
	float xyomc = x * yomc;
	float xzomc = x * zomc;
		
	float yyomc = y * yomc;
	float yzomc = y * zomc;
	float zzomc = z * zomc;

	float xs = x * s;
	float ys = y * s;
	float zs = z * s;

	return matrix4x4(xxomc + c,  xyomc + zs, xzomc - ys, 0.0f,
		xyomc - zs, yyomc + c,  yzomc + xs, 0.0f,
		xzomc + ys, yzomc - xs, zzomc + c, 0.0f,
		0.0f ,0.0f, 0.0f, 1.0f);
}

//********************************************
//Функция возвращает следующий треугольник
//из списка треугольников
//********************************************
polygon* list::Get_From_List ()
{
	polygon *Poly;

	Poly = &PolyList[PolygonCurr];
	PolygonCurr++;
	if(PolygonCurr > PolygonCount)
		return NULL;

	return Poly;
}

//****************************************************
//Функция проверяет пустой ли список стреугольников
//или нет
//****************************************************
bool list::Is_Empty_List ()
{
		if(PolygonCount == 0)
			return true;

		return false;
}

//*****************************************
//Функция добавляет треугольник в список
//*****************************************
void list::Add_To_List(polygon *p)
{
	if ( PolygonCount == 0)
	{	
		PolygonCount++;
		PolyList=(polygon*)malloc(PolygonCount*sizeof(polygon));
	}
	else
	{
		PolygonCount++;
		PolyList=(polygon*)realloc(PolyList, PolygonCount*sizeof(polygon));
	}

	for(UINT i=0;i<3;i++)
		PolyList[PolygonCount-1].Vertex[i] = p->Vertex[i];

	PolyList[PolygonCount-1].TexID = p->TexID;
}

//***************************************************
//Функция возвращает цифровой индекс в зависимости
//от имени файла текстуры
//***************************************************
int CMeshManager::Get_TextureID(char * TexName)
{
	if(!_strcmpi(TexName, "texture1.bmp\n"))
	{
		return 0;
	}
	else if(!_strcmpi(TexName, "texture2.bmp\n"))
	{
		return 1;
	}
	else if(!_strcmpi(TexName, "texture3.bmp\n"))
	{
		return 2;
	}
	
	return -1;
}

//*******************************
//Функция инициализирует сцену
//*******************************
void CMeshManager::Init_MeshManager(HWND hWnd)
{
	m_hWnd = hWnd;

	Timer_Start();

	Create_BackBuffer();

	//ширина текстуры 256, высота текстуры 256
	//256 * 256 = 65536, три цвета r,g,b
	//то есть массив каждой текстуры
	//содержит 256 * 256 * 3 элементов
	//всего на сцене у нас 3 текстуры
	//текстура пола, стен, потолка
	m_LevelTile = new unsigned char *[3];

	//информация для каждой текстуры
	//ширина и высота так как текстуры
	//могут быть разного размера
	m_TexInfo = new texture_info[3];

	//загружаем текстуру пола индекс массива 0
	Load_BMP((char*)"texture1.bmp", 0);
	//загружаем текстуру стен индекс массива 1
	Load_BMP((char*)"texture2.bmp", 1);
	//загружаем текстуру потолка индекс массива 2
	Load_BMP((char*)"texture3.bmp", 2);

	//загружаем вершины и имена текстур из файла
	FILE * Fp;
	fopen_s(&Fp,"level.txt", "rt");

	char Buffer[1024];
	fgets(Buffer, 1024, Fp);

	int Size;
	sscanf_s(Buffer,"%d",&Size);
	
	while(!feof(Fp))
	{
		vector4 *VecPos;
		VecPos = new vector4[4];
		
		//читаем из файла 4 вершины полигона
		//из котрых сложим 2 треугольника
		fgets(Buffer, 1024, Fp);
		sscanf_s(Buffer,"%f %f %f %f %f",&VecPos[0].x, &VecPos[0].y, &VecPos[0].z, &VecPos[0].tu, &VecPos[0].tv);
	
		fgets(Buffer, 1024, Fp);
		sscanf_s(Buffer,"%f %f %f %f %f",&VecPos[1].x, &VecPos[1].y, &VecPos[1].z, &VecPos[1].tu, &VecPos[1].tv);
	
		fgets(Buffer, 1024, Fp);
		sscanf_s(Buffer,"%f %f %f %f %f",&VecPos[2].x, &VecPos[2].y, &VecPos[2].z, &VecPos[2].tu, &VecPos[2].tv);
	
		fgets(Buffer, 1024, Fp);
		sscanf_s(Buffer,"%f %f %f %f %f",&VecPos[3].x, &VecPos[3].y, &VecPos[3].z, &VecPos[3].tu, &VecPos[3].tv);
						
		//читаем из файла имя текстуры этого полигона
		fgets(Buffer, 1024, Fp);
		
		//в зависимости от имени текстуры
		//получаем ее индекс в массиве текстур
		int TexId = Get_TextureID(Buffer);

		polygon Poly;

		Poly.Vertex[0] = VecPos[0];
		Poly.Vertex[1] = VecPos[1];
		Poly.Vertex[2] = VecPos[2];

		Poly.Vertex[0].w = 1.0f;
		Poly.Vertex[1].w = 1.0f;
		Poly.Vertex[2].w = 1.0f;

		Poly.TexID = TexId;

		//добавляем треугольник в список
		m_Polygons.Add_To_List(&Poly);

		Poly.Vertex[0] = VecPos[0];
		Poly.Vertex[1] = VecPos[2];
		Poly.Vertex[2] = VecPos[3];

		Poly.Vertex[0].w = 1.0f;
		Poly.Vertex[1].w = 1.0f;
		Poly.Vertex[2].w = 1.0f;

		Poly.TexID = TexId;
		
		//добавляем треугольник в список
		m_Polygons.Add_To_List(&Poly);

		delete[] VecPos;
	}

	fclose(Fp);

	//инициализируем векторы камеры
	m_VecUp = vector4( 0.0, 1.0, 0.0);
	m_VecLook = vector4( 0.0, 0.0, 1.0);

	if(m_VecLook.z > 0)
	{
		m_VecRight = vector4( 1.0, 0.0, 0.0 );
	}

	if(m_VecLook.z < 0)
	{
		m_VecRight = vector4( -1.0, 0.0, 0.0 );
	}

	m_VecPos = vector4( 25.0f, 5.0f, 25.0f);
	
    //Build the BSP Tree from the Root node
	m_Root = new BSP_tree;
    Build_BSP_Tree(m_Root, m_Polygons);
}

//********************************
//Функция создает задний буфер
//********************************
void CMeshManager::Create_BackBuffer()
{
	RECT Rc;
	::GetClientRect(m_hWnd, &Rc);

	m_BackLpitch = Rc.right * 4;

	m_ViewWidth = Rc.right - Rc.left;
	m_ViewHeight = Rc.bottom - Rc.top;

	DWORD Size = m_ViewWidth * (BITS_PER_PIXEL >> 3) * m_ViewHeight;

	m_Data = (LPBYTE)malloc(Size * sizeof(BYTE));
	m_DataTemp = (LPBYTE)malloc(Size * sizeof(BYTE));

	memset(&m_Bih, 0, sizeof(BITMAPINFOHEADER));
	m_Bih.biSize = sizeof(BITMAPINFOHEADER);
	m_Bih.biWidth = m_ViewWidth;
	m_Bih.biHeight = m_ViewHeight;
	m_Bih.biPlanes = 1;
	m_Bih.biBitCount = BITS_PER_PIXEL;
	m_Bih.biCompression = BI_RGB;
	m_Bih.biSizeImage = Size;

	m_hDD = DrawDibOpen();

}

//********************************
//Функция очищает задний буфер
//********************************
void CMeshManager::Clear_BackBuffer()
{
	for (UINT h = 0; h < m_ViewHeight; h++)
	{
		for (UINT w = 0; w < m_ViewWidth; w++)
		{
			int Index = h * 4 * m_ViewWidth + w * 4;

			m_Data[Index] = (BYTE)(255.0 * 0.3f); // blue
			m_Data[Index + 1] = (BYTE)(255.0 * 0.125f); // green
			m_Data[Index + 2] = 0; // red

			m_Data[Index + 3] = 0;
		}
	}
}

//****************************************
//Функция выводит задний буфер на экран
//****************************************
void CMeshManager::Present_BackBuffer()
{
	//переворачиваем задний буфер
	for (UINT h = 0; h < m_ViewHeight; h++ )
	{
		for (UINT w = 0; w < m_ViewWidth; w++)
		{
			int Index = h * 4 * m_ViewWidth + w * 4;

			BYTE b = m_Data[Index]; // blue
			BYTE g = m_Data[Index + 1]; // green
			BYTE r = m_Data[Index + 2]; // red
			

			int IndexTemp = (m_ViewHeight - 1 - h) * 4 * m_ViewWidth + w * 4;
			m_DataTemp[IndexTemp] = b;
			m_DataTemp[IndexTemp + 1] = g;
			m_DataTemp[IndexTemp + 2] = r;
			m_DataTemp[IndexTemp + 3] = 0;
		}
	}

	//выводим задний буфер на экран
	HDC hDC = GetDC(m_hWnd);
	DrawDibDraw(m_hDD, hDC, 0, 0, m_ViewWidth, m_ViewHeight, &m_Bih, m_DataTemp, 0, 0, m_ViewWidth, m_ViewHeight, 0);
	ReleaseDC(m_hWnd, hDC);
}

//********************************
//Функция удаляет задний буфер
//********************************
void CMeshManager::Delete_BackBuffer()
{
	DrawDibClose(m_hDD);

	free(m_Data);
	m_Data = NULL;

	free(m_DataTemp);
	m_DataTemp = NULL;
}

//**********************************************
//Функция камеры, функция следит за нажатиями
//на клавиши для перемещения по сцене и 
//возвращает матрицу вида
//**********************************************
matrix4x4 CMeshManager::Get_View_Matrix()
{
	float Time = Get_Elapsed_Time();

	POINT MousePos;
	GetCursorPos(&MousePos);

	SetCursorPos(m_ViewWidth/2, m_ViewHeight/2);
    
	int DeltaY=m_ViewHeight/2-MousePos.y;
	int DeltaX = m_ViewWidth / 2 - MousePos.x;

	//для движения камеры вверх-вниз
	//изменить на значение отличное от нуля
	//что бы камера двигалась вверх-вниз
	float RotationScalerY = 0.0f;
	//для движения камеры влево-вправо
	float RotationScalerX = 0.2f;
	
	//движение камеры вверх-вниз
	if(DeltaY<0) RotationScalerY = RotationScalerY;
	else if(DeltaY>0) RotationScalerY = -RotationScalerY;
	else if(DeltaY==0) RotationScalerY = 0;

	matrix4x4 MatRotRight = Matrix_Rotation_Axis(m_VecRight, RotationScalerY);
	
	//векторы в функцию передаем по адресу
	//не по значению поэтому необходимы
	//временные векторы
	vector4 VecTemp0, VecTemp1, VecTemp2;

	Vec4_Mat4x4_Mul(VecTemp0, m_VecRight, MatRotRight);
	Vec4_Mat4x4_Mul(VecTemp1, m_VecUp, MatRotRight);
	Vec4_Mat4x4_Mul(VecTemp2, m_VecLook, MatRotRight);
	
	
	//движение камеры влево- вправо
	if(DeltaX<0) RotationScalerX = RotationScalerX;
	else if(DeltaX>0) RotationScalerX = -RotationScalerX;
	else if(DeltaX==0) RotationScalerX = 0;

	vector4 VecUpTemp = vector4( 0.0f, 1.0f, 0.0f );
	matrix4x4 MatRotUp = Matrix_Rotation_Axis(VecUpTemp, RotationScalerX);
	
	Vec4_Mat4x4_Mul(m_VecRight, VecTemp0, MatRotUp);
	Vec4_Mat4x4_Mul(m_VecUp, VecTemp1, MatRotUp);
	Vec4_Mat4x4_Mul(m_VecLook, VecTemp2, MatRotUp);
	
	//движение камеры
	//вперед, назад, шаг влево, шаг вправо
	float RatioMove = 50;
	vector4 VecTemp = vector4(0,0,0);;
	vector4 VecAccel = vector4(0,0,0);
	
	if(GetAsyncKeyState('W')& 0xFF00) 
	{
		VecTemp = vector4(m_VecLook.x,0, m_VecLook.z);
		VecTemp = VecTemp * RatioMove;
		VecAccel = VecTemp * Time;
	}

	if(GetAsyncKeyState('S')& 0xFF00) 
	{
		VecTemp = vector4(m_VecLook.x,0, m_VecLook.z);
		VecTemp = VecTemp * (-RatioMove);
		VecAccel = VecTemp * Time;
	}

	if(GetAsyncKeyState('D')& 0xFF00) 
	{
		VecTemp = vector4(m_VecRight.x,0, m_VecRight.z);
		VecTemp = VecTemp * RatioMove;
		VecAccel = VecTemp * Time;
	}

	if(GetAsyncKeyState('A')& 0xFF00) 
	{
		VecTemp = vector4(m_VecRight.x,0, m_VecRight.z);
		VecTemp = VecTemp * (-RatioMove);
		VecAccel = VecTemp * Time;
	}

	m_VecPos = m_VecPos + VecAccel;

	//рассчитываем матрицу вида
	Vec4_Normalize(m_VecLook, m_VecLook);
	Vec4_Cross(m_VecUp, m_VecLook, m_VecRight);
	Vec4_Normalize(m_VecUp, m_VecUp);
	Vec4_Cross(m_VecRight, m_VecUp, m_VecLook);
	Vec4_Normalize(m_VecRight, m_VecRight);

	//vector dot
	float xp = -Vec4_Dot(m_VecPos, m_VecRight);
	float yp = -Vec4_Dot(m_VecPos, m_VecUp);
	float zp = -Vec4_Dot(m_VecPos, m_VecLook);

	matrix4x4 MatView = matrix4x4(
		m_VecRight.x, m_VecUp.x, m_VecLook.x, 0,
		m_VecRight.y, m_VecUp.y, m_VecLook.y, 0,
		m_VecRight.z, m_VecUp.z, m_VecLook.z, 0,
		xp,		  yp,	 zp, 1 );

	return MatView;

}

//******************************************************
//Функция трансформирует сцену
//******************************************************
void CMeshManager::Update_MeshManager()
{
	m_MatWorld = matrix4x4(
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1);

	m_MatView = Get_View_Matrix();

	float Fov = PI / 2.0f; // FOV 90 degree
	float Aspect = (float)m_ViewWidth / m_ViewHeight;
	float ZFar = 250.0f;
	m_ZNear = 0.01f;

	float h, w, Q;
	w = (1.0f / tanf(Fov * 0.5f)) / Aspect;
	h = 1.0f / tanf(Fov * 0.5f);
	Q = ZFar / (ZFar - m_ZNear);

	m_MatProj = matrix4x4(
		w, 0, 0, 0,
		0, h, 0, 0,
		0, 0, Q, 1,
		0, 0, -Q * m_ZNear, 0);

	matrix4x4 MatTemp;
	Mat4x4_Mat4x4_Mul(MatTemp, m_MatWorld, m_MatView);
	Mat4x4_Mat4x4_Mul(m_MatRes, MatTemp, m_MatProj);

	Transform_BSP_Tree(m_Root);
}

//******************************************************
//Функция отображает сцену на экране
//******************************************************
void CMeshManager::Draw_MeshManager()
{
	Clear_BackBuffer();

	Draw_BSP_Tree (m_Root, m_VecPos);

	Present_BackBuffer();
}

//************************************************
//Функция выводит список треугольников на экран
//************************************************
void CMeshManager::Draw_Polygon_List(list Polygons)
{
	for (UINT i = 0; i < Polygons.PolygonCount; i++)
    {
		vector4 Vec0, Vec1, Vec2;
		Vec0 = Polygons.PolyList[i].Vertex[0];
		Vec1 = Polygons.PolyList[i].Vertex[1];
		Vec2 = Polygons.PolyList[i].Vertex[2];

		UINT Tex = Polygons.PolyList[i].TexID;

		//устанавливаем текущую текстуру
		m_Tex = (UCHAR *) m_LevelTile[Tex];

		//ширина и высота текущей текстуры
		m_TextureWidth = m_TexInfo[Tex].TexWidth;
		m_TextureHeight = m_TexInfo[Tex].TexHeight;

		Draw_Textured_Triangle(Vec0, Vec1, Vec2);
	}
}

//**********************************************************
//Функция разбивает входящий треугольник на два
//верхний и нижний и вызывает функцию
//растеризации одного треугольника
//или может быть один треугольник с плоским низом, верхом
//**********************************************************
void CMeshManager::Draw_Textured_Triangle(vector4 Vec0,
					vector4 Vec1, vector4 Vec2)
{
	int Side;
	float x1, x2, x3;
	float y1, y2, y3;
	float iz1, uiz1, viz1, iz2, uiz2, viz2, iz3, uiz3, viz3;
	float Temp;
	int y1i, y2i, y3i;
	int dy;
	float dyl, dyr;

	x1 = Vec0.x;
	y1 = Vec0.y;
	x2 = Vec1.x;
	y2 = Vec1.y;
	x3 = Vec2.x;
	y3 = Vec2.y;

	//текстурные координаты в файле
	//имеют диапазон от 0 до 1
	Vec0.tu *= m_TextureWidth - 1;
	Vec0.tv *= m_TextureHeight - 1;
	Vec1.tu *= m_TextureWidth - 1;
	Vec1.tv *= m_TextureHeight - 1;
	Vec2.tu *= m_TextureWidth - 1;
	Vec2.tv *= m_TextureHeight - 1;
	
	//после умножения на матрицу
	//проекции w хранит значение z
	//фактически тут мы делим на z
	iz1 = 1.0f / Vec0.w;
	iz2 = 1.0f / Vec1.w;
	iz3 = 1.0f / Vec2.w;
	
	uiz1 = Vec0.tu * iz1;
	viz1 = Vec0.tv * iz1;
	uiz2 = Vec1.tu * iz2;
	viz2 = Vec1.tv * iz2;
	uiz3 = Vec2.tu * iz3;
	viz3 = Vec2.tv * iz3;

	#define swapfloat(x, y) Temp = x; x = y; y = Temp;

	if (y1 > y2)
	{
		swapfloat(x1, x2);
		swapfloat(y1, y2);
		swapfloat(iz1, iz2);
		swapfloat(uiz1, uiz2);
		swapfloat(viz1, viz2);
	}
	if (y1 > y3)
	{
		swapfloat(x1, x3);
		swapfloat(y1, y3);
		swapfloat(iz1, iz3);
		swapfloat(uiz1, uiz3);
		swapfloat(viz1, viz3);
	}
	if (y2 > y3)
	{
		swapfloat(x2, x3);
		swapfloat(y2, y3);
		swapfloat(iz2, iz3);
		swapfloat(uiz2, uiz3);
		swapfloat(viz2, viz3);
	}
	
	#undef swapfloat

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

	int MinClipY = 0;
	int MaxClipY = m_ViewHeight;

	y1i = (int) floor (y1);
	y2i = (int) floor (y2);
	y3i = (int) floor (y3);

	if(y1i < MinClipY) y1i = 0;
	if(y2i < MinClipY) y2i = 0;
	if(y3i < MinClipY) y3i = 0;

	if(y1i > MaxClipY) y1i = m_ViewHeight;
	if(y2i > MaxClipY) y2i = m_ViewHeight;
	if(y3i > MaxClipY) y3i = m_ViewHeight;

	//отбрасываем треугольники которые вне экрана
	if(y1i == 0 && y2i == 0 && y3i == 0)
		return;

	//отбрасываем треугольники которые вне экрана
	if(y1i == m_ViewHeight && y2i == m_ViewHeight && y3i == m_ViewHeight)
		return;

	//отбрасываем треугольники которые вне экрана
	if(x1 < 0 && x2 < 0 && x3 < 0)
		return;

	//отбрасываем треугольники которые вне экрана
	if(x1 > m_ViewWidth && x2 > m_ViewWidth && x3 > m_ViewWidth)
		return;

	if (!Side)
	{
		dyl = y3 - y1;
		m_dxdyl = (x3 - x1) / dyl;
		m_dudyl = (uiz3 - uiz1) / dyl;
		m_dvdyl = (viz3 - viz1) / dyl;
		m_dzdyl = (iz3 - iz1) / dyl;
	
		dy = (int)y1 - y1i;
		m_xl = x1 - dy * m_dxdyl;
		m_ul = uiz1 - dy * m_dudyl;
		m_vl = viz1 - dy * m_dvdyl;
		m_zl = iz1 - dy * m_dzdyl;

		if (y1i < y2i)
		{
			dyr = y2 - y1;
			m_dxdyr = (x2 - x1) / dyr;
			m_dudyr = (uiz2 - uiz1) / dyr;
			m_dvdyr = (viz2 - viz1) / dyr;
			m_dzdyr = (iz2 - iz1) / dyr;

			m_xr = x1 - dy * m_dxdyr;
			m_ur = uiz1 - dy * m_dudyr;
			m_vr = viz1 - dy * m_dvdyr;
			m_zr = iz1 - dy * m_dzdyr;

			Draw_Textured_Poly(y1i, y2i);
		}
		if (y2i < y3i)
		{
			dyr = y3 - y2;
			m_dxdyr = (x3 - x2) / dyr;
			m_dudyr = (uiz3 - uiz2) / dyr;
			m_dvdyr = (viz3 - viz2) / dyr;
			m_dzdyr = (iz3 - iz2) / dyr;

			dy = (int) y2 - y2i;
			m_xr = x2 - dy * m_dxdyr;
			m_ur = uiz2 - dy * m_dudyr;
			m_vr = viz2 - dy * m_dvdyr;
			m_zr = iz2 - dy * m_dzdyr;

			Draw_Textured_Poly(y2i, y3i);
		}
	}
	else
	{
		dyr = y3 - y1;
		m_dxdyr = (x3 - x1) / dyr;
		m_dudyr = (uiz3 - uiz1) / dyr;
		m_dvdyr = (viz3 - viz1) / dyr;
		m_dzdyr = (iz3 - iz1) / dyr;

		dy = (int)y1 - y1i;
		m_xr = x1 - dy * m_dxdyr;
		m_ur = uiz1 - dy * m_dudyr;
		m_vr = viz1 - dy * m_dvdyr;
		m_zr = iz1 - dy * m_dzdyr;
		
		if (y1i < y2i)
		{
	
			dyl = y2 - y1;
			m_dxdyl = (x2 - x1) / dyl;
			m_dudyl = (uiz2 - uiz1) / dyl;
			m_dvdyl = (viz2 - viz1) / dyl;
			m_dzdyl = (iz2 - iz1) / dyl;

			m_xl = x1 - dy * m_dxdyl;
			m_ul = uiz1 - dy * m_dudyl;
			m_vl = viz1 - dy * m_dvdyl;
			m_zl = iz1 - dy * m_dzdyl;

			Draw_Textured_Poly(y1i, y2i);
		}
		if (y2i < y3i)
		{
			dyl = y3 - y2;
			m_dxdyl = (x3 - x2) / dyl;
			m_dudyl = (uiz3 - uiz2) / dyl;
			m_dvdyl = (viz3 - viz2) / dyl;
			m_dzdyl = (iz3 - iz2) / dyl;
			
			dy = (int) y2 - y2i;
			m_xl = x2 - dy * m_dxdyl;
			m_ul = uiz2 - dy * m_dudyl;
			m_vl = viz2 - dy * m_dvdyl;
			m_zl = iz2 - dy * m_dzdyl;

			Draw_Textured_Poly(y2i, y3i);
		}
	}
}

//*******************************************
//Функция растеризации одного треугольника
//*******************************************
void CMeshManager::Draw_Textured_Poly(int y1, int y2)
{
	int x1, x2;
	int dx;
	float ui, vi, zi;
	float du, dv, dz;

	int MinClipX = 0;
	int MaxClipX = m_ViewWidth;

	for (int yi = y1; yi<y2; yi++)
	{
		x1 = (int)m_xl;
		x2   = (int)m_xr;

		ui = m_ul;
		vi = m_vl;
		zi = m_zl;

		if ((dx = (x2 - x1))>0)
		{
			du = (m_ur - m_ul)/dx;
			dv = (m_vr - m_vl)/dx;
			dz = (m_zr - m_zl)/dx;
		}
		else
		{
			du = (m_ur - m_ul);
			dv = (m_vr - m_vl);
			dz = (m_zr - m_zl);
		}

		if(x1 < MinClipX)
		{
			dx = MinClipX-x1;

			ui+=dx * du;
			vi+=dx * dv;
			zi+=dx * dz;

			x1 = MinClipX;
		}

		if(x2> MaxClipX)
			x2 = MaxClipX;

		for (int xi=x1; xi<x2; xi++)
		{

			float z = 1.0f/zi;
			float u = ui * z;
			float Vec = vi * z;

			UINT Temp = (int)u  + (((int)Vec) * m_TextureWidth);

			//если Temp выходит за размеры массива текстуры
			if (Temp > (m_TextureWidth * m_TextureHeight - 1))
				return;

			if (Temp < 0)
				return;

			Temp= Temp*3;
			
			int Index = yi * m_BackLpitch + xi * 4;
			
			m_Data[Index] = m_Tex[Temp];
			Index++;

			m_Data[Index] = m_Tex[Temp + 1];
			Index++;

			m_Data[Index] = m_Tex[Temp + 2];
			Index++;

			m_Data[Index] =  0;
			Index++;

			ui+=du;
			vi+=dv;
			zi+=dz;
		}

		m_xl+= m_dxdyl;
		m_ul+= m_dudyl;
		m_vl+= m_dvdyl;
		m_zl+= m_dzdyl;

		m_xr+= m_dxdyr;
		m_ur+= m_dudyr;
		m_vr+= m_dvdyr;
		m_zr+= m_dzdyr;
	}
}

//***********************************************
//Функция загрузает BMP файл с текстурой сцены
//***********************************************
bool CMeshManager::Load_BMP(char *Filename, int Tile)
{
	FILE *Fp = NULL;

	fopen_s(&Fp, Filename,"rb");
	if(Fp==NULL)
		MessageBox(NULL, "Error Open BMP File", "INFO", MB_OK);

	BITMAPFILEHEADER Bfh;
	fread(&Bfh, sizeof(Bfh), 1, Fp);

	BITMAPINFOHEADER Bih;
	fread(&Bih, sizeof(Bih), 1, Fp);

	m_LevelTile[Tile] = new unsigned char [Bih.biWidth*Bih.biHeight*3];

	fread(m_LevelTile[Tile],Bih.biWidth*Bih.biHeight*3,1,Fp);

	//для каждой текстуры сохраняем ее ширину и высоту
	m_TexInfo[Tile].TexWidth = Bih.biWidth;
	m_TexInfo[Tile].TexHeight = Bih.biHeight;
	
	return true;
}

//****************************
//Функция строит BSP дерево
//****************************
void CMeshManager::Build_BSP_Tree(BSP_tree *Tree, list Polygons)
{
	memset(Tree,0, sizeof(BSP_tree));
	polygon   *Root = Polygons.Get_From_List();
	Tree->Partition= Root->Get_Plane();
	Tree->Polygons.Add_To_List(Root);

	list      FrontList, BackList;
    polygon   *Poly;

	while ((Poly = Polygons.Get_From_List()) != 0)
	{
		int Result = Tree->Partition.Classify_Polygon (Poly);

		switch (Result)
		{
			case COINCIDENT:
				Tree->Polygons.Add_To_List (Poly);
				break;
			
			case IN_BACK_OF:
				BackList.Add_To_List (Poly);
			    break;
			
			case IN_FRONT_OF:
	            FrontList.Add_To_List (Poly);
		        break;
			
			case SPANNING:
				int cf = 0, cb = 0;
				polygon   *front_piece=NULL, *back_piece=NULL;
				Split_Polygon (Poly, &Tree->Partition, front_piece, back_piece, cf, cb);

				if(cf == 1 && cb == 1)
				{
					FrontList.Add_To_List (&front_piece[0]);
					BackList.Add_To_List (&back_piece[0]);
				}
				else if(cf == 1 && cb == 0)
				{
					FrontList.Add_To_List (&front_piece[0]);
				}
				else if(cf == 1 && cb == 2)
				{
					FrontList.Add_To_List (&front_piece[0]);
					
					BackList.Add_To_List (&back_piece[0]);	
					BackList.Add_To_List (&back_piece[1]);
				}
				else if(cb == 1 && cf == 0)
				{
					BackList.Add_To_List (&back_piece[0]);	
				}
				else if(cb == 1 && cf == 2)
				{
					BackList.Add_To_List (&back_piece[0]);	
						
					FrontList.Add_To_List (&front_piece[0]);	
					FrontList.Add_To_List (&front_piece[1]);
				}
			break;
		  }
	}

	if (!FrontList.Is_Empty_List())
	{
		Tree->Front = new BSP_tree;
		Build_BSP_Tree (Tree->Front, FrontList);
	}
	if ( ! BackList.Is_Empty_List ())
	{
		Tree->Back = new BSP_tree;
		Build_BSP_Tree (Tree->Back, BackList);
	}
}

//*******************************************
//Функция на основе полигона (треугольника)
//создает плоскость
//*******************************************
plane polygon::Get_Plane()
{
	plane p;

	vector4 VecEdge1 = Vertex[1] - Vertex[0];
	vector4 VecEdge2 = Vertex[2] - Vertex[0];
	
	p.a = (VecEdge1.y*VecEdge2.z)-(VecEdge2.y*VecEdge1.z);
    p.b = (VecEdge1.z*VecEdge2.x)-(VecEdge2.z*VecEdge1.x);
    p.c = (VecEdge1.x*VecEdge2.y)-(VecEdge2.x*VecEdge1.y);
    
	float Len = sqrtf(p.a * p.a + p.b * p.b + p.c * p.c);

	p.a = p.a / Len;
	p.b = p.b / Len;
	p.c = p.c / Len;

	p.d=-(p.a*Vertex[0].x+p.b*Vertex[0].y+p.c*Vertex[0].z);

	return p;
}

//*******************************************
//Функция возвращает нормаль к треугольнику
//*******************************************
vector4 polygon::GetNormal()
{
	vector4 VecTemp;
        
	vector4 VecEdge1 = Vertex[1] - Vertex[0];
	vector4 VecEdge2 = Vertex[2] - Vertex[0];
	
	VecTemp.x = (VecEdge1.y*VecEdge2.z)-(VecEdge2.y*VecEdge1.z);
	VecTemp.y = (VecEdge1.z*VecEdge2.x)-(VecEdge2.z*VecEdge1.x);
	VecTemp.z = (VecEdge1.x*VecEdge2.y)-(VecEdge2.x*VecEdge1.y);
    
	return VecTemp;
}

//****************************************************
//Функция определяет расстояние точки до плоскости
//****************************************************
float plane::Classify_Point (vector4 Eye)
{
	return a*Eye.x + b*Eye.y + c*Eye.z + d;
}

//****************************************************
//Функция определяет треугольник лежит на плоскости,
//перед, за плоскостью, или разбивается плоскостью
//****************************************************
int plane::Classify_Polygon (polygon *Poly)
{
	float p[3];
	
	for(UINT i=0;i<3;i++)
		p[i]=a*Poly->Vertex[i].x + b*Poly->Vertex[i].y + c*Poly->Vertex[i].z + d;

	if(p[0]==0 && p[1]==0 && p[2]==0)
		return COINCIDENT;

	else if(p[0]<0 && p[1]<0 && p[2]<0)
		return IN_BACK_OF;

	else if(p[0]>0  && p[1]>0 && p[2]>0)
		return IN_FRONT_OF;
	
	else
		return SPANNING;
}

//*****************************************
//Функция возвращает нормаль к плоскости
//*****************************************
vector4 plane::Normal()
{
	vector4 VecTemp;

	VecTemp.x = a;
	VecTemp.y = b;
	VecTemp.z = c;

	return VecTemp;
}

//*******************************************
//Функция разбивает треугольник плоскостью
//на части
//*******************************************
void CMeshManager::Split_Polygon (polygon *Poly, plane *Part, polygon *&Front, polygon *&Back, int &cf, int &cb)
{
	int Count = 3;
	UINT FrontC = 0, BackC = 0;
	vector4 PtA, PtB, FrontSet[4], BackSet[4];
	float SideA, SideB;

	PtA = Poly->Vertex[Count - 1];

	SideA = Part->Classify_Point ((vector4)PtA);

	float fEpsilon = 0.001f;

	if(SideA < fEpsilon && SideA > -fEpsilon)
		SideA = 0.0f;
   
	for (short i = -1; ++i < Count;)
	{
		PtB = Poly->Vertex[i];

		SideB = Part->Classify_Point ((vector4)PtB);

		if(SideB < fEpsilon && SideB > -fEpsilon)
		SideB = 0.0f;

		if (SideB > 0)
		{
			if (SideA < 0)
			{
				// compute the intersection point of the line
				// from point A to point B with the Partition
				// plane. This is a simple ray-plane intersection.
				vector4 Vec = PtB - PtA;

				vector4 Norm = Part->Normal();
				float   fSect = - Part->Classify_Point (PtA) / Vec4_Dot(Norm, Vec);

				BackSet[BackC++] = PtA + (Vec * fSect);
				FrontSet[FrontC++] = PtA + (Vec * fSect);
			}
		 
			FrontSet[FrontC++] = PtB;
		 
		}
		else if (SideB < 0)
		{
			if (SideA > 0)
			{
				// compute the intersection point of the line
				// from point A to point B with the Partition
				// plane. This is a simple ray-plane intersection.
				vector4 Vec = PtB - PtA;
		
				vector4 Norm = Part->Normal();
				float   fSect = - Part->Classify_Point (PtA) / Vec4_Dot(Norm, Vec);
		
				BackSet[BackC++] = PtA + (Vec * fSect);
				FrontSet[FrontC++] = PtA + (Vec * fSect);
		
			}

			BackSet[BackC++] = PtB;
		}
		else
		{
			BackSet[BackC++] = PtB;
			FrontSet[FrontC++] = PtB;
		}
  
		PtA = PtB;
		SideA = SideB;
	}

	if(FrontC == 4) //in return we got 2 front triangles
	{
		Front = new polygon[2];
		ZeroMemory(Front, 2 * sizeof(polygon));
   
		//first tri
		Front[0].Vertex[0] = FrontSet[0];
		Front[0].Vertex[1] = FrontSet[1];
		Front[0].Vertex[2] = FrontSet[2];
		Front[0].TexID = Poly->TexID;

		//second tri
		Front[1].Vertex[0] = FrontSet[0];
		Front[1].Vertex[1] = FrontSet[2];
		Front[1].Vertex[2] = FrontSet[3];
		Front[1].TexID = Poly->TexID;
  
		//output tri count
		cf = 2;

	}
	else if(FrontC == 3) //in return we got 1 front triangle
	{
		Front = new polygon[1];
		ZeroMemory(Front, 1 * sizeof(polygon));

		Front[0].Vertex[0] = FrontSet[0];
		Front[0].Vertex[1] = FrontSet[1];
		Front[0].Vertex[2] = FrontSet[2];
		Front[0].TexID = Poly->TexID;
		
		//output tri count
		cf = 1;
	}
	else
	{
		//output tri count
		cf = 0;
	}

	if(BackC == 4) //in return we got 2 back triangles
	{
		Back = new polygon[2];
		ZeroMemory(Back, 2 * sizeof(polygon));

		//first tri
		Back[0].Vertex[0] = BackSet[0];
		Back[0].Vertex[1] = BackSet[1];
		Back[0].Vertex[2] = BackSet[2];
		Back[0].TexID = Poly->TexID;
		
		//second tri
		Back[1].Vertex[0] = BackSet[0];
		Back[1].Vertex[1] = BackSet[2];
		Back[1].Vertex[2] = BackSet[3];
		Back[1].TexID = Poly->TexID;

		//output tri count
		cb = 2;
	}
	else if(BackC == 3 ) //in return we got 1 back triangle
	{
		Back = new polygon[1];
		ZeroMemory(Back, 1 * sizeof(polygon));

		Back[0].Vertex[0] = BackSet[0];
		Back[0].Vertex[1] = BackSet[1];
		Back[0].Vertex[2] = BackSet[2];
		Back[0].TexID = Poly->TexID;
		
		//output tri count
		cb = 1;
	}
	else
	{
		//output tri count
		cb = 0;
	}
}

//*******************************************
//Перегруженый оператор вычитания векторов
//*******************************************
vector4 vector4::operator - (const vector4 & Vec)
{
	vector4 VecTemp;

	VecTemp.x = x - Vec.x;
	VecTemp.y = y - Vec.y;
	VecTemp.z = z - Vec.z;
	VecTemp.w = 1.0f;

	VecTemp.tu = tu - Vec.tu;
	VecTemp.tv = tv - Vec.tv;

	return VecTemp;
};

//******************************************
//Перегруженый оператор сложения векторов
//******************************************
vector4 vector4::operator + (const vector4& Vec)
{
	vector4 VecTemp;

	VecTemp.x = x + Vec.x;
	VecTemp.y = y + Vec.y;
	VecTemp.z = z + Vec.z;
	VecTemp.w = 1.0f;

	VecTemp.tu = tu + Vec.tu;
	VecTemp.tv = tv + Vec.tv;

	return VecTemp;
};

//******************************************
//Перегруженый оператор умножения вектора
//на скаляр
//******************************************
vector4 vector4::operator * (const float & Vec)
{
	vector4 VecTemp;

	VecTemp.x = x * Vec;
	VecTemp.y = y * Vec;
	VecTemp.z = z * Vec;
	VecTemp.w = 1.0f;

	VecTemp.tu = tu * Vec;
	VecTemp.tv = tv * Vec;

	return VecTemp;
};

//*************************************************
//Перегруженый оператор присваивания для вектора
//*************************************************
vector4 & vector4::operator = (const vector4 & Vec)
{
	x = Vec.x;
	y = Vec.y;
	z = Vec.z;
	w = Vec.w;

	tu = Vec.tu;
	tv = Vec.tv;

	return *this;
}


//**************************************************
//Конструктор с параметрами для структуры вектора
//**************************************************
vector4::vector4(float ix, float iy, float iz)
{
	x = ix;
	y = iy;
	z = iz;
	w = 1.0f;
}

//***********************************
//Деструктор для структуры вектора
//***********************************
vector4::~vector4()
{
};

//*************************************************
//Конструктор по умолчанию для структуры вектора
//*************************************************
vector4::vector4()
{
};

//*****************************
//Функция удаляет BSP дерево
//*****************************
void CMeshManager::Delete_BSP(BSP_tree *Tree)
{
	if(!Tree)
		return;

    if (Tree->Polygons.PolyList)
    {
		free(Tree->Polygons.PolyList);
    }

	if (Tree->TransformedPolygons.PolyList)
    {
		free(Tree->TransformedPolygons.PolyList);
    }

	Delete_BSP(Tree->Front);
    delete Tree->Front;

    Delete_BSP(Tree->Back);
    delete Tree->Back;
}

//*********************************************
//Функция проверяет находится ли треугольник
//в области просмотра (frustum view)
//*********************************************
bool CMeshManager::Polygon_In_Frustum( UINT NumPoints, vector4* PointList )
{
	UINT f, p;

	//имеется 6 плоскостей пирамиды просмотра
	for( f = 0; f < 6; f++ )
	{
		//для треугольника это 3 вершины
		for( p = 0; p < NumPoints; p++ )
		{
			if(m_Frustum[f][0] * PointList[p].x + m_Frustum[f][1] * PointList[p].y + m_Frustum[f][2] * PointList[p].z + m_Frustum[f][3] > 0 )
            break;
		}
	
		if( p == NumPoints )
			return false;
   }

   return true;
}

//*************************************************
//Функция из матрицы извлекает плоскости 
//отсечения области просмотра - верхнюю, нижнюю,
//левую, правую, блюжнюю, дальнюю
//*************************************************
void CMeshManager::Extract_Frustum()
{
	float Temp;
	
	// Extract the RIGHT clipping plane
	m_Frustum[0][0] = m_MatRes.Mat[M03] - m_MatRes.Mat[M00];
	m_Frustum[0][1] = m_MatRes.Mat[M13] - m_MatRes.Mat[M10];
	m_Frustum[0][2] = m_MatRes.Mat[M23] - m_MatRes.Mat[M20];
	m_Frustum[0][3] = m_MatRes.Mat[M33] - m_MatRes.Mat[M30];

	// Normalize it
	Temp = (float) sqrt(m_Frustum[0][0] * m_Frustum[0][0] + m_Frustum[0][1] * m_Frustum[0][1] + m_Frustum[0][2] * m_Frustum[0][2] );
	m_Frustum[0][0] /= Temp;
	m_Frustum[0][1] /= Temp;
	m_Frustum[0][2] /= Temp;
	m_Frustum[0][3] /= Temp;

	// Extract the LEFT clipping plane
	m_Frustum[1][0] = m_MatRes.Mat[M03] + m_MatRes.Mat[M00];
	m_Frustum[1][1] = m_MatRes.Mat[M13] + m_MatRes.Mat[M10];
	m_Frustum[1][2] = m_MatRes.Mat[M23] + m_MatRes.Mat[M20];
	m_Frustum[1][3] = m_MatRes.Mat[M33] + m_MatRes.Mat[M30];

	// Normalize it
	Temp = (float) sqrt(m_Frustum[1][0] * m_Frustum[1][0] + m_Frustum[1][1] * m_Frustum[1][1] + m_Frustum[1][2] * m_Frustum[1][2] );
	m_Frustum[1][0] /= Temp;
	m_Frustum[1][1] /= Temp;
	m_Frustum[1][2] /= Temp;
	m_Frustum[1][3] /= Temp;

	// Extract the BOTTOM clipping plane
	m_Frustum[2][0] = m_MatRes.Mat[M03] + m_MatRes.Mat[M01];
	m_Frustum[2][1] = m_MatRes.Mat[M13] + m_MatRes.Mat[M11];
	m_Frustum[2][2] = m_MatRes.Mat[M23] + m_MatRes.Mat[M21];
	m_Frustum[2][3] = m_MatRes.Mat[M33] + m_MatRes.Mat[M31];

	// Normalize it
	Temp = (float) sqrt(m_Frustum[2][0] * m_Frustum[2][0] + m_Frustum[2][1] * m_Frustum[2][1] + m_Frustum[2][2] * m_Frustum[2][2] );
	m_Frustum[2][0] /= Temp;
	m_Frustum[2][1] /= Temp;
	m_Frustum[2][2] /= Temp;
	m_Frustum[2][3] /= Temp;

	// Extract the TOP clipping plane
	m_Frustum[3][0] = m_MatRes.Mat[M03] - m_MatRes.Mat[M01];
	m_Frustum[3][1] = m_MatRes.Mat[M13] - m_MatRes.Mat[M11];
	m_Frustum[3][2] = m_MatRes.Mat[M23] - m_MatRes.Mat[M21];
	m_Frustum[3][3] = m_MatRes.Mat[M33] - m_MatRes.Mat[M31];

	// Normalize it
	Temp = (float) sqrt(m_Frustum[3][0] * m_Frustum[3][0] + m_Frustum[3][1] * m_Frustum[3][1] + m_Frustum[3][2] * m_Frustum[3][2] );
	m_Frustum[3][0] /= Temp;
	m_Frustum[3][1] /= Temp;
	m_Frustum[3][2] /= Temp;
	m_Frustum[3][3] /= Temp;

	// Extract the FAR clipping plane
	m_Frustum[4][0] = m_MatRes.Mat[M03] - m_MatRes.Mat[M02];
	m_Frustum[4][1] = m_MatRes.Mat[M13] - m_MatRes.Mat[M12];
	m_Frustum[4][2] = m_MatRes.Mat[M23] - m_MatRes.Mat[M22];
	m_Frustum[4][3] = m_MatRes.Mat[M33] - m_MatRes.Mat[M32];

	// Normalize it
	Temp = (float) sqrt(m_Frustum[4][0] * m_Frustum[4][0] + m_Frustum[4][1] * m_Frustum[4][1] + m_Frustum[4][2] * m_Frustum[4][2] );
	m_Frustum[4][0] /= Temp;
	m_Frustum[4][1] /= Temp;
	m_Frustum[4][2] /= Temp;
	m_Frustum[4][3] /= Temp;

	// Extract the NEAR clipping plane.  This is last on purpose (see pointinfrustum() for reason)
	m_Frustum[5][0] = m_MatRes.Mat[M03] + m_MatRes.Mat[M02];
	m_Frustum[5][1] = m_MatRes.Mat[M13] + m_MatRes.Mat[M12];
	m_Frustum[5][2] = m_MatRes.Mat[M23] + m_MatRes.Mat[M22];
	m_Frustum[5][3] = m_MatRes.Mat[M33] + m_MatRes.Mat[M32];

	// Normalize it
	Temp = (float) sqrt(m_Frustum[5][0] * m_Frustum[5][0] + m_Frustum[5][1] * m_Frustum[5][1] + m_Frustum[5][2] * m_Frustum[5][2] );
	m_Frustum[5][0] /= Temp;
	m_Frustum[5][1] /= Temp;
	m_Frustum[5][2] /= Temp;
	m_Frustum[5][3] /= Temp;
}

//***************************************************
//Функция используется для рассечения треугольника
//ближней плоскостью отсечения
//***************************************************
vector4 CMeshManager::Calc_Edge(vector4 Vec1, vector4 Vec2)
{
	//вершина перед передней плоскостью Vec1
	//вершина за передней проскостью Vec2

	vector4 VecTemp;

	float d = Vec1.z / (Vec1.z - Vec2.z);

	VecTemp.x = (Vec2.x - Vec1.x) * d + Vec1.x;
	VecTemp.y = (Vec2.y - Vec1.y) * d + Vec1.y;
	VecTemp.z = m_ZNear + 0.05f;

	VecTemp.tu = (Vec2.tu - Vec1.tu) * d + Vec1.tu;
	VecTemp.tv = (Vec2.tv - Vec1.tv) * d + Vec1.tv;

	return VecTemp;
}

//***********************************************
//Функция трансформирует BSP дерево (умножение
//на матрицы мира, вида, проекции и т.п.)
//***********************************************
void CMeshManager::Transform_BSP_Tree (BSP_tree *Tree)
{
	if(!Tree)
		return;

	//обнуляем список транформированных полигонов
	//для следующего кадра - например в этом кадре
	//камера переместилась
	if(Tree->TransformedPolygons.PolyList != NULL)
	 {
		 Tree->TransformedPolygons.PolygonCount = 0;
		 free(Tree->TransformedPolygons.PolyList);
		 Tree->TransformedPolygons.PolyList = NULL;
	}

	//извлекаем плоскости отсечения из пирамиды просмотра
	Extract_Frustum();

	//в цикле перебираем все полигоны в списке
	//и трансформируем их - умножение на матрицы
	//вида, мира, отбрасывание задних поверхностей,
	//отсечение по передней плоскости просмотра
	//матрица проекции, деление на z, экранные координаты
	//заносим полигон в список трансформированных полигонов
	for ( UINT j = 0; j < Tree->Polygons.PolygonCount; j++)
	 {
		 
		//плоскости отсечения извлекли и 
		//если полигон вне пирамиды просотра
		//отбрасываем его - не рисуем, переходим
		//к следующему полигону
		if (!Polygon_In_Frustum(3, Tree->Polygons.PolyList[j].Vertex))
			continue;

		//если полигон прошел тест на объем просмотра
		//получаем вершины из списка
		vector4 Vec0 = Tree->Polygons.PolyList[j].Vertex[0];
		vector4 Vec1 = Tree->Polygons.PolyList[j].Vertex[1];
		vector4 Vec2 = Tree->Polygons.PolyList[j].Vertex[2];
		
		//векторы в функцию передаем по адресу
		//не по значению поэтому необходимы
		//временные веторы
		vector4 VecTemp0, VecTemp1, VecTemp2;

		//умножаем вершины на матрицу мира
		Vec4_Mat4x4_Mul(VecTemp0, Vec0, m_MatWorld);
		Vec4_Mat4x4_Mul(VecTemp1, Vec1, m_MatWorld);
		Vec4_Mat4x4_Mul(VecTemp2, Vec2, m_MatWorld);

		//умножаем вершины на матрицу вида (перемещение камеры)
		Vec4_Mat4x4_Mul(Vec0, VecTemp0, m_MatView);
		Vec4_Mat4x4_Mul(Vec1, VecTemp1, m_MatView);
		Vec4_Mat4x4_Mul(Vec2, VecTemp2, m_MatView);
		
		//backface culling
		//отбрасывание задних поверхностей
		//vector subtract
		vector4 VecEdge1 = Vec1 - Vec0;
		vector4 VecEdge2 = Vec2 - Vec0;

		Vec4_Normalize(VecEdge1, VecEdge1);
		Vec4_Normalize(VecEdge2, VecEdge2);

		vector4 VecCross;
		Vec4_Cross(VecCross, VecEdge1, VecEdge2);
		Vec4_Normalize(VecCross, VecCross);

		//Vec0 в пространстве вида
		//vector dot
		float Dot = Vec4_Dot(VecCross, Vec0);

		//если cos от 0 до 90 градусов
		//отбрасываем полигон - не рисуем
		//переходим к следующему полигону
		if (Dot >= 0.0)
			continue;
		
		//если полигон прошел тест на backface culling
		//начинаем отсечение по передней плоскости
		//отсечение производится в пространстве вида
		//в результате отсечения накапливаем новые
		//треугольники в FrontList
		//FrontList может содержать один или два треугольника
		//получившихся после отсечения передней плоскостью
		//отсечения, либо один исходный треугольник
		//если небыло отсечения
		
		list FrontList;

		//все три вершины перед плоскостью
		if( (Vec0.z > 0) && (Vec1.z > 0) && (Vec2.z > 0) )
		{
			polygon Poly;

			Poly.Vertex[0] = Vec0;
			Poly.Vertex[1] = Vec1;
			Poly.Vertex[2] = Vec2;

			Poly.TexID = Tree->Polygons.PolyList[j].TexID; 

			FrontList.Add_To_List(&Poly);
		}

		//вариант 1 - Vec0 за плоскостью Vec1, Vec2 перед плоскостью
		if( (Vec0.z < 0) && (Vec1.z > 0) && (Vec2.z > 0) )
		{
			// Vec0 -> Vec1
			// Vec0 -> Vec2

			vector4 vt0 = Calc_Edge(Vec1, Vec0);
			vector4 vt1 = Calc_Edge(Vec2, Vec0);
			
			polygon Poly;

			Poly.Vertex[0] = vt0;
			Poly.Vertex[1] = Vec1;
			Poly.Vertex[2] = Vec2;

			Poly.TexID = Tree->Polygons.PolyList[j].TexID; 

			FrontList.Add_To_List(&Poly);

			Poly.Vertex[0] = vt0;
			Poly.Vertex[1] = Vec2;
			Poly.Vertex[2] = vt1;

			Poly.TexID = Tree->Polygons.PolyList[j].TexID; 

			FrontList.Add_To_List(&Poly);
		}

		//вариант 2 - Vec1 за плоскостью, Vec0, Vec2 перед плоскостью
		if( (Vec0.z > 0) && (Vec1.z < 0) && (Vec2.z > 0 ) )
		{
			// Vec1 -> Vec2
			// Vec1 -> Vec0
			vector4 vt0 = Calc_Edge(Vec2, Vec1);
			vector4 vt1 = Calc_Edge(Vec0, Vec1);

			polygon Poly;

			Poly.Vertex[0] = vt0;
			Poly.Vertex[1] = Vec2;
			Poly.Vertex[2] = Vec0;

			Poly.TexID = Tree->Polygons.PolyList[j].TexID; 

			FrontList.Add_To_List(&Poly);

			Poly.Vertex[0] = vt0;
			Poly.Vertex[1] = Vec0;
			Poly.Vertex[2] = vt1;

			Poly.TexID = Tree->Polygons.PolyList[j].TexID; 

			FrontList.Add_To_List(&Poly);
		}

		//вариант 3 - Vec2 за плоскостью, Vec0, Vec1 перед плоскостью
		if( (Vec0.z > 0) && (Vec1.z > 0) && (Vec2.z < 0 ) )
		{
			// Vec2 -> Vec0
			// Vec2 -> Vec1
			vector4 vt0 = Calc_Edge(Vec0, Vec2);
			vector4 vt1 = Calc_Edge(Vec1, Vec2);

			polygon Poly;

			Poly.Vertex[0] = vt0;
			Poly.Vertex[1] = Vec0;
			Poly.Vertex[2] = Vec1;

			Poly.TexID = Tree->Polygons.PolyList[j].TexID; 

			FrontList.Add_To_List(&Poly);

			Poly.Vertex[0] = vt0;
			Poly.Vertex[1] = Vec1;
			Poly.Vertex[2] = vt1;

			Poly.TexID = Tree->Polygons.PolyList[j].TexID; 

			FrontList.Add_To_List(&Poly);
		}

		//вариант 4 - Vec0, Vec1 за плоскостью, Vec2 перед плоскостью
		if( (Vec0.z < 0) && (Vec1.z < 0) && (Vec2.z > 0 ) )
		{
			// Vec1 -> Vec2
			// Vec0 -> Vec2
			vector4 vt0 = Calc_Edge(Vec2, Vec1);
			vector4 vt1 = Calc_Edge(Vec2, Vec0);

			polygon Poly;

			Poly.Vertex[0] = vt0;
			Poly.Vertex[1] = Vec2;
			Poly.Vertex[2] = vt1;

			Poly.TexID = Tree->Polygons.PolyList[j].TexID; 

			FrontList.Add_To_List(&Poly);
		}

		//вариант 5 - Vec0, Vec2 за плоскостью, Vec1 перед плоскостью
		if( (Vec0.z < 0) && (Vec1.z > 0) && (Vec2.z < 0 ) )
		{
			// Vec0 -> Vec1
			// Vec2 -> Vec1
			vector4 vt0 = Calc_Edge(Vec1, Vec0);
			vector4 vt1 = Calc_Edge(Vec1, Vec2);

			polygon Poly;

			Poly.Vertex[0] = vt0;
			Poly.Vertex[1] = Vec1;
			Poly.Vertex[2] = vt1;

			Poly.TexID = Tree->Polygons.PolyList[j].TexID; 

			FrontList.Add_To_List(&Poly);
		}

		//вариант 6 - Vec1, Vec2 за плоскостью, Vec0 перед плоскостью
		if( (Vec0.z > 0) && (Vec1.z < 0) && (Vec2.z < 0 ) )
		{
			// Vec2 -> Vec0
			// Vec1 -> Vec0
			vector4 vt0 = Calc_Edge(Vec0, Vec2);
			vector4 vt1 = Calc_Edge(Vec0, Vec1);

			polygon Poly;

			Poly.Vertex[0] = vt0;
			Poly.Vertex[1] = Vec0;
			Poly.Vertex[2] = vt1;

			Poly.TexID = Tree->Polygons.PolyList[j].TexID; 

			FrontList.Add_To_List(&Poly);
		}
		
		//отсечение в пространстве вида сделано
		//можно дальше трансформировать вершины
		//умножаем вершины на матрицу проекции
		//и далее деление на z, и экранные координаты
		//список полигонов FrontList может содержать
		//либо один либо два полигона получившихся после
		//отсечения передней плоскостью отсечения, или
		//если небыло отсечения один полигон
		for ( UINT i = 0; i < FrontList.PolygonCount; i++)
		{
			vector4 Vec0, Vec1, Vec2;

			Vec0 = FrontList.PolyList[i].Vertex[0];
			Vec1 = FrontList.PolyList[i].Vertex[1];
			Vec2 = FrontList.PolyList[i].Vertex[2];

			//умножаем вершины на матрицу проекции
			Vec4_Mat4x4_Mul(Vec0, Vec0, m_MatProj);
			Vec4_Mat4x4_Mul(Vec1, Vec1, m_MatProj);
			Vec4_Mat4x4_Mul(Vec2, Vec2, m_MatProj);

			//после умножения вершин на матрицу проекции
			//деление на z
			Vec0.x = Vec0.x / Vec0.w;
			Vec0.y = Vec0.y / Vec0.w;
			
			Vec1.x = Vec1.x / Vec1.w;
			Vec1.y = Vec1.y / Vec1.w;

			Vec2.x = Vec2.x / Vec2.w;
			Vec2.y = Vec2.y / Vec2.w;

			//после деления на z
			//преобразование вершин в экранные координаты
			Vec0.x = Vec0.x * m_ViewWidth / 2.0f + m_ViewWidth / 2.0f;
			Vec0.y = -Vec0.y * m_ViewHeight / 2.0f + m_ViewHeight / 2.0f;

			Vec1.x = Vec1.x * m_ViewWidth / 2.0f + m_ViewWidth / 2.0f;
			Vec1.y = -Vec1.y * m_ViewHeight / 2.0f + m_ViewHeight / 2.0f;

			Vec2.x = Vec2.x * m_ViewWidth / 2.0f + m_ViewWidth / 2.0f;
			Vec2.y = -Vec2.y * m_ViewHeight / 2.0f + m_ViewHeight / 2.0f;

			polygon Poly;

			Poly.TexID = FrontList.PolyList[i].TexID;

			Poly.Vertex[0] = Vec0;
			Poly.Vertex[1] = Vec1;
			Poly.Vertex[2] = Vec2;

			//добавляем трансформированный полигон
			//в список трансформированных полигонов
			Tree->TransformedPolygons.Add_To_List(&Poly);
		}

		//очищаем список полигонов
		if(FrontList.PolyList != NULL)
		{
			FrontList.PolygonCount = 0;
			free(FrontList.PolyList);
			FrontList.PolyList = NULL;
		}

	}

	//рекурсивно трансформируем все дерево
	Transform_BSP_Tree (Tree->Front);
	Transform_BSP_Tree (Tree->Back);
}

//**************************************
//Функция выводит BSP дерево на экран
//**************************************
void CMeshManager::Draw_BSP_Tree (BSP_tree *Tree, vector4 Eye)
{
	if(!Tree)
		return;

	float Result = Tree->Partition.Classify_Point (Eye);
	
	if (Result > 0)
	{
		Draw_BSP_Tree (Tree->Back, Eye);
		Draw_Polygon_List (Tree->TransformedPolygons);
		Draw_BSP_Tree (Tree->Front, Eye);
	}
	else if (Result < 0)
	{
		Draw_BSP_Tree (Tree->Front, Eye);
		Draw_Polygon_List (Tree->TransformedPolygons);
		Draw_BSP_Tree (Tree->Back, Eye);
	}
	else // Result is 0
	{
		// the Eye point is on the Partition plane...
		Draw_BSP_Tree (Tree->Front, Eye);
		Draw_BSP_Tree (Tree->Back, Eye);
	}
	
}

//***************************
//Функция запускает таймер
//***************************
void CMeshManager::Timer_Start()
{
	QueryPerformanceFrequency((LARGE_INTEGER*)& m_PerfFreq);
	QueryPerformanceCounter((LARGE_INTEGER*)& m_LastTime);
	m_StartTime = m_LastTime;
	m_TimeScale = 1.0f / m_PerfFreq;
}

//*****************************************
//Функция возвращает время пройденное за
//один кадр
//*****************************************
float CMeshManager::Get_Elapsed_Time()
{
	__int64 NowTime;
	QueryPerformanceCounter((LARGE_INTEGER*)& NowTime);
	m_ElapsedTime = (NowTime - m_StartTime) * m_TimeScale;
	m_StartTime = NowTime;
	return m_ElapsedTime;
}

//*************************************************
//Конструктор по умолчанию для структуры матрицы
//*************************************************
matrix4x4::matrix4x4()
{
};

//***********************************
//Деструктор для структуры матрицы
//***********************************
matrix4x4::~matrix4x4()
{
};

//******************************************
//Конструктор с параметрами для структуры
//матрицы
//******************************************
matrix4x4::matrix4x4(float ir1c1, float ir1c2, float ir1c3, float ir1c4,
	float ir2c1, float ir2c2, float ir2c3, float ir2c4,
	float ir3c1, float ir3c2, float ir3c3, float ir3c4,
	float ir4c1, float ir4c2, float ir4c3, float ir4c4)
{
	Mat[M00] = ir1c1; Mat[M01] = ir1c2; Mat[M02] = ir1c3; Mat[M03] = ir1c4;
	Mat[M10] = ir2c1; Mat[M11] = ir2c2; Mat[M12] = ir2c3; Mat[M13] = ir2c4;
	Mat[M20] = ir3c1; Mat[M21] = ir3c2; Mat[M22] = ir3c3; Mat[M23] = ir3c4;
	Mat[M30] = ir4c1; Mat[M31] = ir4c2; Mat[M32] = ir4c3; Mat[M33] = ir4c4;
};

//******************************************
//Функция умножает матрицу на матрицу
//******************************************
void CMeshManager::Mat4x4_Mat4x4_Mul(matrix4x4& MatOut, matrix4x4& Mat1, matrix4x4& Mat2)
{
	//row1 * col1
	MatOut.Mat[M00] = Mat1.Mat[M00] * Mat2.Mat[M00] + Mat1.Mat[M01] * Mat2.Mat[M10] + Mat1.Mat[M02] * Mat2.Mat[M20] + Mat1.Mat[M03] * Mat2.Mat[M30];
	//row1 * col2
	MatOut.Mat[M01] = Mat1.Mat[M00] * Mat2.Mat[M01] + Mat1.Mat[M01] * Mat2.Mat[M11] + Mat1.Mat[M02] * Mat2.Mat[M21] + Mat1.Mat[M03] * Mat2.Mat[M31];
	//row1 * col3
	MatOut.Mat[M02] = Mat1.Mat[M00] * Mat2.Mat[M02] + Mat1.Mat[M01] * Mat2.Mat[M12] + Mat1.Mat[M02] * Mat2.Mat[M22] + Mat1.Mat[M03] * Mat2.Mat[M32];
	//row1 * col4
	MatOut.Mat[M03] = Mat1.Mat[M00] * Mat2.Mat[M03] + Mat1.Mat[M01] * Mat2.Mat[M13] + Mat1.Mat[M02] * Mat2.Mat[M23] + Mat1.Mat[M03] * Mat2.Mat[M33];

	//row2 * col2
	MatOut.Mat[M10] = Mat1.Mat[M10] * Mat2.Mat[M00] + Mat1.Mat[M11] * Mat2.Mat[M10] + Mat1.Mat[M12] * Mat2.Mat[M20] + Mat1.Mat[M13] * Mat2.Mat[M30];
	//row2 * col2
	MatOut.Mat[M11] = Mat1.Mat[M10] * Mat2.Mat[M01] + Mat1.Mat[M11] * Mat2.Mat[M11] + Mat1.Mat[M12] * Mat2.Mat[M21] + Mat1.Mat[M13] * Mat2.Mat[M31];
	//row2 * col3
	MatOut.Mat[M12] = Mat1.Mat[M10] * Mat2.Mat[M02] + Mat1.Mat[M11] * Mat2.Mat[M12] + Mat1.Mat[M12] * Mat2.Mat[M22] + Mat1.Mat[M13] * Mat2.Mat[M32];
	//row2 * col4
	MatOut.Mat[M13] = Mat1.Mat[M10] * Mat2.Mat[M03] + Mat1.Mat[M11] * Mat2.Mat[M13] + Mat1.Mat[M12] * Mat2.Mat[M23] + Mat1.Mat[M13] * Mat2.Mat[M33];

	//row3 * col1
	MatOut.Mat[M20] = Mat1.Mat[M20] * Mat2.Mat[M00] + Mat1.Mat[M21] * Mat2.Mat[M10] + Mat1.Mat[M22] * Mat2.Mat[M20] + Mat1.Mat[M23] * Mat2.Mat[M30];
	//row3 * col2
	MatOut.Mat[M21] = Mat1.Mat[M20] * Mat2.Mat[M01] + Mat1.Mat[M21] * Mat2.Mat[M11] + Mat1.Mat[M22] * Mat2.Mat[M21] + Mat1.Mat[M23] * Mat2.Mat[M31];
	//row3 * col3
	MatOut.Mat[M22] = Mat1.Mat[M20] * Mat2.Mat[M02] + Mat1.Mat[M21] * Mat2.Mat[M12] + Mat1.Mat[M22] * Mat2.Mat[M22] + Mat1.Mat[M23] * Mat2.Mat[M32];
	//row3 * col4
	MatOut.Mat[M23] = Mat1.Mat[M20] * Mat2.Mat[M03] + Mat1.Mat[M21] * Mat2.Mat[M13] + Mat1.Mat[M22] * Mat2.Mat[M23] + Mat1.Mat[M23] * Mat2.Mat[M33];

	//row4 * col1
	MatOut.Mat[M30] = Mat1.Mat[M30] * Mat2.Mat[M00] + Mat1.Mat[M31] * Mat2.Mat[M10] + Mat1.Mat[M32] * Mat2.Mat[M20] + Mat1.Mat[M33] * Mat2.Mat[M30];
	//row4 * col2
	MatOut.Mat[M31] = Mat1.Mat[M30] * Mat2.Mat[M01] + Mat1.Mat[M31] * Mat2.Mat[M11] + Mat1.Mat[M32] * Mat2.Mat[M21] + Mat1.Mat[M33] * Mat2.Mat[M31];
	//row4 * col3
	MatOut.Mat[M32] = Mat1.Mat[M30] * Mat2.Mat[M02] + Mat1.Mat[M31] * Mat2.Mat[M12] + Mat1.Mat[M32] * Mat2.Mat[M22] + Mat1.Mat[M33] * Mat2.Mat[M32];
	//row4 * col4
	MatOut.Mat[M33] = Mat1.Mat[M30] * Mat2.Mat[M03] + Mat1.Mat[M31] * Mat2.Mat[M13] + Mat1.Mat[M32] * Mat2.Mat[M23] + Mat1.Mat[M33] * Mat2.Mat[M33];
}

//******************************************
//Функция умножает вектор на матрицу
//******************************************
void CMeshManager::Vec4_Mat4x4_Mul(vector4& VecOut, vector4& Vec, matrix4x4& Mat)
{
	VecOut.x =	Vec.x * Mat.Mat[M00] +
				Vec.y * Mat.Mat[M10] +
				Vec.z * Mat.Mat[M20] +
						Mat.Mat[M30];

	VecOut.y =	Vec.x * Mat.Mat[M01] +
				Vec.y * Mat.Mat[M11] +
				Vec.z * Mat.Mat[M21] +
						Mat.Mat[M31];

	VecOut.z =	Vec.x * Mat.Mat[M02] +
				Vec.y * Mat.Mat[M12] +
				Vec.z * Mat.Mat[M22] +
						Mat.Mat[M32];

	VecOut.w =	Vec.x * Mat.Mat[M03] +
				Vec.y * Mat.Mat[M13] +
				Vec.z * Mat.Mat[M23] +
						Mat.Mat[M33];

	VecOut.tu = Vec.tu;
	VecOut.tv = Vec.tv;
}

float CMeshManager::Vec4_Dot(vector4& Vec1, vector4& Vec2)
{
	return Vec1.x * Vec2.x + Vec1.y * Vec2.y + Vec1.z * Vec2.z;
}

void CMeshManager::Vec4_Cross(vector4& VecOut, vector4& Vec1, vector4& Vec2)
{
	VecOut.x = Vec1.y * Vec2.z - Vec1.z * Vec2.y;
	VecOut.y = Vec1.z * Vec2.x - Vec1.x * Vec2.z;
	VecOut.z = Vec1.x * Vec2.y - Vec1.y * Vec2.x;
}
