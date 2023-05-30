//======================================================================================
//	Ed Kurlyak 2023 Torus Lighting Software Rendering
//======================================================================================

#include "MeshManager.h"

CMeshManager::CMeshManager()
{
}

CMeshManager::~CMeshManager()
{
	Delete_BackBuffer();

	delete[] m_VertBuff;
	delete[] m_VertBuffTransformed;
	delete[] m_IndexBuff;
	delete[] m_NormalBuff;
	delete[] m_NormalBuffTransformed;
}

void CMeshManager::Mat4x4_Mat4x4_Mul(matrix4x4 MatOut, matrix4x4 Mat1, matrix4x4 Mat2)
{
	for (int Row = 0; Row < 4; Row++)
	{
		for (int Col = 0; Col < 4; Col++)
		{
			float Sum = 0.0;

			for (int Index = 0; Index < 4; Index++)
			{
				Sum += (Mat1[Row][Index] * Mat2[Index][Col]);
			}

			MatOut[Row][Col] = Sum;
		}
	}
}

void CMeshManager::Vec3_Mat4x4_Mul(vector3& VecOut, vector3& Vec, matrix4x4 Mat)
{
	for (int j = 0; j < 3; j++)
	{
		float Sum = 0.0f;
		int i;
		for (i = 0; i < 3; i++)
		{
			Sum += Vec.Mat[i] * Mat[i][j];
		}

		Sum += Mat[i][j];
		VecOut.Mat[j] = Sum;
	}
}

float CMeshManager::Vec3_Dot(vector3& Vec1, vector3& Vec2)
{
	return Vec1.xv * Vec2.xv + Vec1.yv * Vec2.yv + Vec1.zv * Vec2.zv;
}

void CMeshManager::Vec3_Normalize(vector3& VecOut, vector3& Vec)
{
	float Len = sqrtf( (Vec.xv * Vec.xv) + (Vec.yv * Vec.yv) + (Vec.zv * Vec.zv));

	VecOut.xv = Vec.xv / Len;
	VecOut.yv = Vec.yv / Len;
	VecOut.zv = Vec.zv / Len;
}

float CMeshManager::Vec3_Len(vector3& Vec)
{
	return sqrtf(Vec.xv * Vec.xv + Vec.yv * Vec.yv + Vec.zv * Vec.zv);
}

void CMeshManager::Init_MeshManager(HWND hWnd)
{
	m_hWnd = hWnd;

	Build_BackBuffer();

	UINT Rings = 24;
	UINT Sides = 12;

	m_TriangleCount = 2 * Sides * Rings;
    //одно дополнительное кольцо дл€ дублировани€ первого кольца
	m_VertCount  = Sides * (Rings+1);

	m_VertBuff = new vector3[m_VertCount];
	m_VertBuffTransformed = new vector3[m_VertCount];
	m_NormalBuff = new vector3[m_VertCount];
	m_NormalBuffTransformed = new vector3[m_VertCount];

	float OuterRadius = 10.0f; //внешний радиус (общий)
	float InnerRadius = 5.0f; //радиус трубки

	float RingFactor = TWOPI / Rings;
	float SideFactor = TWOPI / Sides;

	UINT Index = 0;

    for( UINT Ring = 0; Ring <= Rings; Ring++ )
    {
        float u = Ring * RingFactor;
        float cu = cosf(u);
        float su = sinf(u);

        for( UINT Side = 0; Side < Sides; Side++ )
        {
            float Vec = Side * SideFactor;
            float cv = cosf(Vec);
            float sv = sinf(Vec);
            float r = (OuterRadius + InnerRadius * cv);
            m_VertBuff[Index].xv = r * cu;
            m_VertBuff[Index].yv = r * su;
            m_VertBuff[Index].zv = InnerRadius * sv;
            
			m_NormalBuff[Index].xv = cv * cu * r;
            m_NormalBuff[Index].yv = cv * su * r;
            m_NormalBuff[Index].zv = sv * r;

			Vec3_Normalize(m_NormalBuff[Index], m_NormalBuff[Index]);

			//tex[Index].tu = u / PI2;
			//tex[Index].tv = Vec / PI2;

			Index ++;
        }
    }

	m_IndexBuff = new UINT[m_TriangleCount * 3];

    Index = 0;

    for( UINT Ring = 0; Ring < Rings; Ring++ )
    {
        int RingStart = Ring * Sides;
        int NextRingStart = (Ring + 1) * Sides;
    
        for( UINT Side = 0; Side < Sides; Side++ )
        {
            int NextSide = (Side+1) % Sides;
            // квадрат - два треугольника
            m_IndexBuff[Index] = (RingStart + Side);
            m_IndexBuff[Index+1] = (NextRingStart + Side);
            m_IndexBuff[Index+2] = (NextRingStart + NextSide);
            
			m_IndexBuff[Index+3] = RingStart + Side;
            m_IndexBuff[Index+4] = NextRingStart + NextSide;
            m_IndexBuff[Index+5] = (RingStart + NextSide);
            
            Index += 6;
        }
    }
	
	//создаем Z буфер
	m_ZBuff = new float*[m_ViewHeight];

	for (UINT i = 0; i < m_ViewHeight; i++)
	{
		m_ZBuff[i] = new float[m_ViewWidth];
	}
}

