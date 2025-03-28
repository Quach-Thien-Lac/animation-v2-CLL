#pragma once
#include "../INIT.hpp"

class ScreenLayout {
public:
    // Get dimensions relative to screen size
    static int GetScreenRelativeWidth(float percentage);
    static int GetScreenRelativeHeight(float percentage);
    static int GetScreenRelativeSpacing(float percentage);

    // Calculate horizontal layout for buttons
    static Rectangle CalculateButtonPosition(int buttonIndex, int totalButtons,
float buttonWidthPercent = 0.2f, float buttonHeightPercent = 0.2f,float buttonSpacingPercent = 0.01f, float verticalPositionPercent = 0.5f);

    // Calculate multiple button positions in a horizontal row
    static std::vector<Rectangle> CalculateButtonGrid(int numButtons, float buttonWidthPercent = 0.2f, float buttonHeightPercent = 0.2f, float buttonSpacingPercent = 0.01f, float verticalPositionPercent = 0.5f);

    // Calculate position for a return button
    static Rectangle CalculateReturnButtonPosition(
        float widthPercent  = 0.15f, // Default: 15% of screen width
        float heightPercent = 0.08f, // Default: 8% of screen height
        float marginPercent = 0.02f);
};

