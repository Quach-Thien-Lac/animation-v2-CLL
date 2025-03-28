#include "../includes/Utility.hpp"


int ScreenLayout::GetScreenRelativeWidth(float percentage)
{
    return static_cast<int>(GetScreenWidth() * percentage);
}

int ScreenLayout::GetScreenRelativeHeight(float percentage)
{
    return static_cast<int>(GetScreenHeight() * percentage);
}

int ScreenLayout::GetScreenRelativeSpacing(float percentage)
{
    return static_cast<int>(GetScreenWidth() * percentage);
}

Rectangle ScreenLayout::CalculateButtonPosition(int buttonIndex, int totalButtons, float buttonWidthPercent, float buttonHeightPercent, float buttonSpacingPercent, float verticalPositionPercent)
{

    int screenWidth   = GetScreenWidth();
    int screenHeight  = GetScreenHeight();
    int buttonWidth   = GetScreenRelativeWidth(buttonWidthPercent);
    int buttonHeight  = GetScreenRelativeHeight(buttonHeightPercent);
    int buttonSpacing = GetScreenRelativeSpacing(buttonSpacingPercent);

    int totalButtonsWidth = totalButtons * buttonWidth + (totalButtons - 1) * buttonSpacing;
    int startX            = (screenWidth - totalButtonsWidth) / 2;
    int startY            = static_cast<int>(screenHeight * verticalPositionPercent);

    return Rectangle{
        (float)(startX + buttonIndex * (buttonWidth + buttonSpacing)),
        (float)startY,
        (float)buttonWidth,
        (float)buttonHeight};
}

std::vector<Rectangle> ScreenLayout::CalculateButtonGrid(int numButtons, float buttonWidthPercent, float buttonHeightPercent, float buttonSpacingPercent, float verticalPositionPercent)
{
    std::vector<Rectangle> positions;
    for (int i = 0; i < numButtons; i++) {
        positions.push_back(CalculateButtonPosition(i, numButtons, buttonWidthPercent, buttonHeightPercent, buttonSpacingPercent, verticalPositionPercent));
    }
    return positions;
}

Rectangle ScreenLayout::CalculateReturnButtonPosition(
    float widthPercent, float heightPercent, float marginPercent)
{

    int screenWidth  = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    int buttonWidth  = GetScreenRelativeWidth(widthPercent);
    int buttonHeight = GetScreenRelativeHeight(heightPercent);
    int margin       = GetScreenRelativeWidth(marginPercent);

    // Position in top-left corner with specified margin
    return Rectangle{
        (float)margin,
        (float)margin,
        (float)buttonWidth,
        (float)buttonHeight};
}