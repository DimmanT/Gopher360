#pragma once
#include <windows.h>
#include <fstream>
#include <string>

class TrayIcon {
	NOTIFYICONDATA Icon = { 0 }; // Icon attributes
	static HWND consoleWindow;
	static HWND fakeWindow;
	static bool hidden;
public:
	static bool run;
public:
	TrayIcon(HWND console) {
		consoleWindow = console; //remember real console window

		//.... Make fake window.......
		WNDCLASSEX main = { 0 };
		main.cbSize = sizeof(WNDCLASSEX);
		main.hInstance = GetModuleHandle(NULL);
		main.lpszClassName = TEXT("MainFakeWindow");
		main.lpfnWndProc = TrayIcon::WndProc;
		RegisterClassEx(&main);

		// Create fake window for hooking messages
		fakeWindow = CreateWindowEx(0, TEXT("MainFakeWindow"), NULL, 0, 0, 0, 0, 0, NULL, NULL, GetModuleHandle(NULL), NULL);
		//..............

		//.... Create Icon ....
		Icon.cbSize = sizeof(NOTIFYICONDATA);
		Icon.hWnd = fakeWindow;
		Icon.uVersion = NOTIFYICON_VERSION;
		Icon.uCallbackMessage = WM_USER;
		Icon.hIcon = (HICON)GetClassLong(consoleWindow, GCL_HICON);
		Icon.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
		
		//setup icon name
		const auto W_NAME = L"Show/hide Gopher360.";
		std::copy(W_NAME, W_NAME + 21, Icon.szTip);

		Shell_NotifyIcon(NIM_ADD, &Icon);
		//...............................
	}
	~TrayIcon() {
		Shell_NotifyIcon(NIM_DELETE, &Icon);
	}

	void loadConfigFile() {
		using namespace std;
		fstream file;
		        file.open("config.ini", ios_base::in);
		if (file.is_open())
		{
			const string HIDE_ON_STARTUP = "HIDE_ON_STARTUP";
			string parameter;
			bool found{ false };
			while (!file.eof())
			{
				getline(file, parameter);
				if (parameter.substr(0, HIDE_ON_STARTUP.size()) == HIDE_ON_STARTUP) {
					found = true;
					remove_if(parameter.begin(), parameter.end(), isspace);
					auto i = find(parameter.begin(), parameter.end(), '=');
					if (i < parameter.end()-1) //need at least 1 character to decide
					{
						char value = *(i + 1);
						if (value == 'Y' || value == 'y' || value == '1') {
							ShowWindow(consoleWindow, SW_HIDE);
							hidden = true;
						}
					}
				}
			}
			file.close();

			if (!found) {
				file.open("config.ini", ios_base::out | ios_base::app);
				file.seekp(0, std::ios_base::end);
				file << '\n' << HIDE_ON_STARTUP << " = NO\n";
			}
			
		}
	}

	static LRESULT CALLBACK WndProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
	{
		switch (message)
		{
			// Message from icon
			case WM_USER:
				if (lParam == WM_RBUTTONUP || lParam == WM_LBUTTONUP)
				{
					if (hidden) {
						ShowWindow(consoleWindow, SW_NORMAL);
						hidden = false;
					} 
					else {
						ShowWindow(consoleWindow, SW_HIDE);
						hidden = true;
					}
				}
				break;

				//Default
			case WM_DESTROY:
				PostQuitMessage(0);
				break;
			default:
				return DefWindowProc(window, message, wParam, lParam);
		}
		return 0;
	}

	static BOOL WINAPI CtrlHandler(DWORD fdwCtrlType)
	{
		switch (fdwCtrlType)
		{
			default:
				return FALSE;
			case CTRL_C_EVENT:
				printf("Ctrl-C event\n\n");
				break;

			case CTRL_CLOSE_EVENT:
				printf("Ctrl-Close event\n\n");
				break;
		}
		run = false;
		DestroyWindow(fakeWindow);
		std::this_thread::sleep_for(std::chrono::milliseconds{ 222 }); //wait to finish main loop

		return true;
	}
};

HWND TrayIcon::consoleWindow = { NULL };
HWND TrayIcon::fakeWindow = { NULL };
bool TrayIcon::run = true;
bool TrayIcon::hidden = false;