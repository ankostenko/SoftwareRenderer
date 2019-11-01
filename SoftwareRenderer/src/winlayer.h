#pragma once

extern bool globalRunning;
extern bool globalPause;

#include <windows.h>
#define _USE_MATH_DEFINES
#include <math.h>
#undef max
#undef min
#include "Image.h"

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

void Win32DrawToWindow(HWND &window, Image &image) {
	HDC hdc = GetDC(window);
	image.flip_vertically();
	StretchDIBits(hdc, 0, 0, image.width, image.height, 0, 0, image.width, image.height, image.data, &buffer_bitmapinfo, DIB_RGB_COLORS, SRCCOPY);
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
void ProcessInput(HWND window, float &angleTheta, float &anglePhi, float &cameraAngleTheta, float &cameraAnglePhi, float &scaleVariable) {
	MSG msg;

	if (PeekMessage(&msg, window, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	switch (msg.message) {
		case WM_KEYUP:
		case WM_KEYDOWN: {
			UINT32 vkCode = msg.wParam;
			bool isKeyDown = ((msg.lParam >> 31) == 0);
			if (isKeyDown) {
				if (vkCode == D_BUTTON) {
					angleTheta += M_PI / 16;
					if (angleTheta > 2 * M_PI) {
						angleTheta = 0;
					}
				} else if (vkCode == A_BUTTON) {
					angleTheta -= M_PI / 16;
					if (angleTheta < 0) {
						angleTheta = 2 * M_PI;
					}
				} else if (vkCode == R_BUTTON) {
					anglePhi += M_PI / 32;
					if (anglePhi > M_PI / 2) {
						anglePhi = M_PI / 2;
					}
				} else if (vkCode == S_BUTTON) {
					anglePhi -= M_PI / 32;
					if (anglePhi < -M_PI / 2) {
						anglePhi = -M_PI / 2;
					}
				} else if (vkCode == J_BUTTON) {
					cameraAngleTheta += M_PI / 16;
					if (cameraAngleTheta > 2 * M_PI) {
						cameraAngleTheta = 0;
					}
				} else if (vkCode == L_BUTTON) {
					cameraAngleTheta -= M_PI / 16;
					if (cameraAngleTheta < 0) {
						cameraAngleTheta = 2 * M_PI;
					}
				} else if (vkCode == I_BUTTON) {
					cameraAnglePhi += M_PI / 16;
					if (cameraAngleTheta > M_PI) {
						cameraAngleTheta = 0;
					}
				} else if (vkCode == K_BUTTON) {
					cameraAnglePhi -= M_PI / 16;
					if (cameraAngleTheta < 0) {
						cameraAngleTheta = M_PI;
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
