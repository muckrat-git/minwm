#include <functional>
#include <iostream>
#include <vector>
#include <raylib.h>
#include <AnimValue.cpp>

#define LINE_COLOR (Color){102, 102, 102, 255}
#define THRESHOLD (Vector2){10, 20}

using namespace std;

Vector2 GetCenter(Rectangle rect) {
    return { rect.x + rect.width / 2, rect.y + rect.height / 2 };
}

namespace WM {
    // - Window Manager Settings -
    float scale = 1;    // GUI Scale
    Font font;          // Font for the wm to use;

    class Window {
        private:
        function<void(Window*)> _renderFunc; 
        Image background;

        public:
        int index = 0;
        AnimValue<Rectangle> frame;     // Window position and size
        Vector2 defaultFrame;           // Default window size
        string name;                    // Window title
        bool dragging = false;          // If the window is being dragged
        bool resizing = false;          // If the window is being resized
        Vector2 grabPos = {0};          // Position from which the window is being dragged
        Vector2 minimum;                // The minimum frame size

        // Constructor
        Window(string name, Rectangle frame, function<void(Window*)> renderFunc) {
            this->name = name;
            this->frame = AnimValue((Rectangle){ frame.x, frame.y, 0, 30 });
            frame.height += 30; // 20px for tilebar
            this->frame.Set(frame, 0.2);
            this->_renderFunc = renderFunc;
            this->defaultFrame = {frame.width, frame.height};

            // Calculate minimum width
            const float fontSize = 17;
            Vector2 titleFrame = MeasureTextEx(font, name.c_str(), fontSize, 0);
            Vector2 btnFrame = MeasureTextEx(font, "- x", fontSize, 0);
            minimum.x = titleFrame.x + 5 * 6 + 60;
            minimum.y = 30;

            // Generate background
            background = GenImageColor(frame.width, frame.height, BLACK);
        }

        Rectangle GetTilebarFrame() {
            Rectangle drawFrame = frame.Get();
            return {
                drawFrame.x, drawFrame.y,
                drawFrame.width, 30
            };
        }

        Rectangle GetResizeBtn() {
            Rectangle drawFrame = frame.Get();
            if(drawFrame.height <= minimum.y + THRESHOLD.y) {
                Rectangle tilebar = GetTilebarFrame();
                return {
                    tilebar.x + tilebar.width - 5, 
                    tilebar.y, 
                    5,
                    tilebar.height
                };
            }
            drawFrame.x += drawFrame.width - 20;
            drawFrame.y += drawFrame.height - 20;
            drawFrame.width = 20;
            drawFrame.height = 20;
            return drawFrame;
        }

        void Draw() {
            // Get relevant frames
            Rectangle drawFrame = frame.Get();
            Rectangle tilebar = GetTilebarFrame();

            bool rolled = drawFrame.height <= minimum.y + THRESHOLD.y;

            // Draw window base
            DrawRectangle(drawFrame.x, drawFrame.y, drawFrame.width, 30, (Color){60, 60, 60, 102});
            DrawRectangle(drawFrame.x, drawFrame.y, drawFrame.width, drawFrame.height, (Color){20, 20, 20, 178});
            DrawRectangleLinesEx(drawFrame, 1, LINE_COLOR);

            // Draw Tilebar
            float fontSize = 17;
            Vector2 titleFrame = MeasureTextEx(font, name.c_str(), fontSize, 0);
            DrawTextEx(font, name.c_str(), GetCenter(tilebar) - titleFrame / 2, fontSize, 0, (Color){255, 255, 255, 214});

            // Tilebar buttons
            fontSize = 19;
            Vector2 btnFrame = MeasureTextEx(font, "- x", fontSize, 0);
            Vector2 buttonBase = (Vector2){tilebar.x + tilebar.width - (rolled ? 15 : 10), tilebar.y + tilebar.height / 2} - btnFrame * (Vector2){0, 0.5};
            float buttonSize = btnFrame.x / 3;
            
            // Check button distance (minimize)
            Color btnColor = (Color){255, 255, 255, 170};
            if(Vector2Distance(GetMousePosition(), buttonBase - (Vector2){buttonSize * 3 - buttonSize / 2, - buttonSize}) < buttonSize) {
                btnColor = (Color){255, 255, 255, 214};

                // Check for press
                if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    if(!rolled) {
                        defaultFrame.y = drawFrame.height;
                        frame.Set({drawFrame.x, drawFrame.y, drawFrame.width, 30}, 0.1);
                    }
                    else
                        frame.Set({drawFrame.x, drawFrame.y, drawFrame.width, defaultFrame.y}, 0.1);
                }

            }
            DrawTextEx(font, "-", buttonBase - (Vector2){btnFrame.x, 0}, fontSize, 0, btnColor);

            // Check button distance (exit)
            btnColor = (Color){255, 255, 255, 170};
            if(Vector2Distance(GetMousePosition(), buttonBase - (Vector2){buttonSize - buttonSize / 2, - buttonSize}) < buttonSize) {
                btnColor = (Color){255, 255, 255, 214};

                // Check for press
                if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    // Set minimum to 0,0 and close
                    minimum = {0,0};
                    frame.Set({drawFrame.x, drawFrame.y, drawFrame.width, 0}, 0.1);
                }
            }
            DrawTextEx(font, "x", buttonBase - (Vector2){btnFrame.x / 3, 0}, fontSize, 0, btnColor);