void CMeshManager::Update_MeshManager()
{
	static float Angle = 0.0f;

	matrix4x4 MatRotateX = {
		1, 0, 0, 0,
        0, cosf(Angle), sinf(Angle), 0,
        0, -sinf(Angle), cosf(Angle), 0,
        0, 0, 0, 1 };
	
	matrix4x4 MatRotateY = {
		cosf(Angle), 0, -sinf(Angle), 0,
		0, 1, 0, 0,
		sinf(Angle), 0, cosf(Angle), 0,
		0, 0, 0, 1};

	matrix4x4 MatRotateZ = {
		cosf(Angle), sinf(Angle), 0, 0,
		-sinf(Angle), cosf(Angle), 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1 };
	
	Angle += PI/175.0f;
    if(Angle > PI*2.0f)
		Angle = 0.0f;

	//при помощи этой матрицы можно
	//перемещать торус по сцене
	matrix4x4 MatWorld = {
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 30, 1 };

	matrix4x4 MatView = {
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1 };

	float Fov = PI/2.0f; // FOV 90 degree
    float Aspect = (float) m_ViewWidth / m_ViewHeight;
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

	for (UINT i = 0; i < m_VertCount; i++)
	{
		matrix4x4 MatTemp1, MatTemp2;
		Mat4x4_Mat4x4_Mul(MatTemp1, MatRotateX, MatRotateY);
		Mat4x4_Mat4x4_Mul(MatTemp2, MatTemp1, MatRotateZ);
		Mat4x4_Mat4x4_Mul(MatTemp1, MatTemp2, MatWorld);
		Mat4x4_Mat4x4_Mul(MatTemp2, MatTemp1, MatView);

		matrix4x4 MatNormal = {
			MatTemp2[0][0],	MatTemp2[0][1], MatTemp2[0][2], 0.0f,
			MatTemp2[1][0],	MatTemp2[1][1], MatTemp2[1][2], 0.0f,
			MatTemp2[2][0],	MatTemp2[2][1], MatTemp2[2][2], 0.0f,
			0.0f,			0.0f,			0.0f,			1.0f };

		//трансформируем нормали
		vector3 Norm;
		Vec3_Mat4x4_Mul(Norm, m_NormalBuff[i], MatNormal);
		Vec3_Normalize(Norm, Norm);
		m_NormalBuffTransformed[i] = Norm;

		//трансформируем вершины
		vector3 Vec;
		Vec3_Mat4x4_Mul(Vec, m_VertBuff[i], MatTemp2);

		vector3 VecS;
		Vec3_Mat4x4_Mul(VecS, Vec, MatProj);

		VecS.xs = VecS.xv / VecS.zv;
		VecS.ys = VecS.yv / VecS.zv;
		
		VecS.xs = VecS.xs * m_ViewWidth / 2.0f + m_ViewWidth / 2.0f;
		VecS.ys = -VecS.ys * m_ViewHeight / 2.0f + m_ViewHeight / 2.0f;

		Vec.xs = VecS.xs;
		Vec.ys = VecS.ys;

		m_VertBuffTransformed[i] = Vec;
	}
}

