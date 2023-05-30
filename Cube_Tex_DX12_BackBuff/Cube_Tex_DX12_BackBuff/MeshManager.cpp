//======================================================================================
//	Ed Kurlyak 2023 Software Rendering Textured Cube DirectX12 Backbuffer
//======================================================================================

#include "MeshManager.h"

CMeshManager::CMeshManager()
{
}

CMeshManager::~CMeshManager()
{
	if (m_d3dDevice != nullptr)
		FlushCommandQueue();
}

void CMeshManager::EnableDebugLayer_CreateFactory()
{
#if defined(DEBUG) || defined(_DEBUG) 
	{
		Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
		ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
		debugController->EnableDebugLayer();
	}
#endif

	ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&m_dxgiFactory)));
}

void CMeshManager::Create_Device()
{
	HRESULT hardwareResult = D3D12CreateDevice(
		nullptr,             // default adapter
		D3D_FEATURE_LEVEL_11_0,
		IID_PPV_ARGS(&m_d3dDevice));

	// Fallback to WARP device.
	if (FAILED(hardwareResult))
	{
		Microsoft::WRL::ComPtr<IDXGIAdapter> pWarpAdapter;
		ThrowIfFailed(m_dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&pWarpAdapter)));

		ThrowIfFailed(D3D12CreateDevice(
			pWarpAdapter.Get(),
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&m_d3dDevice)));
	}
}

void CMeshManager::CreateFence_GetDescriptorsSize()
{
	ThrowIfFailed(m_d3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE,
		IID_PPV_ARGS(&m_Fence)));

	m_RtvDescriptorSize = m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	m_DsvDescriptorSize = m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	m_CbvSrvUavDescriptorSize = m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void CMeshManager::Check_Multisample_Quality()
{
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
	msQualityLevels.Format = m_BackBufferFormat;
	msQualityLevels.SampleCount = 4;
	msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	msQualityLevels.NumQualityLevels = 0;
	ThrowIfFailed(m_d3dDevice->CheckFeatureSupport(
		D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
		&msQualityLevels,
		sizeof(msQualityLevels)));

	m_4xMsaaQuality = msQualityLevels.NumQualityLevels;
	assert(m_4xMsaaQuality > 0 && "Unexpected MSAA quality level.");
}

void CMeshManager::Create_CommandList_Allocator_Queue()
{
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	ThrowIfFailed(m_d3dDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_CommandQueue)));

	ThrowIfFailed(m_d3dDevice->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(m_DirectCmdListAlloc.GetAddressOf())));

	ThrowIfFailed(m_d3dDevice->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		m_DirectCmdListAlloc.Get(),
		nullptr,
		IID_PPV_ARGS(m_CommandList.GetAddressOf())));

	m_CommandList->Close();

}

void CMeshManager::Create_SwapChain()
{
	m_SwapChain.Reset();

	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferDesc.Width = m_ClientWidth;
	sd.BufferDesc.Height = m_ClientHeight;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = m_BackBufferFormat;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.SampleDesc.Count = m_4xMsaaState ? 4 : 1;
	sd.SampleDesc.Quality = m_4xMsaaState ? (m_4xMsaaQuality - 1) : 0;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = m_SwapChainBufferCount;
	sd.OutputWindow = m_hWnd;
	sd.Windowed = true;
	sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	ThrowIfFailed(m_dxgiFactory->CreateSwapChain(
		m_CommandQueue.Get(),
		&sd,
		m_SwapChain.GetAddressOf()));
}

void CMeshManager::Create_RtvAndDsv_DescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
	rtvHeapDesc.NumDescriptors = m_SwapChainBufferCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 0;
	ThrowIfFailed(m_d3dDevice->CreateDescriptorHeap(
		&rtvHeapDesc, IID_PPV_ARGS(m_RtvHeap.GetAddressOf())));

	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NodeMask = 0;
	ThrowIfFailed(m_d3dDevice->CreateDescriptorHeap(
		&dsvHeapDesc, IID_PPV_ARGS(m_DsvHeap.GetAddressOf())));
}

void CMeshManager::Resize_SwapChainBuffers()
{
	assert(m_d3dDevice);
	assert(m_SwapChain);
	assert(m_DirectCmdListAlloc);

	FlushCommandQueue();

	ThrowIfFailed(m_CommandList->Reset(m_DirectCmdListAlloc.Get(), nullptr));

	for (int i = 0; i < m_SwapChainBufferCount; ++i)
		m_SwapChainBuffer[i].Reset();

	m_DepthStencilBuffer.Reset();

	ThrowIfFailed(m_SwapChain->ResizeBuffers(
		m_SwapChainBufferCount,
		m_ClientWidth, m_ClientHeight,
		m_BackBufferFormat,
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));
}