            // Don't bother drawing corner resize or rendering content if rolled
            if(rolled) {
                // Draw tilebar resize btn
                DrawLine(tilebar.x + tilebar.width - 5, tilebar.y, tilebar.x + tilebar.width - 5, tilebar.y + tilebar.height, LINE_COLOR); 
                return;
            }

            // Corner resize btn
            DrawLine(drawFrame.x + drawFrame.width - 20, drawFrame.y + drawFrame.height, drawFrame.x + drawFrame.width, drawFrame.y + drawFrame.height - 20, (Color){102, 102, 102, 140});
            DrawLine(drawFrame.x + drawFrame.width - 10, drawFrame.y + drawFrame.height - 5, drawFrame.x + drawFrame.width - 5, drawFrame.y + drawFrame.height - 10, (Color){102, 102, 102, 140});

            // Skip other drawing if closing
            if(minimum.x == 0 && minimum.y == 0) return;

            // Call internal render function
            if(_renderFunc != nullptr) _renderFunc(this); 
        }

        // Processes window interactions, returns true if an interaction occured
        bool DoInteractions(vector<Window> * windows) {
            // Get mouse
            Vector2 mouse = GetMousePosition();

            // Get current frame
            Rectangle current = frame.Get();

            // Check dragging
            if(this->dragging) {
                // If mouse down
                SetMouseCursor(MOUSE_CURSOR_RESIZE_ALL);
                current.x += mouse.x - current.x - grabPos.x;
                current.y += mouse.y - current.y - grabPos.y;

                // Check collisions
                for(Window w : *windows) {
                    if(w.index == index) continue;

                    Rectangle wFrame = w.frame.Get();
                    if(CheckCollisionRecs(current, wFrame)) {
                        // Snap each edge

                        // T-T & B-B
                        if(abs(current.y - wFrame.y) < 20) current.y = wFrame.y;
                        else if(abs(current.y - (wFrame.y + wFrame.height) + current.height) < 20) current.y = wFrame.y + wFrame.height - current.height - 1;
                        
                        // T-B & B-T
                        if(abs(current.y - (wFrame.y + wFrame.height)) < 20) current.y = wFrame.y + wFrame.height - 1;
                        else if(abs(current.y - wFrame.y + current.height) < 20) current.y = wFrame.y - current.height;
                        
                        // L-L & R-R
                        if(abs(current.x - wFrame.x) < 20) current.x = wFrame.x;
                        else if(abs(current.x - (wFrame.x + wFrame.width) + current.width) < 20) current.x = wFrame.x + wFrame.width - current.width + 1;

                        // L-R & R-L
                        if(abs(current.x - (wFrame.x + wFrame.width)) < 20) current.x = wFrame.x + wFrame.width - 1;
                        else if(abs(current.x - wFrame.x + current.width) < 20) current.x = wFrame.x - current.width + 1;
                        
                    }
                }

                frame.Set(current, 0.03);

                if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) this->dragging = false;
                return true;
            }

            // Check resizing
            if(this->resizing) {
                // If mouse down
                SetMouseCursor(MOUSE_CURSOR_RESIZE_NWSE);

                // Calculate delta
                Vector2 delta = GetMousePosition() - (Vector2){current.x+current.width, current.y+current.height};
                current.width += delta.x;
                current.height += delta.y;

                // Snap width and height down
                if(current.width < minimum.x + THRESHOLD.x) current.width = minimum.x;
                if(current.height < minimum.y + THRESHOLD.y) current.height = minimum.y;

                frame.Set(current, 0.05);

                if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) this->resizing = false;
                return true;
            }
            
            // Check resize button
            if(CheckCollisionPointRec(mouse, GetResizeBtn())) {
                SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);

                if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    this->resizing = true;
                    return true;
                }
                return false;
            }

            // Check tilebar
            Rectangle tilebar = GetTilebarFrame();
            tilebar.width -= 40;
            if(CheckCollisionPointRec(mouse, tilebar)) {
                SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);

                if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    this->dragging = true;
                    this->grabPos = GetMousePosition() - (Vector2){current.x, current.y};
                    return true;
                }
            }

            return false;
        }
    };

    vector<Window> windows;

    // Draw the window gui
    void Draw() {
        for(Window & w : windows) w.Draw();
    }

    // Update windows
    void Update() {
        SetMouseCursor(MOUSE_CURSOR_DEFAULT);
        for(int i = 0; i < windows.size(); ++i) {
            Window & w = windows[i];
            w.index = i;

            // Check for closed windows
            if(w.frame.Get().height == 0) {
                windows.erase(windows.begin() + i);
                --i;
            }

            if(w.DoInteractions(&windows)) {
                // Move w to back of queue
                Window tmp = windows[windows.size()-1];
                windows[windows.size()-1] = w;
                w = tmp;
                break;
            }
        }
    }

    // Load font and other data
    void Load() {
        WM::font = LoadFontEx("RobotoMono-Regular.ttf", 42, NULL, 0);
        Image img = LoadImageFromTexture(WM::font.texture);
        ImageFormat(&img, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
        ImageResize(&img, img.width * 2, img.height * 2);
        ImageBlurGaussian(&img, 1);       
        ImageResize(&img, img.width / 2, img.height / 2);
        UnloadTexture(WM::font.texture);
        WM::font.texture = LoadTextureFromImage(img);
        UnloadImage(img);
        SetTextureFilter(WM::font.texture, TEXTURE_FILTER_BILINEAR);
    }

    // Unload everything
    void Unload() {
        UnloadFont(WM::font);
    }

    // Make a new window
    void Create(Window win) {
        windows.push_back(win);
    }
};