//======================================================================================
//	Ed Kurlyak 2023 Software Rendering Textured Cube DirectX12 Backbuffer
//======================================================================================

#ifndef _MESHMANAGER_
#define _MESHMANAGER_

#include <windows.h>
#include <windowsx.h>
#include <math.h>
#include <stdio.h>

#include <wrl.h>
#include <dxgi1_4.h>
#include <d3d12.h>
#include "d3dx12.h"
#include <comdef.h>

#include "d3dUtil.h"

#include "Timer.h"

#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

#define PI 3.14159265358979f

typedef float matrix4x4[4][4];

struct tex_coord2
{
	int tu, tv;
};

struct vector3
{
	vector3() {};
	vector3(float xi, float yi, float zi) : x(xi), y(yi), z(zi) {};

	union
	{
		float Vec[3];
		struct {
			float x, y, z;
		};
	};

	vector3& operator = (const vector3& v_in)
	{
		x = v_in.x;
		y = v_in.y;
		z = v_in.z;

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
	void EnableDebugLayer_CreateFactory();
	void Create_Device();
	void CreateFence_GetDescriptorsSize();
	void Check_Multisample_Quality();
	void Create_CommandList_Allocator_Queue();
	void Create_SwapChain();
	void Create_RtvAndDsv_DescriptorHeaps();
	void Resize_SwapChainBuffers();
	void FlushCommandQueue();
	void Create_RenderTarget();
	void Create_DepthStencil_Buff_And_View();
	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView();
	void Execute_Init_Commands();
	void Update_ViewPort_And_Scissor();
	D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView();
	ID3D12Resource* CurrentBackBuffer();
	
	void Read_BMP_File(const char* szTexFileName);
	vector3 Vec3_Mat4x4_Mul(vector3 &VecIn, matrix4x4 MatIn);
	void Draw_Textured_Triangle(vector3 VecIn1, tex_coord2 TexIn1,
		vector3 VecIn2, tex_coord2 TexIn2,
		vector3 VecIn3, tex_coord2 TexIn3);
	void Draw_Textured_Poly(int y1, int y2);
	
	void Draw_MeshManager2();
	
	CTimer m_Timer;

	UCHAR* m_Res;

	UINT m_VertCount;
	UINT m_TriangleCount;

	vector3* m_VertBuff;
	vector3* m_VertBuffTransformed;
	tex_coord2* m_TexCoord;
	DWORD* m_IndexBuff;

	float m_dxdyl, m_dudyl, m_dvdyl, m_dzdyl,
		m_dxdyr, m_dudyr, m_dvdyr, m_dzdyr;
	float m_ul, m_ur,
		m_vl, m_vr,
		m_zl, m_zr,
		m_xl, m_xr;

	int m_TextureWidth;
	int m_TextureHeight;

	unsigned char * m_BackBuffer;
	Microsoft::WRL::ComPtr<ID3D12Resource> UploadHeap = nullptr;

	Microsoft::WRL::ComPtr<IDXGIFactory4> m_dxgiFactory;
	Microsoft::WRL::ComPtr<ID3D12Device> m_d3dDevice;
	Microsoft::WRL::ComPtr<ID3D12Fence> m_Fence;

	UINT m_RtvDescriptorSize = 0;
	UINT m_DsvDescriptorSize = 0;
	UINT m_CbvSrvUavDescriptorSize = 0;

	DXGI_FORMAT m_BackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

	bool      m_4xMsaaState = false;
	UINT      m_4xMsaaQuality = 0;

	Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_CommandQueue;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_DirectCmdListAlloc;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_CommandList;

	Microsoft::WRL::ComPtr<IDXGISwapChain> m_SwapChain;

	int m_ClientWidth = 800;
	int m_ClientHeight = 600;

	static const int m_SwapChainBufferCount = 2;

	HWND m_hWnd;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_RtvHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_DsvHeap;

	Microsoft::WRL::ComPtr<ID3D12Resource> m_SwapChainBuffer[m_SwapChainBufferCount];
	Microsoft::WRL::ComPtr<ID3D12Resource> m_DepthStencilBuffer;

	Microsoft::WRL::ComPtr<ID3D12Resource> TexBackBuffer;

	UINT64 m_CurrentFence = 0;
	int m_CurrBackBuffer = 0;

	DXGI_FORMAT m_DepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	D3D12_VIEWPORT m_ScreenViewport;
	D3D12_RECT m_ScissorRect;
};

#endif