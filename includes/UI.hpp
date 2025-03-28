#pragma once
#include "../INIT.hpp"
#include "../includes/Utility.hpp"
#include "../includes/Button.hpp"

class SceneComponent {
public:
    // virtual ~SceneComponent() = default;

    virtual void init()               = 0;
    virtual void update()             = 0;
    virtual void draw()               = 0;
    virtual void handleWindowResize() = 0;
    virtual void clean()              = 0;
};

class ReturnButtonComponent : public SceneComponent {
private:
    std::unique_ptr<ChangeSceneButton> buttons;
    Scene returnScene;
    int fontSize;

public:
    ReturnButtonComponent(Scene returnTo, int fontSize);
    void init() override;
    void update() override;
    void draw() override;
    void handleWindowResize() override;
    void clean() override;

    void setReturnScene(Scene scene);
    void setFontSize(int newSize);
};

// =========================================================

class Camera2DComponent : public SceneComponent {
private:
    Camera2D camera;
    bool isPanning;
    Vector2 lastMousePosition;
    float minZoom;
    float maxZoom;

public:
    Camera2DComponent();

    void init() override;
    void update() override;
    void draw() override; // This doesn't draw anything
    void handleWindowResize() override;
    void clean() override;

    // Camera control methods
    void beginMode();
    void endMode();
    void resetCamera();

    // Transform methods
    Vector2 screenToWorld(Vector2 position);
    Vector2 worldToScreen(Vector2 position);
};

// =========================================================

class SceneManager {
protected:
    int baseFontSize;
    int titleFontSize;
    std::vector<std::unique_ptr<SceneComponent>> components;

public:
    SceneManager();
    virtual ~SceneManager();
    virtual void init()   = 0;
    virtual void update() = 0;
    virtual void draw()   = 0;
    virtual void clean()  = 0;

    void updateFontSize();

    // Component management methods
    template<typename T, typename... Args>
    T* addComponent(Args&&... args) {
        auto component = std::make_unique<T>(std::forward<Args>(args)...);
        T* ptr         = component.get();
        components.push_back(std::move(component));
        return ptr;
    }

    void updateComponents();
    void drawComponents();
    void handleComponentsResize();
    void cleanComponents();
};

// =========================================================

class Title : public SceneManager {
private:
    Texture2D background;

public:
    void init() override;
    void update() override;
    void draw() override;
    void clean() override;
};

class Menu : public SceneManager {
private:
    std::vector<std::unique_ptr<Button>> buttons;

public:
    void init() override;
    void update() override;
    void draw() override;
    void clean() override;
};

// =========================================================

class Application {
private:
    std::vector<std::unique_ptr<SceneManager>> scenes;
    SceneManager* currentScene = nullptr;
    static Application* instance;
    static float lastSceneChangeTime;

public:
    Application();
    ~Application();

    static Application* getInstance();

    SceneManager* getCurScene();
    void changeScene(Scene newScene);
    void update();
    void draw();

    static bool canChangeScene();
    static constexpr float sceneChangeCooldown = 0.5f; // 0.5 second cooldown 
};