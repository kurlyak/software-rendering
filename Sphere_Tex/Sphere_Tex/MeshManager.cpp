//======================================================================================
//	Ed Kurlyak 2023 Textured Sphere Mesh Software
//======================================================================================

#include "MeshManager.h"

CMeshManager::CMeshManager()
{
}

CMeshManager::~CMeshManager()
{
	Delete_Backbuffer();

	delete [] m_VertBuff;
	delete [] m_VertBuffTransformed;
	delete [] m_IndexBuff;
	delete [] m_TexCoord;

	delete [] m_Res;
}

vector3 CMeshManager::Vec3_Mat4x4_Mul(vector3& VecIn, matrix4x4 MatIn)
{
	vector3 VecOut;

	for ( int j = 0; j < 3; j++)
	{
		float Sum = 0.0f;
		int i;
		for ( i = 0; i < 3; i++)
		{
			Sum += VecIn.Vec[i] * MatIn[i][j];
		}
		
		Sum += MatIn[i][j];
		VecOut.Vec[j] = Sum;
	}

	return VecOut;
}

void CMeshManager::Init_MeshManager(HWND hWnd)
{
	m_hWnd = hWnd;

	Read_BMP_File(".//texture.bmp");
	
	Create_Backbuffer();
	
	int Stacks = 10;
	int Slices = 20;
	//float TWOPI  = 6.28318530717958f;
	//float PI     = 3.14159265358979f;
	float ThetaFac = PI2 / (float)Slices;
	float PhiFac = PI / (float)Stacks;
	float Radius = 8.0f;

	m_VertCount =  (Slices + 1) * (Stacks + 1);
	m_TriangleCount =  Slices * Stacks * 2;

	m_VertBuff = new vector3[m_VertCount];
	m_TexCoord = new tex_coord2[m_VertCount];
	m_IndexBuff = new DWORD[m_TriangleCount  * 3];
	m_VertBuffTransformed = new vector3[m_VertCount];

	int Index = 0;
	
	for(int l = 0; l <= Slices; l++)
	{
		for(int b = 0; b <= Stacks; b++)
		{
			float sb = PhiFac * b;
			float sl = ThetaFac * l;

			m_VertBuff[Index].x = Radius * sinf(sb) * sinf(sl);
			m_VertBuff[Index].y = Radius * cosf(sb);
			m_VertBuff[Index].z = Radius * sinf(sb) * cosf(sl);

			m_TexCoord[Index].tu = (m_TextureWidth - 1) - (int) (( (float)l / (float)(Slices) ) * (float)(m_TextureWidth - 1) );
			m_TexCoord[Index].tv = (int) (( (float)b / (float)(Stacks) ) * (float)(m_TextureHeight - 1) );

			Index++;
		}
	}

	Index = 0;

	for ( int l = 0; l < Slices; l++ )
	{
		for ( int b = 0; b < Stacks; b++ )
		{
			int Next = l * (Stacks + 1) + b;
			int NxtSection = (l + 1) * (Stacks + 1) + b;

			m_IndexBuff[Index] = Next;
			m_IndexBuff[Index + 1] = Next + 1;
			m_IndexBuff[Index + 2] = NxtSection + 1;

			m_IndexBuff[Index + 3] = Next;
			m_IndexBuff[Index + 4] = NxtSection + 1;
			m_IndexBuff[Index + 5] = NxtSection;

			Index += 6;
		}
	}
}

void CMeshManager::Update_MeshManager()
{
	static float Angle = 0.0f;
	
	matrix4x4 MatRotateX = {
		1, 0, 0, 0,
        0, cosf(Angle), sinf(Angle), 0,
        0,-sinf(Angle),  cosf(Angle), 0,
        0, 0, 0, 1 };

	matrix4x4 MatRotateY = {
		cosf(Angle), 0, -sinf(Angle), 0,
		0, 1, 0, 0,
		sinf(Angle), 0, cosf(Angle), 0,
		0, 0, 0, 1 };

	Angle += PI/100.0f;
    if(Angle > PI2)
		Angle = 0;

	//при помощи этой матрицы можно
	//перемещать сферу по сцене
	matrix4x4 MatWorld = {
		1, 0, 0, 0,
		0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 15.0f, 1 };
	
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
    matrix4x4 MatProj = {
		w, 0, 0, 0,
        0, h, 0, 0,
        0, 0, Q, 1,
        0, 0, -Q*ZNear, 0 };
    */

    matrix4x4 MatProj = {
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
		VecTemp = Vec3_Mat4x4_Mul(VecTemp, MatWorld);
		VecTemp = Vec3_Mat4x4_Mul(VecTemp, MatProj);

		VecTemp.x = VecTemp.x / VecTemp.z;
		VecTemp.y = VecTemp.y / VecTemp.z;

		VecTemp = Vec3_Mat4x4_Mul(VecTemp, MatScreen);

		m_VertBuffTransformed[i] = VecTemp;
	}
}