void CMeshManager::Draw_Color_Poly(int y1, int y2)
{
	float ri, gi, bi;
	float dr, dg, db;
	float zi, dz;

	for ( int yi = y1; yi < y2; yi++ )
	{

		ri = m_redl;
		gi = m_greenl;
		bi = m_bluel;
		zi = m_zl;
		
		if((m_xr - m_xl) > 0) // делить на 0 нельз€
		{
			dr = (m_redr - m_redl)/(m_xr - m_xl);
			dg = (m_greenr - m_greenl)/(m_xr - m_xl);
			db = (m_bluer - m_bluel)/(m_xr - m_xl);
			dz = (m_zr - m_zl)/(m_xr - m_xl);
		}
		else
		{
			dr = 0;
			dg = 0;
			db = 0;
			dz = 0;
		}

		for (int xi=(int)m_xl; xi < m_xr; xi++)
		{
			float fZVal = m_ZBuff[yi][xi];

			//если глубина fZVal в Z буфере меньше
			//чем глубина пиксел€ Z
			//не рисуем пиксель пропускаем его
			if( fZVal < zi )
			{	
				
				ri+=dr;
				gi+=dg;
				bi+=db;
				zi+=dz;

				continue;
			}

			m_ZBuff[yi][xi] = zi;
			
			UINT Index =  yi * 4 * m_ViewWidth + xi * 4;
			
			m_Data[Index] = (BYTE) bi; // blue
			m_Data[Index + 1] = (BYTE) gi; // green
			m_Data[Index + 2] = (BYTE) ri; // red
			m_Data[Index + 3] = 0; 

			ri+=dr;
			gi+=dg;
			bi+=db;
			zi+=dz;
		}

		m_xl+=m_dxl;
		m_redl+=m_dredl;
		m_greenl+=m_dgreenl;
		m_bluel+=m_dbluel;
		m_zl+=m_dzl;

		m_xr+=m_dxr;
		m_redr+=m_dredr;
		m_greenr+=m_dgreenr;
		m_bluer+=m_dbluer;
		m_zr+=m_dzr;
	}
}

void CMeshManager::Draw_Color_Triangle(float x1,float y1, float z1,
					   float x2,float y2, float z2,
					   float x3,float y3, float z3,
					   color_rgb Color1,
					   color_rgb Color2,
					   color_rgb Color3)
{
	float Temp;
	UINT TempRGB;
	UINT Side;

	if (y2 < y1)
	{
		SWAP(x2,x1,Temp);
		SWAP(y2,y1,Temp);
		SWAP(z2,z1,Temp);
		
		SWAP(Color2.r, Color1.r, TempRGB);
		SWAP(Color2.g, Color1.g, TempRGB);
		SWAP(Color2.b, Color1.b, TempRGB);
	}

	if (y3 < y1)
	{
		SWAP(x3,x1,Temp);
		SWAP(y3,y1,Temp);
		SWAP(z3,z1,Temp);

		SWAP(Color3.r, Color1.r, TempRGB);
		SWAP(Color3.g, Color1.g, TempRGB);
		SWAP(Color3.b, Color1.b, TempRGB);
	}

	if (y3 < y2)
	{
		SWAP(x3,x2,Temp);
		SWAP(y3,y2,Temp);
		SWAP(z3,z2,Temp);

		SWAP(Color3.r, Color2.r, TempRGB);
		SWAP(Color3.g, Color2.g, TempRGB);
		SWAP(Color3.b, Color2.b, TempRGB);
	}


	if (y2 > y1 && 	y3 > y2)
	{
		float dxdy1 = (x2 - x1) / (y2 - y1);
		float dxdy2 = (x3 - x1) / (y3 - y1);
		Side = dxdy2 > dxdy1;
	}

	if (y1 == y2)
		Side = x1 > x2;
	if (y2 == y3)
		Side = x3 > x2;

	if (!Side)	//длинее лева€ сторона
	{
		m_xl = x1;
		m_redl = (float) Color1.r;
		m_greenl = (float) Color1.g;
		m_bluel = (float) Color1.b;
		m_zl = z1;
		
		m_dxl = (x3 - x1) / (y3 - y1);
		m_dredl = (Color3.r - Color1.r) / (y3 - y1);
		m_dgreenl = (Color3.g - Color1.g) / (y3 - y1);
		m_dbluel = (Color3.b - Color1.b) / (y3 - y1);
		m_dzl = (z3 - z1) / (y3 - y1);
		
		if ( y1 < y2)
		{
			m_xr = x1;
			m_redr = (float) Color1.r;
			m_greenr = (float) Color1.g;
			m_bluer = (float) Color1.b;
			m_zr = z1;

			m_dxr = (x2 - x1) / (y2 - y1);
			m_dredr = (Color2.r - Color1.r) / (y2 - y1);
			m_dgreenr = (Color2.g - Color1.g) / (y2 - y1);
			m_dbluer = (Color2.b - Color1.b) / (y2 - y1);
			m_dzr = (z2 - z1) / (y2 - y1);

			Draw_Color_Poly((int)y1, (int)y2);	
		}
		if(y2 < y3)
		{
			m_xr = x2;
			m_redr = (float) Color2.r;
			m_greenr = (float) Color2.g;
			m_bluer = (float) Color2.b;
			m_zr = z2;

			m_dxr = (x3 - x2) / (y3 - y2);
			m_dredr = (Color3.r - Color2.r) / (y3 - y2);
			m_dgreenr = (Color3.g - Color2.g) / (y3 - y2);
			m_dbluer = (Color3.b - Color2.b) / (y3 - y2);
			m_dzr = (z3 - z2) / (y3 - y2);

			Draw_Color_Poly((int)y2, (int)y3);
		}
	}
	else
	{
		m_xr = x1;
		m_redr = (float) Color1.r;
		m_greenr = (float) Color1.g;
		m_bluer = (float) Color1.b;
		m_zr = z1;
		
		m_dxr = (x3 - x1) / (y3 - y1);
		m_dredr = (Color3.r - Color1.r) / (y3 - y1);
		m_dgreenr = (Color3.g - Color1.g) / (y3 - y1);
		m_dbluer = (Color3.b - Color1.b) / (y3 - y1);
		m_dzr = (z3 - z1) / (y3 - y1);

		if (y1 < y2)
		{
			m_xl = x1;
			m_redl = (float) Color1.r;
			m_greenl = (float) Color1.g;
			m_bluel = (float) Color1.b;
			m_zl = z1;

			m_dxl = (x2 - x1) / (y2 - y1);
			m_dredl = (Color2.r - Color1.r) / (y2 - y1);
			m_dgreenl = (Color2.g - Color1.g) / (y2 - y1);
			m_dbluel = (Color2.b - Color1.b) / (y2 - y1);
			m_dzl = (z2 - z1) / (y2 - y1);

			Draw_Color_Poly((int)y1, (int)y2);
		}
		if (y2 < y3)
		{
			m_xl = x2;
			m_redl = (float) Color2.r;
			m_greenl = (float) Color2.g;
			m_bluel = (float) Color2.b;
			m_zl = z2;

			m_dxl = (x3 - x2) / (y3 - y2);
			m_dredl = (Color3.r - Color2.r) / (y3 - y2);
			m_dgreenl = (Color3.g - Color2.g) / (y3 - y2);
			m_dbluel = (Color3.b - Color2.b) / (y3 - y2);
			m_dzl = (z3 - z2) / (y3 - y2);
			
			Draw_Color_Poly((int)y2, (int)y3);
		}
	}
}

