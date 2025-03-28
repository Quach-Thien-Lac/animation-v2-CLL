#pragma once
#include "../INIT.hpp"
#include "../includes/Node.hpp"

class GraphNode : public PolyNode {
public:
    // Constants for force-directed layout
    static constexpr float REPULSE      = 300000;
    static constexpr float ATTRACT      = 0.2f;
    static constexpr float LENGTH_LIMIT = 150.0f;
    static constexpr float MIN_DISTANCE = 80.0f;

    // Boundaries for the graph layout
    static constexpr float MARGIN = 100.0f; // Margin from screen edges

public:
    explicit GraphNode(Font font);
    virtual ~GraphNode() = default;

    // Physics properties
    void setVelocity(Vector2 velocity);
    Vector2 getVelocity() const;

    // Node connections
    void makeAdjacent(GraphNode* node);
    bool isAdjacent(const GraphNode& node) const;
    const std::vector<GraphNode*>& getAdjacent() const;

    // Force calculations for graph layout
    Vector2 getRepulsion(const GraphNode& node) const;
    Vector2 getAttraction(const GraphNode& node) const;
    Vector2 getTotalAttraction() const;

    // Override update to include physics
    void update(float dt);

    // Set screen boundaries for all nodes
    static void setScreenBoundaries(float left, float right, float top, float bottom);

private:
    std::vector<GraphNode*> mAdjacent;
    Vector2 mVelocity;

    // Static boundaries that will be set from screen size
    static float sLeft;
    static float sRight;
    static float sTop;
    static float sBottom;
};
