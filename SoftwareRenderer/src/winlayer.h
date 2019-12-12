#pragma once

enum KeyBindings {
	S_BUTTON = 0x53,
	C_BUTTON = 0x43,
	R_BUTTON = 0x52,
	B_BUTTON = 0x42,
	D_BUTTON = 0x44,
	A_BUTTON = 0x41,
	L_BUTTON = 0x4C,
	J_BUTTON = 0x4A,
	I_BUTTON = 0x49,
	K_BUTTON = 0x4B,
	X_BUTTON = 0x58,
	Z_BUTTON = 0x5A,
	P_BUTTON = 0x50,
	W_BUTTON = 0x57,
	Q_BUTTON = 0x51,
	E_BUTTON = 0x45,
	// Player
	T_BUTTON = 0x54,
	F_BUTTON = 0x46,
	G_BUTTON = 0x47,
	H_BUTTON = 0x48,
};

extern bool globalRunning;
extern bool globalPause;

#undef max
#undef min

void *buffer_memory;
int buffer_width;
int buffer_height;
BITMAPINFO buffer_bitmapinfo;

struct Mouse {
	int x;
	int y;
	float wheelFOV;
};

struct Mouse mouse = { buffer_width / 2, buffer_height / 2 };

struct Layer {
	float yaw;
	int direction;
};

struct Layer layer;

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

void Win32ShowCursor(bool cursor) {
	ShowCursor(cursor);
}

void Win32DrawToWindow(HWND &window, void *image, int width, int height) {
	HDC hdc = GetDC(window);
	StretchDIBits(hdc, 0, 0, width, height, 0, 0, width, height, image, &buffer_bitmapinfo, DIB_RGB_COLORS, SRCCOPY);
}

// Alpha - around X axis, Beta - around Y axis, Gamma - around Z axis
void ProcessInput(HWND window, float &angleAlpha, float &angleBeta, float &angleGamma, int &forwardDirection, int &rightDirection, float &scaleVariable, float deltaTime) {
	MSG msg;

	if (GetAsyncKeyState(T_BUTTON)) {
		layer.direction = 1;
	}
	if (GetAsyncKeyState(A_BUTTON)) {
		angleBeta -= M_PI * deltaTime;
		if (angleBeta < 0) {
			angleBeta = 2 * M_PI;
		}
	}
	if (GetAsyncKeyState(D_BUTTON)) {
		angleBeta += M_PI * deltaTime;
		if (angleBeta > 2 * M_PI) {
			angleBeta = 0;
		}
	}

	while (PeekMessage(&msg, window, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		switch (msg.message) {
			// Mouse
			// NOTE: Turned out that editor's features require more knowledge than I expected.
			// So probably I put it away for a while.
			case WM_LBUTTONDOWN:
			case WM_LBUTTONUP: {

			} break;
			case WM_ACTIVATE: {
				OutputDebugStringA("Activated\n");
				SetCursorPos(buffer_width / 2, buffer_height / 2);
			} break;
			case WM_MOUSEMOVE: {
				mouse.x = GET_X_LPARAM(msg.lParam);
				mouse.y = GET_Y_LPARAM(msg.lParam);

				POINT pt = { buffer_width / 2, buffer_height / 2 };
				ClientToScreen(window, &pt);
				SetCursorPos(pt.x, pt.y);
			} break;
			case WM_MOUSEWHEEL: {
				int scroll = GET_WHEEL_DELTA_WPARAM(msg.wParam);
				float wheelSensitivity = 1.0f;

				mouse.wheelFOV += (float)scroll / fabs(scroll) * wheelSensitivity;
				if (mouse.wheelFOV >= 45.0f) {
					mouse.wheelFOV = 45.0f;
				}
				if (mouse.wheelFOV <= 1.0f) {
					mouse.wheelFOV = 1.0f;
				}
			} break;
				// Keyboard
			case WM_KEYUP:
			case WM_KEYDOWN: {
				UINT32 vkCode = msg.wParam;
				bool isKeyDown = ((msg.lParam >> 31) == 0);

				if (vkCode == VK_ESCAPE) {
					globalRunning = !globalRunning;
				}

				/*if (isKeyDown) {
					if (vkCode == D_BUTTON) {
						angleBeta += M_PI * deltaTime;
						if (angleBeta > 2 * M_PI) {
							angleBeta = 0;
						}
					}
					else if (vkCode == A_BUTTON) {
						angleBeta -= M_PI * deltaTime;
						if (angleBeta < 0) {
							angleBeta = 2 * M_PI;
						}
					}
					else if (vkCode == E_BUTTON) {
						angleGamma -= M_PI / 4 * deltaTime;
						if (angleGamma < 0) {
							angleGamma = 2 * M_PI;
						}
					}
					else if (vkCode == Q_BUTTON) {
						angleGamma += M_PI / 4 * deltaTime;
						if (angleGamma > 2 * M_PI) {
							angleGamma = 0;
						}
					}
					else if (vkCode == W_BUTTON) {
						angleAlpha -= M_PI / 4 * deltaTime;
						if (angleAlpha < 0) {
							angleAlpha = 2 * M_PI;
						}
					}
					else if (vkCode == S_BUTTON) {
						angleAlpha += M_PI / 4 * deltaTime;
						if (angleAlpha > 2 * M_PI) {
							angleAlpha = 0;
						}
					}
					else if (vkCode == J_BUTTON) {
						rightDirection = -1;
					}
					else if (vkCode == L_BUTTON) {
						rightDirection = 1;
					}
					else if (vkCode == I_BUTTON) {
						forwardDirection = 1;
					}
					else if (vkCode == K_BUTTON) {
						forwardDirection = -1;
					}
					else if (vkCode == VK_ESCAPE) {
						globalRunning = !globalRunning;
					}
					else if (vkCode == X_BUTTON) {
						scaleVariable *= 1.2f;
					}
					else if (vkCode == Z_BUTTON) {
						scaleVariable /= 1.2f;
					}
					else if (vkCode == P_BUTTON) {
						globalPause = !globalPause;
					}
					else if (vkCode == T_BUTTON) {
						layer.direction = 1;
					}
					else if (vkCode == G_BUTTON) {
						layer.direction = -1;
					}
					else if (vkCode == F_BUTTON) {
						layer.yaw += 1 * deltaTime;
					}
					else if (vkCode == H_BUTTON) {
						layer.yaw -= 1 * deltaTime;
					}
				}*/
			} break;

		}
	}
	return;
}