void CMeshManager::FlushCommandQueue()
{
	m_CurrentFence++;

	ThrowIfFailed(m_CommandQueue->Signal(m_Fence.Get(), m_CurrentFence));

	if (m_Fence->GetCompletedValue() < m_CurrentFence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);

		ThrowIfFailed(m_Fence->SetEventOnCompletion(m_CurrentFence, eventHandle));

		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
}

void CMeshManager::Create_RenderTarget()
{
	m_CurrBackBuffer = 0;

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(m_RtvHeap->GetCPUDescriptorHandleForHeapStart());

	for (UINT i = 0; i < m_SwapChainBufferCount; i++)
	{
		ThrowIfFailed(m_SwapChain->GetBuffer(i, IID_PPV_ARGS(&m_SwapChainBuffer[i])));
		m_d3dDevice->CreateRenderTargetView(m_SwapChainBuffer[i].Get(), nullptr, rtvHeapHandle);
		rtvHeapHandle.Offset(1, m_RtvDescriptorSize);
	}

}

void CMeshManager::Create_DepthStencil_Buff_And_View()
{
	D3D12_RESOURCE_DESC depthStencilDesc;
	depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilDesc.Alignment = 0;
	depthStencilDesc.Width = m_ClientWidth;
	depthStencilDesc.Height = m_ClientHeight;
	depthStencilDesc.DepthOrArraySize = 1;
	depthStencilDesc.MipLevels = 1;

	depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;

	depthStencilDesc.SampleDesc.Count = m_4xMsaaState ? 4 : 1;
	depthStencilDesc.SampleDesc.Quality = m_4xMsaaState ? (m_4xMsaaQuality - 1) : 0;
	depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE optClear;
	optClear.Format = m_DepthStencilFormat;
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;
	ThrowIfFailed(m_d3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&depthStencilDesc,
		D3D12_RESOURCE_STATE_COMMON,
		&optClear,
		IID_PPV_ARGS(m_DepthStencilBuffer.GetAddressOf())));

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Format = m_DepthStencilFormat;
	dsvDesc.Texture2D.MipSlice = 0;
	m_d3dDevice->CreateDepthStencilView(m_DepthStencilBuffer.Get(), &dsvDesc, DepthStencilView());

	m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_DepthStencilBuffer.Get(),
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE));
}

D3D12_CPU_DESCRIPTOR_HANDLE CMeshManager::DepthStencilView()
{
	return m_DsvHeap->GetCPUDescriptorHandleForHeapStart();
}

void CMeshManager::Execute_Init_Commands()
{
	ThrowIfFailed(m_CommandList->Close());
	ID3D12CommandList* cmdsLists[] = { m_CommandList.Get() };
	m_CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	FlushCommandQueue();
}

void CMeshManager::Update_ViewPort_And_Scissor()
{
	m_ScreenViewport.TopLeftX = 0;
	m_ScreenViewport.TopLeftY = 0;
	m_ScreenViewport.Width = static_cast<float>(m_ClientWidth);
	m_ScreenViewport.Height = static_cast<float>(m_ClientHeight);
	m_ScreenViewport.MinDepth = 0.0f;
	m_ScreenViewport.MaxDepth = 1.0f;

	m_ScissorRect = { 0, 0, m_ClientWidth, m_ClientHeight };
}



D3D12_CPU_DESCRIPTOR_HANDLE CMeshManager::CurrentBackBufferView()
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(
		m_RtvHeap->GetCPUDescriptorHandleForHeapStart(),
		m_CurrBackBuffer,
		m_RtvDescriptorSize);
}

ID3D12Resource* CMeshManager::CurrentBackBuffer()
{
	return m_SwapChainBuffer[m_CurrBackBuffer].Get();
}


