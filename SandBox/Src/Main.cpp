#include "Game.h"

#if defined(_WIN32)
    #include <Windows.h>
    #include <VersionHelpers.h>
#endif

using namespace Bel;

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    srand(static_cast<unsigned int>(time(nullptr)));
    GameApp app;

    if (!app.Initialize())
    {
        return 1;
    }

    app.Run();
    app.Shutdown();

    return 0;
}