void CMeshManager::Draw_MeshManager ()
{
	Clear_Backbuffer();

    for (UINT i = 0; i < m_TriangleCount; i++)
    {
		vector3 Vec1 = m_VertBuffTransformed[m_IndexBuff[i * 3 + 0]];
		vector3 Vec2 = m_VertBuffTransformed[m_IndexBuff[i * 3 + 1]];
		vector3 Vec3 = m_VertBuffTransformed[m_IndexBuff[i * 3 + 2]];

		tex_coord2 Tex1 = m_TexCoord[m_IndexBuff[i * 3 + 0]];
		tex_coord2 Tex2 = m_TexCoord[m_IndexBuff[i * 3 + 1]];
		tex_coord2 Tex3 = m_TexCoord[m_IndexBuff[i * 3 + 2]];

		//используем псевдоскал€рное (косое) умножение векторов
		//дл€ отбрасывани€ задних поверхностей
		float s = (Vec2.x - Vec1.x) * ( Vec3.y - Vec1.y) - (Vec2.y - Vec1.y) * (Vec3.x - Vec1.x);

		if(s <= 0)
			continue;

		Draw_Textured_Triangle(Vec1, Tex1, Vec2, Tex2, Vec3, Tex3 );
     }  

	Present_Backbuffer();
}

void CMeshManager::Read_BMP_File(const char *TexFileName)
{
	FILE *Fp;

	fopen_s(&Fp, TexFileName,"rb");
	if(Fp==NULL)
		MessageBox(NULL, "Error Open File", "INFO", MB_OK);

	BITMAPFILEHEADER bfh;
	fread(&bfh, sizeof(bfh), 1, Fp);

	BITMAPINFOHEADER bih;
	fread(&bih, sizeof(bih), 1, Fp);
	
	fseek(Fp, bfh.bfOffBits, SEEK_SET);

	m_Res = new unsigned char [bih.biWidth*bih.biHeight*3];
	fread(m_Res,bih.biWidth*bih.biHeight*3,1,Fp);

	fclose(Fp);

	m_TextureWidth = bih.biWidth;
	m_TextureHeight = bih.biHeight;
}

void CMeshManager::Draw_Textured_Triangle(vector3 VecIn1, tex_coord2 TexIn1,
						  vector3 VecIn2, tex_coord2 TexIn2,
						  vector3 VecIn3, tex_coord2 TexIn3)
{
	int Side;
	float x1, x2, x3;
	float y1, y2, y3;
	float iz1, uiz1, viz1, iz2, uiz2, viz2, iz3, uiz3, viz3;
	float Temp;
	
	x1 = VecIn1.x;
	y1 = VecIn1.y;
	x2 = VecIn2.x;
	y2 = VecIn2.y;
	x3 = VecIn3.x;
	y3 = VecIn3.y;
	
	iz1 = 1.0f / VecIn1.z;
	iz2 = 1.0f / VecIn2.z;
	iz3 = 1.0f / VecIn3.z;
	
	uiz1 = TexIn1.tu * iz1;
	viz1 = TexIn1.tv * iz1;
	uiz2 = TexIn2.tu * iz2;
	viz2 = TexIn2.tv * iz2;
	uiz3 = TexIn3.tu * iz3;
	viz3 = TexIn3.tv * iz3;

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

	if (!Side) //длинее лева€ сторона
	{
		m_xl = x1;
		m_ul = uiz1;
		m_vl = viz1;
		m_zl = iz1;

		m_dxdyl = (x3 - x1) / (y3 - y1);
		m_dudyl = (uiz3 - uiz1) / (y3 - y1);
		m_dvdyl = (viz3 - viz1) / (y3 - y1);
		m_dzdyl = (iz3 - iz1) / (y3 - y1);
		
		if (y1 < y2)
		{
			m_xr = x1;
			m_ur = uiz1;
			m_vr = viz1;
			m_zr = iz1;

			m_dxdyr = (x2 - x1) / (y2 - y1);
			m_dudyr = (uiz2 - uiz1) / (y2 - y1);
			m_dvdyr = (viz2 - viz1) / (y2 - y1);
			m_dzdyr = (iz2 - iz1) / (y2 - y1);
			
			Draw_Textured_Poly((int)y1, (int)y2);

		}
		if (y2 < y3)
		{
			m_xr = x2;
			m_ur = uiz2;
			m_vr = viz2;
			m_zr = iz2;

			m_dxdyr = (x3 - x2) / (y3 - y2);
			m_dudyr = (uiz3 - uiz2) / (y3 - y2);
			m_dvdyr = (viz3 - viz2) / (y3 - y2);
			m_dzdyr = (iz3 - iz2) / (y3 - y2);
			
			Draw_Textured_Poly((int)y2, (int)y3);
		}
	}
	else
	{
		m_xr = x1;
		m_ur = uiz1;
		m_vr = viz1;
		m_zr = iz1;
		
		m_dxdyr = (x3 - x1) / (y3 - y1);
		m_dudyr = (uiz3 - uiz1) / (y3 - y1);
		m_dvdyr = (viz3 - viz1) / (y3 - y1);
		m_dzdyr = (iz3 - iz1) / (y3 - y1);
				
		if (y1 < y2)
		{
			m_xl = x1;
			m_ul = uiz1;
			m_vl = viz1;
			m_zl = iz1;

			m_dxdyl = (x2 - x1) / (y2 - y1);
			m_dudyl = (uiz2 - uiz1) / (y2 - y1);
			m_dvdyl = (viz2 - viz1) / (y2 - y1);
			m_dzdyl = (iz2 - iz1) / (y2 - y1);
			
			Draw_Textured_Poly((int)y1, (int)y2);
		}
		if (y2 < y3)
		{
			m_xl = x2;
			m_ul = uiz2;
			m_vl = viz2;
			m_zl = iz2;

			m_dxdyl = (x3 - x2) / (y3 - y2);
			m_dudyl = (uiz3 - uiz2) / (y3 - y2);
			m_dvdyl = (viz3 - viz2) / (y3 - y2);
			m_dzdyl = (iz3 - iz2) / (y3 - y2);
						
			Draw_Textured_Poly((int)y2, (int)y3);
		}
	}
}

