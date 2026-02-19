#include "pch.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int cmdShow)
{
    // AMIGA NTSC FULLSCREEN
    // CPU_RUN(320, 200, true, true);	// AMIGA PAL FULLSCREEN
    //CPU_RUN(320, 256, true, true);

    WSADATA data;
    WSAStartup(MAKEWORD(2, 2), &data);

	Sleep(1000); // Wait for server to be ready
    // RETRO FULLSCREEN
    CPU_RUN(512, 256, false, true);

    // MODERN
    // CPU_RUN(1024, 576);

    // FULL HD (please use release)
    // CPU_RUN(1920, 1080, true);

    return 0;
}