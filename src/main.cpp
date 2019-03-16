#include "ofMain.h"
#include "ofApp.h"
//#include "../resource.h"
/*
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {

	ofGLFWWindowSettings settings;
	settings.width = 800;
	settings.height = 500;
	settings.resizable = false;
	ofCreateWindow(settings);

	HWND hwnd = ofGetWin32Window();
	HICON hMyIcon = LoadIcon(hInstance, MAKEINTRESOURCE(MAIN_ICON));
	SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hMyIcon);

	ofRunApp(new ofApp());
}
*/

int main() {
	ofSetupOpenGL(800, 500, OF_WINDOW);
	ofRunApp(new ofApp());
}

