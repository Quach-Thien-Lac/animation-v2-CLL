#include "../INIT.hpp"
#include "../includes/UI.hpp"
#include "../includes/Button.hpp"
#include "../includes/Utility.hpp"

#include "../includes/AVLTree.hpp"
#include "../includes/Graph.hpp"
#include "../includes/HashTable.hpp"
#include "../includes/LinkedList.hpp"

Application* Application::instance     = nullptr;
float Application::lastSceneChangeTime = 0.0f;

// =========================================================

ReturnButtonComponent::ReturnButtonComponent(Scene returnTo, int fontSize)
    : returnScene(returnTo), fontSize(fontSize) {}

void ReturnButtonComponent::init()
{
    buttons = std::make_unique<ChangeSceneButton>(
        ScreenLayout::CalculateReturnButtonPosition(),
        "Back",
        fontSize,
        returnScene,
        "images/left_arrow.png",
        SKYBLUE, BLUE, RED, BLACK,
        true); // isBack = true
}

void ReturnButtonComponent::update()
{
    if (buttons) {
        buttons->update();
    }
}

void ReturnButtonComponent::draw()
{
    if (buttons) {
        buttons->draw();
    }
}

void ReturnButtonComponent::handleWindowResize()
{
    if (buttons) {
        buttons->setBounds(ScreenLayout::CalculateReturnButtonPosition());
    }
}

void ReturnButtonComponent::clean()
{
    buttons.reset();
}

void ReturnButtonComponent::setReturnScene(Scene scene)
{
    returnScene = scene;
    if (buttons) {
        buttons->setTargetScene(scene);
    }
}

void ReturnButtonComponent::setFontSize(int newSize)
{
    fontSize = newSize;
    if (buttons) {
        buttons->setFontSize(fontSize);
    }
}

// =========================================================

Camera2DComponent::Camera2DComponent() : isPanning(false), minZoom(0.1f), maxZoom(5.0f)
{
    resetCamera();
}

void Camera2DComponent::init()
{
    resetCamera();
}

void Camera2DComponent::update()
{
    // Zoom with mouse wheel
    float wheel = GetMouseWheelMove();
    if (wheel != 0) {
        // Get mouse position before zoom for better zooming at cursor position
        Vector2 mouseWorldPos = screenToWorld(GetMousePosition());

        // Apply zoom
        camera.zoom += wheel * 0.1f * camera.zoom;
        camera.zoom = Clamp(camera.zoom, minZoom, maxZoom);

        // Adjust target to keep the mouse position fixed after zoom
        Vector2 mouseWorldPosAfter = screenToWorld(GetMousePosition());
        camera.target.x += mouseWorldPos.x - mouseWorldPosAfter.x;
        camera.target.y += mouseWorldPos.y - mouseWorldPosAfter.y;
    }

    // Panning/ Dragging with middle mouse button
    if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE) || IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
        Vector2 currentMousePos = GetMousePosition();

        if (!isPanning) {
            lastMousePosition = currentMousePos;
            isPanning         = true;
        }
        else {
            Vector2 delta = Vector2Subtract(lastMousePosition, currentMousePos);
            // Scale delta by inverse zoom factor
            delta.x /= camera.zoom;
            delta.y /= camera.zoom;

            camera.target.x += delta.x;
            camera.target.y += delta.y;

            lastMousePosition = currentMousePos;
        }
    }
    else {
        isPanning = false;
    }

    // Reset camera with R key
    if (IsKeyPressed(KEY_R)) {
        resetCamera();
    }
}

void Camera2DComponent::draw()
{
    // It's part of the component lifecycle, but actual camera use is handled by beginMode/endMode
}

void Camera2DComponent::beginMode()
{
    BeginMode2D(camera);
}

void Camera2DComponent::endMode()
{
    EndMode2D();
}

void Camera2DComponent::handleWindowResize()
{
    // Update offset on window resize to keep center point
    camera.offset = Vector2{
        static_cast<float>(GetScreenWidth()) / 2.0f,
        static_cast<float>(GetScreenHeight()) / 2.0f};
}

void Camera2DComponent::clean()
{
    // Nothing to clean up
}

void Camera2DComponent::resetCamera()
{
    float centerX   = GetScreenWidth() / 2.0f;
    float centerY   = GetScreenHeight() / 2.0f;
    camera.target   = Vector2{0, 0};
    camera.offset   = Vector2{centerX, centerY};
    camera.rotation = 0.0f;
    camera.zoom     = 1.0f;
}

