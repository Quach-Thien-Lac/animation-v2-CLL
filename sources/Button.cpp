#include "../includes/UI.hpp"
#include "../includes/Button.hpp"

Button::Button(Rectangle bounds, const char* buttonText, int fontSize, Color normal, Color hover, Color click, Color textColor)
    : bounds(bounds),
      text(buttonText),
      fontSize(fontSize),
      normalColor(normal),
      hoverColor(hover),
      clickColor(click),
      textColor(textColor),
      isHovered(false)
{
    fontSize = 20; // Default
}

void Button::update()
{
    Vector2 mousePos = GetMousePosition();
    isHovered        = CheckCollisionPointRec(mousePos, bounds);
}

void Button::draw()
{
    Color boxColor = isHovered ? hoverColor : normalColor;
    drawButtonBackground(boxColor);
    drawCenteredText();
}

bool Button::isClicked()
{
    return isHovered && IsMouseButtonReleased(MOUSE_BUTTON_LEFT);
}

Rectangle Button::getBounds() const
{
    return bounds;
}

void Button::setBounds(Rectangle newBounds)
{
    bounds = newBounds;
}

void Button::setPosition(float x, float y)
{
    bounds.x = x;
    bounds.y = y;
}

void Button::setColors(Color normal, Color hover, Color text)
{
    normalColor = normal;
    hoverColor  = hover;
    textColor   = text;
}

void Button::setText(const char* buttonText)
{
    text = buttonText;
}

void Button::setFontSize(int size)
{
    fontSize = size;
}

void Button::drawButtonBackground(Color boxColor)
{
    DrawRectangleRec(bounds, boxColor);
    DrawRectangleLinesEx(bounds, 2, isHovered ? BLACK : DARKGRAY);
}

void Button::drawCenteredText(float yOffset)
{
    if (text && text[0] != '\0') {
        int textWidth = MeasureText(text, fontSize);
        float textX   = bounds.x + (bounds.width - textWidth) * 0.5f;
        float textY   = bounds.y + (bounds.height - fontSize) * 0.5f + yOffset;
        DrawText(text, static_cast<int>(textX), static_cast<int>(textY), fontSize, textColor);
    }
}

// =========================================================
ChangeSceneButton::ChangeSceneButton(Rectangle bounds, const char* buttonText, int fontSize, Scene destination, const char* iconPath, Color normalColor, Color hoverColor, Color clickColor, Color textColor, bool isBack)
    : Button(bounds, buttonText, fontSize, normalColor, hoverColor, clickColor, textColor),
      targetScene(destination), ownTexture(true), isBackButton(isBack)
{
    icon = LoadTexture(iconPath);
    if (isBackButton) {
        fontSize = static_cast<int>(fontSize * 0.9f);
    }
}

ChangeSceneButton::~ChangeSceneButton()
{
    if (ownTexture) {
        UnloadTexture(icon);
    }
}

void ChangeSceneButton::update()
{
    Button::update();

    if (isClicked()) {
        Application::getInstance()->changeScene(targetScene);
    }
}

void ChangeSceneButton::draw()
{
    Color boxColor = normalColor;
    if (isHovered) {
        boxColor = IsMouseButtonDown(MOUSE_BUTTON_LEFT) ? clickColor : hoverColor;
    }

    // Draw the icon and borders
    DrawTexturePro(icon, Rectangle{0, 0, static_cast<float>(icon.width), static_cast<float>(icon.height)}, bounds, Vector2{0, 0}, 0.0f, boxColor);
    DrawRectangleLinesEx(bounds, 2, isHovered ? BLACK : DARKGRAY);

    if (!isBackButton) 
        drawCenteredText(4.0f*fontSize);
}

Scene ChangeSceneButton::getTargetScene() const
{
    return targetScene;
}

void ChangeSceneButton::setTargetScene(Scene newTarget)
{
    targetScene = newTarget;
}

void ChangeSceneButton::setIcon(const char* iconPath)
{
    if (ownTexture) {
        UnloadTexture(icon);
    }

    icon       = LoadTexture(iconPath);
    ownTexture = true;
}

bool ChangeSceneButton::isBack() const
{
    return isBackButton;
}

void ChangeSceneButton::setIsBack(bool back)
{
    isBackButton = back;
}

// =========================================================

ActionButton::ActionButton(Rectangle bounds, const char* buttonText, int fontSize,
             std::function<void()> action, Color normal, Color hover, Color click, Color textCol): Button(bounds, buttonText, fontSize, normal, hover, click, textCol),
      clickAction(action) {}

void ActionButton::update()
{
    Button::update();
    if (isClicked() && clickAction) {
        clickAction();
    }
}

void ActionButton::draw()
{
    Color boxColor = isHovered ? hoverColor : normalColor;
    drawButtonBackground(boxColor);
    drawCenteredText();
}