#include <windows.h>
#include <windowsx.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dx10.h>
#include <xnamath.h>
#include <math.h>
#include <vector>
#include <stdio.h>

// include the Direct3D Library file
#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "d3dx11.lib")
#pragma comment (lib, "d3dx10.lib")


#define SCREEN_WIDTH  1680
#define SCREEN_HEIGHT 1050
#define VS_FILE L"Shader.fx"
#define PS_FILE L"Shader.fx"
#define POLYGON 20
#define M_PI 3.14159265358979323846

struct Vertex    //Overloaded Vertex Structure
{
	Vertex() {}
	Vertex(float x, float y, float z,
		float cr, float cg, float cb, float ca)
		: pos(x, y, z), color(cr, cg, cb, ca) {}

	XMFLOAT3 pos;
	XMFLOAT4 color;
};

D3D11_INPUT_ELEMENT_DESC layout[] =
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
};

UINT numElements = ARRAYSIZE(layout);

// global declarations
IDXGISwapChain *swapchain;				// the pointer to the swap chain interface
ID3D11Device *dev;						// the pointer to our Direct3D device interface
ID3D11DeviceContext *devcon;			// the pointer to our Direct3D device context
ID3D11RenderTargetView *backbuffer;		// global declaration
ID3D11Buffer *vertexBuffer;				// Vertex Buffer
ID3D11Buffer *indexBuffer;				// Index Buffer
ID3D11VertexShader* VS;					// Vertex Shader
ID3D11PixelShader* PS;					// Pixel Shader
ID3D10Blob* VS_Buffer;					// VS buffer
ID3D10Blob* PS_Buffer;					// PS buffer
ID3D11InputLayout* vertLayout;			// Input Layout
ID3D11DepthStencilView* depthStencilView;	// Depth Views
ID3D11Texture2D* depthStencilBuffer;		// Depth Buffers

// function prototypes
void InitD3D(HWND hWnd);    // sets up and initializes Direct3D
void CleanD3D(void);        // closes Direct3D and releases memory
void RenderFrame(void);
bool InitScene();
void GenerateVertices(std::vector <Vertex> &, float);
void GenerateIndices(std::vector <int> &);
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);		// the WindowProc function prototype

static double d2r(double d) {
	return (d / 180.0) * ((double)M_PI);
}

// the entry point for any Windows program
int WINAPI WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
{
	HWND hWnd;
	WNDCLASSEX wc;

	ZeroMemory(&wc, sizeof(WNDCLASSEX));

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	//wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.lpszClassName = L"WindowClass";

	RegisterClassEx(&wc);

	RECT wr = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

	hWnd = CreateWindowEx(NULL,
		L"WindowClass",
		L"Our First Direct3D Program",
		WS_OVERLAPPEDWINDOW,
		0,
		0,
		wr.right - wr.left,
		wr.bottom - wr.top,
		NULL,
		NULL,
		hInstance,
		NULL);

	ShowWindow(hWnd, nCmdShow);

	// set up and initialize Direct3D
	InitD3D(hWnd);

	// enter the main loop:

	MSG msg;

	while (TRUE)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if (msg.message == WM_QUIT)
				break;
		}
		else
		{
			// Run game code here
			// ...
			// ...
			RenderFrame();
		}
	}

	// clean up DirectX and COM
	CleanD3D();

	return msg.wParam;
}




/***************************************************************************************************************************************************/




// this is the main message handler for the program
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		return 0;
	} break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

// this function initializes and prepares Direct3D for use
void InitD3D(HWND hWnd)
{
	// create a struct to hold information about the swap chain
	DXGI_SWAP_CHAIN_DESC scd;

	// clear out the struct for use
	ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));

	// fill the swap chain description struct
	scd.BufferCount = 1;                                    // one back buffer
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;     // use 32-bit color
	scd.BufferDesc.Width = SCREEN_WIDTH;                    // set the back buffer width
	scd.BufferDesc.Height = SCREEN_HEIGHT;                  // set the back buffer height
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;      // how swap chain is to be used
	scd.OutputWindow = hWnd;                                // the window to be used
	scd.SampleDesc.Count = 4;                               // how many multisamples
	scd.Windowed = TRUE;                                    // windowed/full-screen mode
	scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;     // allow full-screen switching

	// create a device, device context and swap chain using the information in the scd struct
	D3D11CreateDeviceAndSwapChain(NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		D3D11_CREATE_DEVICE_BGRA_SUPPORT,
		NULL,
		NULL,
		D3D11_SDK_VERSION,
		&scd,
		&swapchain,
		&dev,
		NULL,
		&devcon);


	/*
		Set render targets
	*/

	// get the address of the back buffer
	ID3D11Texture2D *pBackBuffer;
	swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

	// use the back buffer address to create the render target
	dev->CreateRenderTargetView(pBackBuffer, NULL, &backbuffer);
	pBackBuffer->Release();

	// set the render target as the back buffer
	devcon->OMSetRenderTargets(1, &backbuffer, NULL);


	/*
		Set Viewport
	*/

	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = SCREEN_WIDTH;
	viewport.Height = SCREEN_HEIGHT;

	devcon->RSSetViewports(1, &viewport);

	InitScene();
}