Vector2 Camera2DComponent::screenToWorld(Vector2 position)
{
    return GetScreenToWorld2D(position, camera);
}

Vector2 Camera2DComponent::worldToScreen(Vector2 position)
{
    return GetWorldToScreen2D(position, camera);
}

// =========================================================

SceneManager::SceneManager()
{
    baseFontSize  = static_cast<int>(GetScreenWidth() * 0.025f);
    titleFontSize = static_cast<int>(baseFontSize * 2.5f);
}

SceneManager::~SceneManager()
{
    cleanComponents();
}

void SceneManager::updateFontSize()
{
    baseFontSize  = static_cast<int>(GetScreenWidth() * 0.025f);
    titleFontSize = static_cast<int>(baseFontSize * 2.5f);
}

void SceneManager::updateComponents()
{
    size_t i = 0;
    while (i < components.size())
    {
        components[i]->update();
        if (Application::getInstance()->getCurScene() != this)
            break;
        ++i;
    }
}

void SceneManager::drawComponents()
{
    for (auto& component : components) {
        component->draw();
    }
}

void SceneManager::handleComponentsResize()
{
    for (auto& component : components) {
        component->handleWindowResize();
        // Check if the component is a ReturnButtonComponent
        auto* returnButton = dynamic_cast<ReturnButtonComponent*>(component.get());
        if (returnButton) {
            returnButton->setFontSize(baseFontSize);
        }
    }
}

void SceneManager::cleanComponents()
{
    for (auto& component : components) {
        component->clean();
    }
    components.clear();
}

// =========================================================

void Title::init()
{
    background = LoadTexture("images/a_ship_in_the_water.jpg");
}

