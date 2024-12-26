#include <raylib.h>
#include "WM.cpp"

int main(int argc, char *argv[]) {
    // Get starting dimensions
    int w = 1200;
    int h = 900;
    if(argc == 2) {
        w = stoi(argv[0]);
        h = stoi(argv[1]);
    } 

    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_TRANSPARENT | FLAG_FULLSCREEN_MODE);
    InitWindow(1200, 900, "minwm test");
    WM::Load();

    WM::Create(WM::Window( 
        "Test Window",
        {30, 30, 480, 300},
        function<void(WM::Window*)>([](WM::Window* self) { 

        })
    ));
    WM::Create(WM::Window( 
        "Test Window 2",
        {30, 30, 480, 300},
        function<void(WM::Window*)>([](WM::Window* self) { 

        })
    ));

    while(!WindowShouldClose()) {
        // Draw WM
        BeginDrawing();
        ClearBackground(BLANK);

        WM::Draw();

        EndDrawing();

        // Update windows
        WM::Update();
    }

    WM::Unload();
    CloseWindow();
}