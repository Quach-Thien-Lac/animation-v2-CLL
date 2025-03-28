#pragma once
#include "../INIT.hpp"
#include "../includes/Edge.hpp"

class PolyNode {
public:
    enum Highlight {
        None,
        Primary,
        Secondary,
    };

public:
    explicit PolyNode(Font font);
    ~PolyNode();

    // Data management
    std::string getData() const;
    int getIntData() const;
    float getRadius() const;

    void setData(const std::string& data);
    void setData(int data);
    void swapData(PolyNode* node);
    void setLabel(const std::string& label);
    void setLabel(int label);
    void setPoint(int points);
    void highlight(Highlight type);
    void setRadius(float radius);
    void resetDataScale();

    // Edge management
    void addEdgeOut(PolyNode* to, int type = Directed);
    void addEdgeIn(std::shared_ptr<Edge> edge);
    void removeEdgeOut(PolyNode* to);
    void removeEdgeIn(std::shared_ptr<Edge> edge);
    void removeAllEdges();
    void highlightEdge(PolyNode* to, bool highlight = true);
    void setEdgeWeight(PolyNode* to, int weight);
    void setEdgeType(PolyNode* to, int type);
    void clearEdgeHighlights();

    // Position management
    void setPosition(float x, float y);
    void setPosition(Vector2 position);
    Vector2 getPosition() const;

    // Update and draw
    void update(float dt);
    void draw();

    const std::vector<std::shared_ptr<Edge>>& getOutEdges() const { return outEdges; }

private:
    // Helper methods
    void renderNodeShape();
    void renderText();

private:
    // Node properties
    std::string mData;
    std::string mLabel;
    Vector2 mPosition;
    float mRadius;

    // Visual properties
    Color mBaseColor;
    Color mCurrentColor;
    Color mTextColor;
    Color mHighlightPrimaryColor;
    Color mHighlightSecondaryColor;
    Font mFont;
    float mFontSize;

    // Edges
    std::vector<std::shared_ptr<Edge>> inEdges;
    std::vector<std::shared_ptr<Edge>> outEdges;

    // Animation properties
    float mScale;
    float mTargetScale;
};