#include "../includes/GraphNode.hpp"

// Initialize static boundaries
float GraphNode::sLeft   = MARGIN;
float GraphNode::sRight  = 800.0f - MARGIN; // Default fallback
float GraphNode::sTop    = MARGIN;
float GraphNode::sBottom = 600.0f - MARGIN; // Default fallback

GraphNode::GraphNode(Font font)
    : PolyNode(font), mVelocity({0, 0})
{
    // Initialize the node with zero velocity
}

void GraphNode::setVelocity(Vector2 velocity)
{
    mVelocity = velocity;
}

Vector2 GraphNode::getVelocity() const
{
    return mVelocity;
}

void GraphNode::makeAdjacent(GraphNode* node)
{
    // Skip if already adjacent
    if (isAdjacent(*node))
        return;

    // Add to adjacent list
    mAdjacent.push_back(node);

    // Create visual edge
    addEdgeOut(node);
}

bool GraphNode::isAdjacent(const GraphNode& node) const
{
    return std::find(mAdjacent.begin(), mAdjacent.end(), &node) != mAdjacent.end();
}

const std::vector<GraphNode*>& GraphNode::getAdjacent() const
{
    return mAdjacent;
}

Vector2 GraphNode::getRepulsion(const GraphNode& node) const
{
    // Calculate distance vector between nodes
    Vector2 distance = Vector2Subtract(getPosition(), node.getPosition());
    float magnitude  = Vector2Length(distance);

    // Avoid division by zero
    if (magnitude < 1.0f)
        magnitude = 1.0f;

    float repulseFactor = REPULSE;
    if (magnitude < MIN_DISTANCE) {
        repulseFactor *= (MIN_DISTANCE / magnitude) * 2.0f; // Extra strong repulsion when too close
    }

    // Normalize and apply repulsion force inversely proportional to distance
    return Vector2Scale(Vector2Normalize(distance), REPULSE / (magnitude * magnitude));
}

Vector2 GraphNode::getAttraction(const GraphNode& node) const
{
    // Calculate distance vector between nodes
    Vector2 distance = Vector2Subtract(node.getPosition(), getPosition());
    float magnitude  = Vector2Length(distance);

    // Calculate force proportional to distance beyond the ideal length
    float force = 0.0f;
    if (magnitude > LENGTH_LIMIT)
        force = (magnitude - LENGTH_LIMIT) * ATTRACT;

    // Apply force in the direction of the other node
    return Vector2Scale(Vector2Normalize(distance), force);
}

Vector2 GraphNode::getTotalAttraction() const
{
    Vector2 total = {0, 0};

    // Sum attractions from all adjacent nodes
    for (auto node : mAdjacent) {
        total = Vector2Add(total, getAttraction(*node));
    }

    return total;
}

void GraphNode::update(float dt)
{
    // Call parent update to handle animations
    PolyNode::update(dt);

    // Apply velocity to position
    Vector2 newPosition = Vector2Add(getPosition(), Vector2Scale(mVelocity, dt));

    // Apply boundaries using static screen-aware values
    if (newPosition.x < sLeft)
        newPosition.x = sLeft;
    if (newPosition.x > sRight)
        newPosition.x = sRight;
    if (newPosition.y < sTop)
        newPosition.y = sTop;
    if (newPosition.y > sBottom)
        newPosition.y = sBottom;

    setPosition(newPosition);

    // Apply damping to velocity (optional)
    mVelocity = Vector2Scale(mVelocity, 0.95f);
}

void GraphNode::setScreenBoundaries(float left, float right, float top, float bottom)
{
    sLeft   = left;
    sRight  = right;
    sTop    = top;
    sBottom = bottom;
}
