#include "INIT.hpp"
#include "includes/UI.hpp"

int main() {
    // There's constructor and destructor for Application
    Application app;
    while (!WindowShouldClose()) {
        app.getCurScene()->update();
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawCircle(0, 0, 30, RED);
        app.getCurScene()->draw();
        DrawFPS(GetScreenWidth() - 100, GetScreenHeight() - 50);
        EndDrawing();
    }
    app.getCurScene()->clean();
    return 0;
}