void CMeshManager::Init_MeshManager(HWND hWnd)
{
	m_hWnd = hWnd;

	EnableDebugLayer_CreateFactory();

	Create_Device();

	CreateFence_GetDescriptorsSize();

	Check_Multisample_Quality();

	Create_CommandList_Allocator_Queue();

	Create_SwapChain();

	Create_RtvAndDsv_DescriptorHeaps();

	Resize_SwapChainBuffers();

	Create_RenderTarget();

	Create_DepthStencil_Buff_And_View();

	Execute_Init_Commands();

	Update_ViewPort_And_Scissor();

	m_BackBuffer = new unsigned char[800 * 600 * 4];

	const UINT64 UploadBufferSize = GetRequiredIntermediateSize(CurrentBackBuffer(), 0, 1);

	ThrowIfFailed(m_d3dDevice.Get()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(UploadBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&UploadHeap)));
		
	m_Timer.TimerStart(30);

	m_Res = NULL;

	m_VertCount = 24;
	m_TriangleCount = 12;

	Read_BMP_File(".//texture.bmp");

	vector3 VertBuffTemp[24] = {
		vector3(-5.000000,-5.000000,-5.000000),
		vector3(-5.000000,-5.000000,5.000000),
		vector3(5.000000,-5.000000,5.000000),
		vector3(5.000000,-5.000000,-5.000000),
		vector3(-5.000000,5.000000,-5.000000),
		vector3(5.000000,5.000000,-5.000000),
		vector3(5.000000,5.000000,5.000000),
		vector3(-5.000000,5.000000,5.000000),
		vector3(-5.000000,-5.000000,-5.000000),
		vector3(5.000000,-5.000000,-5.000000),
		vector3(5.000000,5.000000,-5.000000),
		vector3(-5.000000,5.000000,-5.000000),
		vector3(5.000000,-5.000000,-5.000000),
		vector3(5.000000,-5.000000,5.000000),
		vector3(5.000000,5.000000,5.000000),
		vector3(5.000000,5.000000,-5.000000),
		vector3(5.000000,-5.000000,5.000000),
		vector3(-5.000000,-5.000000,5.000000),
		vector3(-5.000000,5.000000,5.000000),
		vector3(5.000000,5.000000,5.000000),
		vector3(-5.000000,-5.000000,5.000000),
		vector3(-5.000000,-5.000000,-5.000000),
		vector3(-5.000000,5.000000,-5.000000),
		vector3(-5.000000,5.000000,5.000000) };

	tex_coord2 TexCoordTemp[24] = {
		m_TextureWidth - 1,	m_TextureHeight - 1,
		m_TextureWidth - 1,	0,
		0,					0,
		0,					m_TextureHeight - 1,
		0,					m_TextureHeight - 1,
		m_TextureWidth - 1,	m_TextureHeight - 1,
		m_TextureWidth - 1,	0,
		0,					0,
		0,					m_TextureHeight - 1,
		m_TextureWidth - 1,	m_TextureHeight - 1,
		m_TextureWidth - 1,	0,
		0,					0,
		0,					m_TextureHeight - 1,
		m_TextureWidth - 1,	m_TextureHeight - 1,
		m_TextureWidth - 1,	0,
		0,					0,
		0,					m_TextureHeight - 1,
		m_TextureWidth - 1,	m_TextureHeight - 1,
		m_TextureWidth - 1,	0,
		0,					0,
		0,					m_TextureHeight - 1,
		m_TextureWidth - 1,	m_TextureHeight - 1,
		m_TextureWidth - 1,	0,
		0,					0 };

	DWORD IndexBuffTemp[36] = {
		0,2,1,
		2,0,3,
		4,6,5,
		6,4,7,
		8,10,9,
		10,8,11,
		12,14,13,
		14,12,15,
		16,18,17,
		18,16,19,
		20,22,21,
		22,20,23 };

	m_VertBuff = NULL;
	m_VertBuff = new vector3[24];

	m_VertBuffTransformed = NULL;
	m_VertBuffTransformed = new vector3[24];

	m_TexCoord = NULL;
	m_TexCoord = new tex_coord2[24];

	m_IndexBuff = NULL;
	m_IndexBuff = new DWORD[36];

	memcpy(m_VertBuff, VertBuffTemp, 24 * sizeof(vector3));
	memcpy(m_TexCoord, TexCoordTemp, 24 * sizeof(tex_coord2));
	memcpy(m_IndexBuff, IndexBuffTemp, 36 * sizeof(DWORD));
}

