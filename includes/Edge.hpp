#pragma once
#include "../INIT.hpp"

class PolyNode;

// Edge types (bit flags for flexibility)
enum EdgeType {
    Hidden   = 1 << 0,
    Directed = 1 << 1,
    Weighted = 1 << 2,
    Circular = 1 << 3
};

// Target attachment positions
enum TargetType {
    Left,
    Right,
    Bottom,
    Top
};

class Edge {

public:
    Edge(PolyNode* from, PolyNode* to, int type = EdgeType::Directed);

    // Edge appearance methods
    void setColor(Color color);
    void setHighlight(bool highlight);
    void setType(int type);
    void setWeight(int weight);
    void setThickness(float thickness);

    // Pointer-specific methods
    void setLabel(const std::string& label);
    void setLabelOffset(float x, float y);
    void setCircular(bool circular);
    void setTarget(PolyNode* node, TargetType type = Left);
    void resetDestination();
    void setTargetPosition(Vector2 position);

    // Getters
    PolyNode* getTo() const;
    PolyNode* getFrom() const;
    Vector2 getDestination() const;

    // Update and draw
    void update(float dt);
    void draw();

private:
    // Drawing helper methods
    void drawLine(Vector2 start, Vector2 end);
    void drawArrow(Vector2 start, Vector2 end);
    void drawCircularArrow(Vector2 start, Vector2 end);
    void drawWeightLabel();
    void drawPointerLabel();

    // Calculation helpers
    Vector2 calculateTargetPoint(PolyNode* node, TargetType type);

private:
    // Core properties
    PolyNode *mFrom, *mTo;
    int mType;
    int mWeight;

    // Visual properties
    Color mColor;
    Color mHighlightColor;
    float mThickness;
    bool mHighlighted;

    // Pointer-specific properties
    std::string mLabel;
    Vector2 mLabelOffset;
    Vector2 mDestination;
    Vector2 mTargetDestination;
    TargetType mTargetType;
    bool mIsCircular;

    // Animation properties
    float mAnimationSpeed;

    // Constants
    static constexpr float ARROW_SIZE        = 10.0f;
    static constexpr float CIRCULAR_OFFSET   = -60.0f;
    static constexpr float DEFAULT_THICKNESS = 2.0f;
};