#pragma once

extern bool globalRunning;
extern bool globalPause;

#undef max
#undef min

void *buffer_memory;
int buffer_width;
int buffer_height;
BITMAPINFO buffer_bitmapinfo;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
		case WM_SIZE: {
			RECT rect;
			GetClientRect(hwnd, &rect);
			buffer_width = rect.right - rect.left;
			buffer_height = rect.bottom - rect.top;

			unsigned int buffer_size = buffer_width * buffer_height * sizeof(unsigned int);

			if (buffer_memory) VirtualFree(buffer_memory, 0, MEM_RELEASE);

			buffer_memory = VirtualAlloc(0, buffer_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

			buffer_bitmapinfo.bmiHeader.biSize = sizeof(buffer_bitmapinfo.bmiHeader);
			buffer_bitmapinfo.bmiHeader.biWidth = buffer_width;
			buffer_bitmapinfo.bmiHeader.biHeight = buffer_height;
			buffer_bitmapinfo.bmiHeader.biPlanes = 1;
			buffer_bitmapinfo.bmiHeader.biBitCount = 24;
			buffer_bitmapinfo.bmiHeader.biCompression = BI_RGB;
		} break;

		default: {
			return DefWindowProc(hwnd, uMsg, wParam, lParam);
		}
	}
	return 0;
}

HWND Win32CreateWindow(int width, int height, const char *name) {
	WNDCLASS wc = { };

	wc.lpfnWndProc = WindowProc;
	wc.lpszClassName = "Sample Window Class";

	RegisterClass(&wc);
	HWND hwnd = CreateWindowEx(0, wc.lpszClassName, name, WS_OVERLAPPEDWINDOW | WS_VISIBLE,
							   CW_USEDEFAULT, CW_USEDEFAULT, width + 16, height + 39, NULL, NULL, NULL, NULL);

	if (hwnd == NULL) {
		return 0;
	}

	return hwnd;
}

void Win32DrawToWindow(HWND &window, void *image, int width, int height) {
	HDC hdc = GetDC(window);
	StretchDIBits(hdc, 0, 0, width, height, 0, 0, width, height, image, &buffer_bitmapinfo, DIB_RGB_COLORS, SRCCOPY);
}

#define S_BUTTON 0x53
#define C_BUTTON 0x43
#define R_BUTTON 0x52
#define B_BUTTON 0x42
#define D_BUTTON 0x44
#define A_BUTTON 0x41
#define L_BUTTON 0x4C
#define J_BUTTON 0x4A
#define I_BUTTON 0x49
#define K_BUTTON 0x4B
#define X_BUTTON 0x58
#define Z_BUTTON 0x5A
#define P_BUTTON 0x50
#define W_BUTTON 0x57
#define Q_BUTTON 0x51
#define E_BUTTON 0x45
// Alpha - around X axis, Beta - around Y axis, Gamma - around Z axis
void ProcessInput(HWND window, float &angleAlpha, float &angleBeta, float &angleGamma, float &cameraAngleAlpha, float &cameraAngleBeta, float &scaleVariable, float deltaTime) {
	MSG msg;

	if (PeekMessage(&msg, window, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	switch (msg.message) {
		// Mouse
		// NOTE: Turned out that editor's features require more knowledge than I expected.
		// So probably I put it away for a while.
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP: {
			OutputDebugStringA("Mouse left button was clicked\n");
		} break;
		// Keyboard
		case WM_KEYUP:
		case WM_KEYDOWN: {
			UINT32 vkCode = msg.wParam;
			bool isKeyDown = ((msg.lParam >> 31) == 0);
			if (isKeyDown) {
				if (vkCode == D_BUTTON) {
					angleBeta += M_PI / 8 * deltaTime;
					if (angleBeta > 2 * M_PI) {
						angleBeta = 0;
					}
				} else if (vkCode == A_BUTTON) {
					angleBeta -= M_PI / 8 * deltaTime;
					if (angleBeta < 0) {
						angleBeta = 2 * M_PI;
					}
				} else if (vkCode == E_BUTTON) {
					angleGamma -= M_PI / 8 * deltaTime;
					if (angleGamma < 0) {
						angleGamma = 2 * M_PI;
					}
				} else if (vkCode == Q_BUTTON) {
					angleGamma += M_PI / 8 * deltaTime;
					if (angleGamma > 2 * M_PI) {
						angleGamma = 0;
					}
				} else if (vkCode == W_BUTTON) {
					angleAlpha += M_PI / 8 * deltaTime;
					if (angleAlpha > 2 * M_PI) {
						angleAlpha = 0;
					}
				} else if (vkCode == S_BUTTON) {
					angleAlpha -= M_PI / 8 * deltaTime;
					if (angleAlpha < 0) {
						angleAlpha = 2 * M_PI;
					}
				} else if (vkCode == J_BUTTON) {
					cameraAngleBeta -= M_PI / 8 * deltaTime;
					if (cameraAngleBeta < 0) {
						cameraAngleBeta = 2 * M_PI;
					}
				} else if (vkCode == L_BUTTON) {
					cameraAngleBeta += M_PI / 8 * deltaTime;
					if (cameraAngleBeta > 2 * M_PI) {
						cameraAngleBeta = 0;
					}
				} else if (vkCode == I_BUTTON) {
					cameraAngleAlpha -= M_PI / 8 * deltaTime;
					if (cameraAngleBeta < 0) {
						cameraAngleBeta = 2 * M_PI;
					}
				} else if (vkCode == K_BUTTON) {
					cameraAngleAlpha += M_PI / 8 * deltaTime;
					if (cameraAngleBeta > 2 * M_PI) {
						cameraAngleBeta = 0;
					}
				} else if (vkCode == VK_ESCAPE) {
					globalRunning = !globalRunning;
				} else if (vkCode == X_BUTTON) {
					scaleVariable *= 1.2f;
				} else if (vkCode == Z_BUTTON) {
					scaleVariable /= 1.2f;
				} else if (vkCode == P_BUTTON) {
					globalPause = !globalPause;
				}
			}
		} break;

	}
	return;
}