void CMeshManager::Update_MeshManager()
{
	m_Timer.CalculateFPS();
	float ElapsedTime = m_Timer.GetElaspedTime();

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

	matrix4x4 MatRotateZ = {
		cosf(Angle), sinf(Angle), 0, 0,
		-sinf(Angle), cosf(Angle), 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1 };

	Angle += ElapsedTime;
	if (Angle > PI * 2.0f)
		Angle = 0.0f;

	matrix4x4 MatWorld = {
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 18.0f, 1 };

	matrix4x4 MatView = {
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1 };

	float Fov = PI / 2.0f; // FOV 90 degree
	float Aspect = (float)m_ClientWidth / m_ClientHeight;
	float ZFar = 100.0f;
	float ZNear = 0.1f;

	float h, w, Q;

	w = (1.0f / tanf(Fov * 0.5f)) / Aspect;
	h = 1.0f / tanf(Fov * 0.5f);
	Q = ZFar / (ZFar - ZNear);

	/*
	//полный расчет матрицы проекции нам не нужен
	matrix4x4 MatProj={
		w, 0, 0, 0,
		0, h, 0, 0,
		0, 0, Q, 1,
		0, 0, -Q*fZNear, 0 };
	*/

	matrix4x4 MatProj = {
		w, 0, 0, 0,
		0, h, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1 };

	float Alpha = 0.5f * m_ClientWidth;
	float Beta = 0.5f * m_ClientHeight;

	matrix4x4 MatScreen = {
		Alpha,  0,	    0,    0,
		0,      -Beta,  0,    0,
		0,		0,		1,    0,
		Alpha,  Beta,	0,    1 };

	for (UINT i = 0; i < m_VertCount; i++)
	{
		vector3 Vec;

		Vec = Vec3_Mat4x4_Mul(m_VertBuff[i], MatRotateX);
		Vec = Vec3_Mat4x4_Mul(Vec, MatRotateY);
		Vec = Vec3_Mat4x4_Mul(Vec, MatRotateZ);
		Vec = Vec3_Mat4x4_Mul(Vec, MatWorld);
		Vec = Vec3_Mat4x4_Mul(Vec, MatView);
		Vec = Vec3_Mat4x4_Mul(Vec, MatProj);

		Vec.x = Vec.x / Vec.z;
		Vec.y = Vec.y / Vec.z;

		Vec = Vec3_Mat4x4_Mul(Vec, MatScreen);

		m_VertBuffTransformed[i] = Vec;
	}
}

void CMeshManager::Draw_MeshManager()
{
	ThrowIfFailed(m_DirectCmdListAlloc->Reset());

	ThrowIfFailed(m_CommandList->Reset(m_DirectCmdListAlloc.Get(), nullptr));

	m_CommandList->RSSetViewports(1, &m_ScreenViewport);
	m_CommandList->RSSetScissorRects(1, &m_ScissorRect);

	m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };
	m_CommandList->ClearRenderTargetView(CurrentBackBufferView(), ClearColor, 0, nullptr);
	m_CommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	m_CommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());

	Draw_MeshManager2();

	m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	ThrowIfFailed(m_CommandList->Close());

	ID3D12CommandList* cmdsLists[] = { m_CommandList.Get() };
	m_CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	ThrowIfFailed(m_SwapChain->Present(0, 0));

	m_CurrBackBuffer = (m_CurrBackBuffer + 1) % m_SwapChainBufferCount;

	FlushCommandQueue();
}

void CMeshManager::Read_BMP_File(const char* szTexFileName)
{
	FILE* Fp;

	fopen_s(&Fp, szTexFileName, "rb");
	if (Fp == NULL) printf("Error Open File");

	BITMAPFILEHEADER Bfh;
	fread(&Bfh, sizeof(Bfh), 1, Fp);

	BITMAPINFOHEADER Bih;
	fread(&Bih, sizeof(Bih), 1, Fp);

	fseek(Fp, Bfh.bfOffBits, SEEK_SET);

	m_Res = new unsigned char[Bih.biWidth * Bih.biHeight * 3];
	fread(m_Res, Bih.biWidth * Bih.biHeight * 3, 1, Fp);

	fclose(Fp);

	m_TextureWidth = Bih.biWidth;
	m_TextureHeight = Bih.biHeight;
}

vector3 CMeshManager::Vec3_Mat4x4_Mul(vector3 VecIn, matrix4x4 MatIn)
{
	vector3 VecOut;

	for (int j = 0; j < 3; j++)
	{
		float Sum = 0.0f;
		int i;
		for (i = 0; i < 3; i++)
		{
			Sum += VecIn.Vec[i] * MatIn.Mat[i][j];
		}

		Sum += MatIn.Mat[i][j];
		VecOut.Vec[j] = Sum;
	}

	return VecOut;
}

