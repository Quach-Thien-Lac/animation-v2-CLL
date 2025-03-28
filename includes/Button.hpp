#pragma once
#include "../INIT.hpp"

class SceneManager;

class Button {
protected:
    Rectangle bounds;

    Color normalColor;
    Color hoverColor;
    Color clickColor;
    Color textColor;

    const char* text;
    int fontSize;
    bool isHovered;

public:
    Button(Rectangle bounds, const char* buttonText, int fontSize, Color normal = LIGHTGRAY, Color hover = YELLOW, Color click = RED, Color textCol = BLACK);
    virtual ~Button() = default;

    virtual void update();
    virtual void draw() = 0;
    virtual bool isClicked();

    // Getters and setters
    Rectangle getBounds() const;
    void setBounds(Rectangle newBounds);
    void setPosition(float x, float y);
    void setColors(Color normal, Color hover, Color text);
    void setText(const char* buttonText);
    void setFontSize(int size);

protected:
    void drawButtonBackground(Color boxColor);
    void drawCenteredText(float yOffset = 0.0f);
};

// =========================================================

class ChangeSceneButton : public Button {
private:
    Scene targetScene;
    Texture2D icon;
    bool ownTexture; // Whether we loaded the texture and should unload it
    bool isBackButton;

public:
    // Constructor with texture path
    ChangeSceneButton(Rectangle bounds, const char* buttonText, int fontSize, Scene destination, const char* iconPath, Color normal = LIGHTGRAY, Color hover = GRAY, Color click = RED, Color textCol = BLACK, bool isBack = false);

    ~ChangeSceneButton() override;

    void update() override;
    void draw() override;

    Scene getTargetScene() const;
    void setTargetScene(Scene newTarget);
    void setIcon(const char* iconPath);
    bool isBack() const;
    void setIsBack(bool back);
};

class ActionButton : public Button {
private:
    std::function<void()> clickAction;

public:
    ActionButton(Rectangle bounds, const char* buttonText, int fontSize,
                 std::function<void()> action,
                 Color normal = LIGHTGRAY, Color hover = LIGHTGRAY,
                 Color click = GRAY, Color textCol = BLACK);

    void update() override;
    void draw() override;
};