bool InitScene()
{
	//Compile Shaders from shader file
	D3DX11CompileFromFile(VS_FILE, 0, 0, "VS", "vs_5_0", 0, 0, 0, &VS_Buffer, 0, 0);
	D3DX11CompileFromFile(PS_FILE, 0, 0, "PS", "ps_5_0", 0, 0, 0, &PS_Buffer, 0, 0);

	//Create the Shader Objects
	dev->CreateVertexShader(VS_Buffer->GetBufferPointer(), VS_Buffer->GetBufferSize(), NULL, &VS);
	dev->CreatePixelShader(PS_Buffer->GetBufferPointer(), PS_Buffer->GetBufferSize(), NULL, &PS);

	//Set Vertex and Pixel Shaders
	devcon->VSSetShader(VS, 0, 0);
	devcon->PSSetShader(PS, 0, 0);

	float factor = SCREEN_HEIGHT * 1.0 / SCREEN_WIDTH;

	//Create the vertex buffer
	std::vector<Vertex> v =
	{
		Vertex(0.0f * factor, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f),
	};

	GenerateVertices(v, factor);

	std::vector<int> indices;

	GenerateIndices(indices);

	//This is for setting indices
	D3D11_BUFFER_DESC indexBufferDesc;
	ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));

	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(Vertex) * 3 * (POLYGON + 1);
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA indexBufferData;

	ZeroMemory(&indexBufferData, sizeof(indexBufferData));
	indexBufferData.pSysMem = &indices[0];
	dev->CreateBuffer(&indexBufferDesc, &indexBufferData, &indexBuffer);

	devcon->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	//This is for setting vertices

	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));

	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(Vertex) * (POLYGON + 1);
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA vertexBufferData;

	ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
	vertexBufferData.pSysMem = (void *)&v[0];
	dev->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &vertexBuffer);

	//Set the vertex buffer
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	devcon->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);

	//Create the Input Layout
	dev->CreateInputLayout(layout, numElements, VS_Buffer->GetBufferPointer(),
		VS_Buffer->GetBufferSize(), &vertLayout);

	//Set the Input Layout
	devcon->IASetInputLayout(vertLayout);

	//Set Primitive Topology
	devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//Describe our Depth/Stencil Buffer
	D3D11_TEXTURE2D_DESC depthStencilDesc;
	depthStencilDesc.Width = SCREEN_WIDTH;
	depthStencilDesc.Height = SCREEN_HEIGHT;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	//Create depth stencil view
	dev->CreateTexture2D(&depthStencilDesc, NULL, &depthStencilBuffer);
	dev->CreateDepthStencilView(depthStencilBuffer, NULL, &depthStencilView);

	//Bind to pipeline
	devcon->OMSetRenderTargets(1, &backbuffer, depthStencilView);
	
	//Create the Viewport
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
	
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = SCREEN_WIDTH;
	viewport.Height = SCREEN_HEIGHT;

	//Set the Viewport
	devcon->RSSetViewports(1, &viewport);

	return true;
}

// this is the function used to render a single frame
void RenderFrame(void)
{
	// clear the back buffer to a deep blue
	devcon->ClearRenderTargetView(backbuffer, D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f));
	
	//Clear depth/stencil view
	devcon->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// do 3D rendering on the back buffer here
	devcon->DrawIndexed(3 * POLYGON, 0, 0);

	// switch the back buffer and the front buffer
	swapchain->Present(0, 0);
}

// this is the function that cleans up Direct3D and COM
void CleanD3D(void)
{
	// close and release all existing COM objects
	swapchain->SetFullscreenState(FALSE, NULL);    // switch to windowed
	swapchain->Release();
	dev->Release();
	devcon->Release();
	backbuffer->Release();
	vertexBuffer->Release();
	VS->Release();
	PS->Release();
	VS_Buffer->Release();
	PS_Buffer->Release();
	vertLayout->Release();
	depthStencilView->Release();
	depthStencilBuffer->Release();
}


void GenerateVertices(std::vector <Vertex> &vec, float factor)
{
	/*float yFac = 0.5f;
	float xFac = yFac * factor;
	float degree = 0;
	float inc = 360.0 / POLYGON;
	
	while (degree <= 359.9999999)
	{
		float rad = d2r(degree);
		vec.push_back(Vertex(xFac * sin(rad), yFac * cos(rad), 0.0f, 1.0f, 0.0f, 0.0f, 1.0f));
		degree += inc;
	}*/

	vec = {
		Vertex(-1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f),
		Vertex(-1.0f, +1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f),
		Vertex(+1.0f, +1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f),
		Vertex(+1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f),
		Vertex(-1.0f, -1.0f, +1.0f, 0.0f, 1.0f, 1.0f, 1.0f),
		Vertex(-1.0f, +1.0f, +1.0f, 1.0f, 1.0f, 1.0f, 1.0f),
		Vertex(+1.0f, +1.0f, +1.0f, 1.0f, 0.0f, 1.0f, 1.0f),
		Vertex(+1.0f, -1.0f, +1.0f, 1.0f, 0.0f, 0.0f, 1.0f),
	};
}


void GenerateIndices(std::vector <int> &indices)
{	
	/*int second = 1;
	while (second < POLYGON)
	{
		indices.push_back(0);
		indices.push_back(second);
		indices.push_back(second + 1);
		second++;
	}
	indices.push_back(0);
	indices.push_back(POLYGON);
	indices.push_back(1);*/

	indices = {
		// front face
		0, 1, 2,
		0, 2, 3,

		// back face
		4, 6, 5,
		4, 7, 6,

		// left face
		4, 5, 1,
		4, 1, 0,

		// right face
		3, 2, 6,
		3, 6, 7,

		// top face
		1, 5, 6,
		1, 6, 2,

		// bottom face
		4, 0, 3,
		4, 3, 7
	};
}