void CMeshManager::Draw_Textured_Poly(int y1, int y2)
{
	float ui, vi, zi;
	float du, dv, dz;
	
	for (int y = y1; y<y2; y++)
	{
		if ((m_xr - m_xl)>0)
		{
			du = (m_ur - m_ul)/(m_xr - m_xl);
			dv = (m_vr - m_vl)/(m_xr - m_xl);
			dz = (m_zr - m_zl)/(m_xr - m_xl);
		}
		else
		{
			du = 0;
			dv = 0;
			dz = 0;
		}

		int xln = (int)m_xl;

		float dxt = 1 - (m_xl - xln);

		zi = m_zl + dxt * dz;
		ui = m_ul + dxt * du;
		vi = m_vl + dxt * dv;
				
		for (int x=(int)m_xl; x<(int)m_xr; x++)
		{
			float z = 1.0f/zi;
			float u = ui * z;
			float Vec = vi * z;

			UINT t = (int)u  + (((int)Vec) * m_TextureWidth);
			
			if ( t < 0 || t > (m_TextureWidth * m_TextureHeight - 1) )
				continue;

			t= t*3;

			UINT Index =  y * 4 * m_ViewWidth + x * 4;

			m_Data[Index + 0] = (BYTE) m_Res[t + 0]; // blue
			m_Data[Index + 1] = (BYTE) m_Res[t + 1]; // green
			m_Data[Index + 2] = (BYTE) m_Res[t + 2]; // red
			m_Data[Index + 3] = 0; 
				
			ui+=du;
			vi+=dv;
			zi+=dz;
		}

		m_xl+=m_dxdyl;
		m_ul+=m_dudyl;
		m_vl+=m_dvdyl;
		m_zl+=m_dzdyl;

		m_xr+=m_dxdyr;
		m_ur+=m_dudyr;
		m_vr+=m_dvdyr;
		m_zr+=m_dzdyr;
	}
}

void CMeshManager::Create_Backbuffer()
{
	RECT Rc;
	::GetClientRect(m_hWnd, &Rc);

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

void CMeshManager::Clear_Backbuffer()
{
	for ( UINT i = 0; i <  m_ViewHeight; i++)
	{
		for ( UINT j = 0; j < m_ViewWidth; j++ )
		{
			int Index = i * 4 * m_ViewWidth + j * 4;

			m_Data[Index + 0] = (BYTE) (255.0 * 0.3f); // blue
			m_Data[Index + 1] = (BYTE)(255.0 * 0.125f); // green
			m_Data[Index + 2] = 0; // red
			m_Data[Index + 3] = 0; 
		}
	}
}

void CMeshManager::Present_Backbuffer()
{
	//переворачиваем задний буфер
	DWORD m_dwSize = m_ViewWidth * (BITS_PER_PIXEL >> 3) * m_ViewHeight;
	
	LPBYTE m_DataTemp = (LPBYTE)malloc(m_dwSize * sizeof(BYTE));

	for (UINT h = 0; h < m_ViewHeight; h++ )
	{
		for (UINT w = 0; w < m_ViewWidth; w++)
		{
			int Index = h * 4 * m_ViewWidth + w * 4;

			BYTE b = m_Data[Index]; // blue
			BYTE g = m_Data[Index + 1]; // green
			BYTE Radius = m_Data[Index + 2]; // red
			
			int indx_temp = (m_ViewHeight - 1 - h) * 4 * m_ViewWidth + w * 4;
			m_DataTemp[indx_temp] = b;
			m_DataTemp[indx_temp + 1] = g;
			m_DataTemp[indx_temp + 2] = Radius;
			m_DataTemp[indx_temp + 3] = 0;
		}
	}

	HDC hDC = GetDC(m_hWnd);
	DrawDibDraw(m_hDD, hDC, 0, 0, m_ViewWidth, m_ViewHeight, &m_Bih, m_DataTemp, 0, 0, m_ViewWidth, m_ViewHeight, 0);
	ReleaseDC(m_hWnd, hDC);

	free(m_DataTemp);
}
	
void CMeshManager::Delete_Backbuffer()
{
	DrawDibClose(m_hDD);

	free(m_Data);
	m_Data = NULL;
}
