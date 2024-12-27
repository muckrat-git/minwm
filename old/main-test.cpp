#include <raylib.h>
#include "WM.cpp"

int main(int argc, char *argv[]) {
    // Get starting dimensions
    int w = 1200;
    int h = 900;
    if(argc == 2) {
        w = stoi(argv[1]);
        h = stoi(argv[2]);
    } 

    SetConfigFlags(FLAG_VSYNC_HINT);
    InitWindow(w, h, "minwm test");
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
        ClearBackground(BLACK);

        WM::Draw();

        EndDrawing();

        // Update windows
        WM::Update();
    }

    WM::Unload();
    CloseWindow();
}