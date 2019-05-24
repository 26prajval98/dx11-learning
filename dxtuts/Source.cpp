// include the basic windows header file
#include <windows.h>
#include <windowsx.h>
#include <string>
// the WindowProc function prototype
LRESULT CALLBACK WW(HWND hWnd,
	UINT message,
	WPARAM wParam,
	LPARAM lParam);

class T
{
public:
	T();
	~T();
	static void inc();
	static int x;

private:
};

T::T()
{
	x = 0;
}

T::~T()
{
	x = 0;
}

int T::x = 0;

void T::inc()
{
	T::x = T::x + 1;
	char buffer[10];
	itoa(T::x, buffer, 10);
	std::string str(buffer);
	//MessageBox(NULL,(LPCWSTR)buffer, (LPCWSTR)buffer, S_OK);
}

// the entry point for any Windows program
int WINAPI WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
{
	// the handle for the window, filled by a function
	HWND hWnd;
	// this struct holds information for the window class
	WNDCLASSEX wc;

	// clear out the window class for use
	ZeroMemory(&wc, sizeof(WNDCLASSEX));

	// fill in the struct with the needed information
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WW;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.lpszClassName = L"WindowClass1";

	// register the window class
	RegisterClassEx(&wc);

	RECT wr = { 0, 0, 1920, 1080 };    // set the size, but not the position
	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);    // adjust the size

	// create the window and use the result as the handle
	hWnd = CreateWindowEx(NULL,
		L"WindowClass1",    // name of the window class
		L"Our First Windowed Program",   // title of the window
		WS_OVERLAPPEDWINDOW,    // window style
		300,    // x-position of the window
		300,    // y-position of the window
		wr.right - wr.left,    // width of the window
		wr.bottom - wr.top,    // height of the window
		NULL,    // we have no parent window, NULL
		NULL,    // we aren't using menus, NULL
		hInstance,    // application handle
		NULL);    // used with multiple windows, NULL

// display the window on the screen
	ShowWindow(hWnd, nCmdShow);

	// enter the main loop:

	// this struct holds Windows event messages
	MSG msg;

	// wait for the next message in the queue, store the result in 'msg'
	while (GetMessage(&msg, NULL, 0, 0))
	{
		// translate keystroke messages into the right format
		TranslateMessage(&msg);

		// send the message to the WindowProc function
		DispatchMessage(&msg);
	}

	// return this part of the WM_QUIT message to Windows
	return msg.wParam;
}

// this is the main message handler for the program
LRESULT CALLBACK WW(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	T::inc();
	// sort through and find what code to run for the message given
	switch (message)
	{

		// this message is read when the window is closed
	case WM_DESTROY:
	{
		// close the application entirely
		MessageBox(NULL, L"WORKING", L"WORKING", S_OK);
		PostQuitMessage(0);
		return 0;
	} break;

	}
	// Handle any messages the switch statement didn't
	return DefWindowProc(hWnd, message, wParam, lParam);
}