void Title::update()
{
    if (IsWindowResized())
        updateFontSize();

    if (Application::canChangeScene()) {
        if (IsKeyPressed(KEY_ENTER) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            Application::getInstance()->changeScene(Scene::MENU);
        if (IsKeyPressed(KEY_L))
            Application::getInstance()->changeScene(Scene::LINKEDLIST);
    }
}

void Title::draw()
{
    float scale = fmaxf((float)GetScreenWidth() / background.width, (float)GetScreenHeight() / background.height);
    DrawTextureEx(background, Vector2{0, 0}, 0, scale, WHITE);

    float elapsedTime = static_cast<float>(GetTime());
    float alpha       = 0.7f + 0.3f * sinf(elapsedTime * 2.0f);
    Color textColor   = {255, 255, 255, static_cast<unsigned char>(alpha * 255)};

    const char* titleName = "Data Structure Visualizer";
    DrawText(titleName, (GetScreenWidth() - MeasureText(titleName, titleFontSize)) / 2, GetScreenHeight() / 5, titleFontSize, WHITE);

    const char* guideText = "Press Enter to continue...";
    DrawText(guideText, (GetScreenWidth() - MeasureText(guideText, baseFontSize)) / 3, GetScreenHeight() * 4 / 5, baseFontSize, textColor);
}

void Title::clean()
{
    UnloadTexture(background);
}

// =========================================================

void Menu::init()
{
    // Clear any existing buttons (in case this is called multiple times)
    buttons.clear();

    // Calculate positions for all 4 buttons at once
    std::vector<Rectangle> buttonPositions = ScreenLayout::CalculateButtonGrid(
        4,     // 4 buttons
        0.2f,  // button width: 20% of screen width
        0.2f,  // button height: 20% of screen height
        0.01f, // spacing: 1% of screen width
        0.5f   // vertical position: middle of screen
    );

    // Add Scene Selection Buttons with icons - positioned horizontally
    buttons.push_back(std::make_unique<ChangeSceneButton>(
        buttonPositions[0],
        "Hash Table",
        baseFontSize,
        Scene::HASHTABLE,
        "images/img_holder.jpg",
        SKYBLUE,
        BLUE,
        RED,
        BLACK,
        false));

    buttons.push_back(std::make_unique<ChangeSceneButton>(
        buttonPositions[1],
        "Linked List",
        baseFontSize,
        Scene::LINKEDLIST,
        "images/img_holder.jpg",
        SKYBLUE,
        BLUE,
        RED,
        BLACK,
        false));

    buttons.push_back(std::make_unique<ChangeSceneButton>(
        buttonPositions[2],
        "AVL Tree",
        baseFontSize,
        Scene::AVLTREE,
        "images/img_holder.jpg",
        SKYBLUE,
        BLUE,
        BLACK,
        BLACK,
        false));

    buttons.push_back(std::make_unique<ChangeSceneButton>(
        buttonPositions[3],
        "Graph",
        baseFontSize,
        Scene::GRAPH,
        "images/img_holder.jpg",
        SKYBLUE,
        BLUE,
        BLACK,
        BLACK,
        false));

    // Add return button component (returns to title)
    addComponent<ReturnButtonComponent>(Scene::TITLE, baseFontSize)->init();
}

void Menu::update()
{
    // Update regular buttons
    size_t i = 0;
    while (i < buttons.size()) {
        buttons[i]->update();
        if (Application::getInstance()->getCurScene() != this) // To not access nullptr
            break;
        ++i;
    }

    // Update components (including return button)
    updateComponents();

    if (IsWindowResized()) {
        updateFontSize();

        // Handle component resizing (for return button)
        handleComponentsResize();

        std::vector<Rectangle> buttonPositions = ScreenLayout::CalculateButtonGrid(
            static_cast<int>(buttons.size()),
            0.2f,  // button width: 20% of screen width
            0.2f,  // button height: 20% of screen height
            0.01f, // spacing: 1% of screen width
            0.5f   // vertical position: middle of screen
        );

        for (size_t i = 0; i < buttons.size(); ++i) {
            buttons[i]->setBounds(buttonPositions[i]);
            buttons[i]->setFontSize(baseFontSize);
        }
    }
}

void Menu::draw()
{
    ClearBackground(RAYWHITE);

    const char* titleName = "Data Structure Visualizer";
    DrawText(titleName, (GetScreenWidth() - MeasureText(titleName, titleFontSize)) / 2, GetScreenHeight() / 5, titleFontSize, BLACK);

    // Draw all regular buttons
    for (auto& button : buttons) {
        button->draw();
    }

    // Draw components (return button)
    drawComponents();
}

void Menu::clean()
{
    buttons.clear();
    // UnloadTexture(menuBackground);
    cleanComponents();
}

// =========================================================

Application::Application()
{

    SetTargetFPS(60);
    InitWindow(960, 540, "CS163");
    SetExitKey(KEY_ESCAPE);

    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetWindowMaxSize(1920, 1080);
    SetWindowMinSize(960, 540);

    instance = this; // To get instance of the Application from child

    scenes.resize(6);
    scenes[static_cast<int>(Scene::TITLE)]      = std::make_unique<Title>();
    scenes[static_cast<int>(Scene::MENU)]       = std::make_unique<Menu>();
    scenes[static_cast<int>(Scene::HASHTABLE)]  = std::make_unique<HashTable>();
    scenes[static_cast<int>(Scene::LINKEDLIST)] = std::make_unique<LinkedList>();
    scenes[static_cast<int>(Scene::AVLTREE)]    = std::make_unique<AVLTree>();
    scenes[static_cast<int>(Scene::GRAPH)]      = std::make_unique<Graph>();

    currentScene = scenes[static_cast<int>(Scene::TITLE)].get();
    if (currentScene)
        currentScene->init();
}

Application::~Application()
{
    if (currentScene)
        currentScene->clean();

    CloseWindow();
}

void Application::update()
{
    if (currentScene) {
        currentScene->update();
    }
}

void Application::draw()
{
    if (currentScene)
        currentScene->draw();
}

Application* Application::getInstance()
{
    return instance;
}

SceneManager* Application::getCurScene()
{
    return currentScene;
}

void Application::changeScene(Scene newScene)
{
    if (!canChangeScene())
        return;

    int sceneIndex = static_cast<int>(newScene);
    if (sceneIndex >= 0 && sceneIndex < 6 && scenes[sceneIndex]) {
        if (currentScene)
            currentScene->clean();
        currentScene = scenes[sceneIndex].get();
        currentScene->init();

        // Update last change scene
        lastSceneChangeTime = static_cast<float>(GetTime());
    }
}

bool Application::canChangeScene()
{
    return (GetTime() - lastSceneChangeTime) >= sceneChangeCooldown;
}