void CMeshManager::Draw_MeshManager ()
{
	//очищаем Z-Buffer
	for (UINT i = 0; i < m_ViewHeight; i++)
	{
		for(UINT j = 0; j < m_ViewWidth; j++)
		{
			//дальн€€ плоскость отсечени€ 25000.0f
			//это максимальна€ глубина Z
			//мы этой глубиной очищаем Z-Buffer
			m_ZBuff[i][j] = 25000.0f;
		}
	}

	Clear_BackBuffer();

    for (UINT i = 0; i < m_TriangleCount; i++)
    {
		vector3 Vec1 = m_VertBuffTransformed[m_IndexBuff[i * 3]];
		vector3 Vec2 = m_VertBuffTransformed[m_IndexBuff[i * 3 + 1]];
		vector3 Vec3 = m_VertBuffTransformed[m_IndexBuff[i * 3 + 2]];

		vector3 Norm1 = m_NormalBuffTransformed[m_IndexBuff[i * 3]];
		vector3 Norm2 = m_NormalBuffTransformed[m_IndexBuff[i * 3 + 1]];
		vector3 Norm3 = m_NormalBuffTransformed[m_IndexBuff[i * 3 + 2]];
		
		color_rgb rgb1;
		color_rgb rgb2;
		color_rgb rgb3;

		vector3 VecPosLight = vector3(-25.0f, 0.0f, -25.0f);
		vector3 DiffLightColor= vector3(255.0f, 255.0f, 128.0f);

		vector3 VecLightDir1 = VecPosLight - Vec1;
		vector3 VecLightDir2 = VecPosLight - Vec2;
		vector3 VecLightDir3 = VecPosLight - Vec3;

		float Dist1 = Vec3_Len(VecLightDir1);
		float Dist2 = Vec3_Len(VecLightDir2);
		float Dist3 = Vec3_Len(VecLightDir3);

		Vec3_Normalize(VecLightDir1, VecLightDir1);
		Vec3_Normalize(VecLightDir2, VecLightDir2);
		Vec3_Normalize(VecLightDir3, VecLightDir3);
	
		float DotProd1 = Vec3_Dot(Norm1,VecLightDir1);
		float DotProd2 = Vec3_Dot(Norm2,VecLightDir2);
		float DotProd3 = Vec3_Dot(Norm3,VecLightDir3);

		// Attenuate
		vector3 Att = vector3(0.0f, 0.02f, 0.0f);

		vector3 AttFac1 = vector3(1.0f, Dist1, Dist1*Dist1);
		vector3 AttFac2 = vector3(1.0f, Dist2, Dist2*Dist2);
		vector3 AttFac3 = vector3(1.0f, Dist3, Dist3*Dist3);

		float Att1 = 1.0f / Vec3_Dot(Att, AttFac1);
		float Att2 = 1.0f / Vec3_Dot(Att, AttFac2);
		float Att3 = 1.0f / Vec3_Dot(Att, AttFac3);
	
		//вершина имеет черный цвет если cos<=0
		if(DotProd1 <= 0)
		{
			rgb1 = color_rgb(0,0,0);
		}
		else
		{
			rgb1 = DiffLightColor * Att1 * DotProd1;
		}
		
		//вершина имеет черный цвет если cos<=0
		if(DotProd2 <= 0)
		{
			rgb2 = color_rgb(0,0,0);
		}
		else
		{
			rgb2 = DiffLightColor * Att2 * DotProd2;
		}
		
		//вершина имеет черный цвет если cos<=0
		if(DotProd3 <= 0)
		{
			rgb3 = color_rgb(0,0,0);
		}
		else
		{
			rgb3 = DiffLightColor * Att3 * DotProd3;
		}

		//добавл€ем ambient цвет
		rgb1 = rgb1 + color_rgb(24, 24, 12);
		//делаем clamp
		if(rgb1.r > 255)
			rgb1.r = 255;

		if(rgb1.g > 255)
			rgb1.g = 255;

		if(rgb1.b > 255)
			rgb1.b = 255;

		//добавл€ем ambient цвет
		rgb2 = rgb2 + color_rgb(24, 24, 12);
		//делаем clamp
		if(rgb2.r > 255)
			rgb2.r = 255;

		if(rgb2.g > 255)
			rgb2.g = 255;

		if(rgb2.b > 255)
			rgb2.b = 255;

		//добавл€ем ambient цвет
		rgb3 = rgb3 + color_rgb(24, 24, 12);
		//делаем clamp
		if(rgb3.r > 255)
			rgb3.r = 255;

		if(rgb3.g > 255)
			rgb3.g = 255;

		if(rgb3.b > 255)
			rgb3.b = 255;

		//используем псевдоскал€рное (косое) умножение векторов
		//дл€ отбрасывани€ задних поверхностей
		float s = (Vec2.xs - Vec1.xs) * (Vec3.ys - Vec1.ys) - (Vec2.ys - Vec1.ys) * (Vec3.xs - Vec1.xs);

		if (s <= 0)
			continue;

		Draw_Color_Triangle(Vec1.xs, Vec1.ys, Vec1.zv,
							Vec2.xs, Vec2.ys, Vec2.zv,
							Vec3.xs, Vec3.ys, Vec3.zv,
							rgb1, rgb2, rgb3);
     }  

	Present_BackBuffer();
}

void CMeshManager::Build_BackBuffer()
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

void CMeshManager::Clear_BackBuffer()
{
	for ( UINT i = 0; i <  m_ViewHeight; i++)
	{
		for ( UINT j = 0; j < m_ViewWidth; j++ )
		{
			int Index = i * 4 * m_ViewWidth + j * 4;

			m_Data[Index] = (BYTE) (255.0 * 0.3f); // blue
			m_Data[Index + 1] = (BYTE)(255.0 * 0.125f); // green
			m_Data[Index + 2] = 0; // red

			m_Data[Index + 3] = 0; 
		}
	}
}

void CMeshManager::Delete_BackBuffer()
{
	DrawDibClose(m_hDD);

	free(m_Data);
	m_Data = NULL;
}

void CMeshManager::Present_BackBuffer( )
{    
	//правильно было бы перевернуть задний буфер
	HDC hDC = GetDC(m_hWnd);
	DrawDibDraw(m_hDD, hDC, 0, 0, m_ViewWidth, m_ViewHeight, &m_Bih, m_Data, 0, 0, m_ViewWidth, m_ViewHeight, 0);
	ReleaseDC(m_hWnd, hDC);
}