void CMeshManager::Draw_Textured_Triangle(vector3 Vec1, tex_coord2 TexCoord1,
	vector3 Vec2, tex_coord2 TexCoord2,
	vector3 Vec3, tex_coord2 TexCoord3)
{
	int Side;
	float x1, x2, x3;
	float y1, y2, y3;
	float iz1, uiz1, viz1, iz2, uiz2, viz2, iz3, uiz3, viz3;
	float Tempf;

	x1 = Vec1.x;
	y1 = Vec1.y;
	x2 = Vec2.x;
	y2 = Vec2.y;
	x3 = Vec3.x;
	y3 = Vec3.y;

	iz1 = 1.0f / Vec1.z;
	iz2 = 1.0f / Vec2.z;
	iz3 = 1.0f / Vec3.z;

	uiz1 = TexCoord1.tu * iz1;
	viz1 = TexCoord1.tv * iz1;
	uiz2 = TexCoord2.tu * iz2;
	viz2 = TexCoord2.tv * iz2;
	uiz3 = TexCoord3.tu * iz3;
	viz3 = TexCoord3.tv * iz3;

#define swapfloat(x, y) Tempf = x; x = y; y = Tempf;

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

	if (!Side)
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

	for (int yi = y1; yi < y2; yi++)
	{
		ui = m_ul;
		vi = m_vl;
		zi = m_zl;

		if ((m_xr - m_xl) > 0)
		{
			du = (m_ur - m_ul) / (m_xr - m_xl);
			dv = (m_vr - m_vl) / (m_xr - m_xl);
			dz = (m_zr - m_zl) / (m_xr - m_xl);
		}
		else
		{
			du = 0;
			dv = 0;
			dz = 0;
		}

		for (int xi = (int)m_xl; xi < (int)m_xr; xi++)
		{
			float z = 1.0f / zi;
			float u = ui * z;
			float v = vi * z;

			int t = (int)u + (((int)v) * m_TextureWidth);

			if (t < 0 || t >(m_TextureWidth * m_TextureHeight - 1))
				continue;

			t = t * 3;

			int Index = yi * 4 * m_ClientWidth + xi * 4;

			m_BackBuffer[Index + 0] = (BYTE)m_Res[t + 2]; // red
			m_BackBuffer[Index + 1] = (BYTE)m_Res[t + 1]; // green
			m_BackBuffer[Index + 2] = (BYTE)m_Res[t + 0]; // blue
			m_BackBuffer[Index + 3] = 0;

			ui += du;
			vi += dv;
			zi += dz;
		}

		m_xl += m_dxdyl;
		m_ul += m_dudyl;
		m_vl += m_dvdyl;
		m_zl += m_dzdyl;

		m_xr += m_dxdyr;
		m_ur += m_dudyr;
		m_vr += m_dvdyr;
		m_zr += m_dzdyr;
	}
}

void CMeshManager::Draw_MeshManager2()
{
	//очищаем pBuffer (экран)
	for (int x = 0; x < 800; x++)
	{
		for (int y = 0; y < 600; y++)
		{
			int Index = y * 800 * 4 + x * 4;

			m_BackBuffer[Index] = 0; //red
			m_BackBuffer[Index + 1] = 32; //green
			m_BackBuffer[Index + 2] = 77; //blue
			m_BackBuffer[Index + 3] = 0; //Alpha
		}
	}

	for (UINT i = 0; i < m_TriangleCount; i++)
	{
		vector3 Vec1 = m_VertBuffTransformed[m_IndexBuff[i * 3 + 0]];
		vector3 Vec2 = m_VertBuffTransformed[m_IndexBuff[i * 3 + 1]];
		vector3 Vec3 = m_VertBuffTransformed[m_IndexBuff[i * 3 + 2]];

		tex_coord2 TexCoord1 = m_TexCoord[m_IndexBuff[i * 3 + 0]];
		tex_coord2 TexCoord2 = m_TexCoord[m_IndexBuff[i * 3 + 1]];
		tex_coord2 TexCoord3 = m_TexCoord[m_IndexBuff[i * 3 + 2]];

		//используем псевдоскал€рное (косое) произведение векторов
		//дл€ отбрасывани€ задних поверхностей
		//векторы в экранных координатах
		float s = (Vec2.x - Vec1.x) * (Vec3.y - Vec1.y) - (Vec2.y - Vec1.y) * (Vec3.x - Vec1.x);

		if (s <= 0)
			continue;

		//рисуем текущий треугольник
		Draw_Textured_Triangle(Vec1, TexCoord1, Vec2, TexCoord2, Vec3, TexCoord3);
	}

	//копируем m_BackBuffer в backbuffer DX12
	m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_DEST));

	D3D12_SUBRESOURCE_DATA textureData = {};
	textureData.pData = m_BackBuffer;
	textureData.RowPitch = 800 * 4;
	textureData.SlicePitch = textureData.RowPitch * 600;

	UpdateSubresources(m_CommandList.Get(), CurrentBackBuffer(), UploadHeap.Get(), 0, 0, 1, &textureData);

	m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET));
}