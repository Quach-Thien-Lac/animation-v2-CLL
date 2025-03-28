#include "../includes/Node.hpp"

PolyNode::PolyNode(Font font)
    : mFont(font),
      mData(""),
      mLabel(""),
      mPosition({0, 0}),
      mRadius(30.0f),
      mScale(1.0f),
      mTargetScale(1.0f),
      mFontSize(20.0f),
      mBaseColor(WHITE),
      mCurrentColor(WHITE),
      mTextColor(BLACK),
      mHighlightPrimaryColor(RED),
      mHighlightSecondaryColor(BLUE)
{
}

PolyNode::~PolyNode()
{
    removeAllEdges();
}

// Data management
std::string PolyNode::getData() const
{
    return mData;
}

int PolyNode::getIntData() const
{
    try {
        return std::stoi(mData);
    }
    catch (const std::exception&) {
        return 0;
    }
}

float PolyNode::getRadius() const
{
    return mRadius * mScale;
}

void PolyNode::setData(const std::string& data)
{
    mData = data;
    // Set animation scale effect for data change
    mTargetScale = 1.2f;
}

void PolyNode::setData(int data)
{
    setData(std::to_string(data));
}

void PolyNode::swapData(PolyNode* node)
{
    if (node) {
        std::string temp = mData;
        mData            = node->mData;
        node->mData      = temp;

        // Set animation scale effect for both nodes
        mTargetScale       = 1.2f;
        node->mTargetScale = 1.2f;
    }
}

void PolyNode::setLabel(const std::string& label)
{
    mLabel = label;
}

void PolyNode::setLabel(int label)
{
    mLabel = std::to_string(label);
}

void PolyNode::setPoint(int points)
{
    // placeholder, implementation depends on requirements
}

void PolyNode::highlight(Highlight type)
{
    switch (type) {
        case Highlight::None:
            mCurrentColor = mBaseColor;
            break;
        case Highlight::Primary:
            mCurrentColor = mHighlightPrimaryColor;
            break;
        case Highlight::Secondary:
            mCurrentColor = mHighlightSecondaryColor;
            break;
    }
}

void PolyNode::setRadius(float radius)
{
    mRadius = radius;
}

void PolyNode::resetDataScale()
{
    mTargetScale = 1.0f;
}

// Edge management
void PolyNode::addEdgeOut(PolyNode* to, int type)
{
    if (!to)
        return;

    // Check if edge already exists
    for (auto& edge : outEdges) {
        if (edge->getTo() == to) {
            return; // Edge already exists
        }
    }

    // Create new edge
    std::shared_ptr<Edge> newEdge = std::make_shared<Edge>(this, to, type);
    outEdges.push_back(newEdge);
    to->addEdgeIn(newEdge);
}

void PolyNode::addEdgeIn(std::shared_ptr<Edge> edge)
{
    if (edge) {
        inEdges.push_back(edge);
    }
}

void PolyNode::removeEdgeOut(PolyNode* to)
{
    if (!to)
        return;

    auto it = std::find_if(outEdges.begin(), outEdges.end(),
                           [to](const std::shared_ptr<Edge>& edge) { return edge->getTo() == to; });

    if (it != outEdges.end()) {
        // Remove from destination's inEdges
        PolyNode* toNode = (*it)->getTo();
        if (toNode) {
            toNode->removeEdgeIn(*it);
        }

        // Remove from our outEdges
        outEdges.erase(it);
    }
}

void PolyNode::removeEdgeIn(std::shared_ptr<Edge> edge)
{
    auto it = std::find(inEdges.begin(), inEdges.end(), edge);
    if (it != inEdges.end()) {
        inEdges.erase(it);
    }
}

void PolyNode::removeAllEdges()
{
    // Clear outgoing edges first (which will also remove corresponding incoming edges)
    while (!outEdges.empty()) {
        removeEdgeOut(outEdges[0]->getTo());
    }

    // Clear any remaining incoming edges (shouldn't be necessary if all nodes follow this pattern)
    inEdges.clear();
}

void PolyNode::highlightEdge(PolyNode* to, bool highlight)
{
    auto it = std::find_if(outEdges.begin(), outEdges.end(),
                           [to](const std::shared_ptr<Edge>& edge) { return edge->getTo() == to; });

    if (it != outEdges.end()) {
        (*it)->setHighlight(highlight);
    }
}

void PolyNode::setEdgeWeight(PolyNode* to, int weight)
{
    auto it = std::find_if(outEdges.begin(), outEdges.end(),
                           [to](const std::shared_ptr<Edge>& edge) { return edge->getTo() == to; });

    if (it != outEdges.end()) {
        (*it)->setWeight(weight);
    }
}

void PolyNode::setEdgeType(PolyNode* to, int type)
{
    auto it = std::find_if(outEdges.begin(), outEdges.end(),
                           [to](const std::shared_ptr<Edge>& edge) { return edge->getTo() == to; });

    if (it != outEdges.end()) {
        (*it)->setType(type);
    }
}

void PolyNode::clearEdgeHighlights()
{
    for (auto& edge : outEdges) {
        edge->setHighlight(false);
    }
}

// Position management
void PolyNode::setPosition(float x, float y)
{
    mPosition.x = x;
    mPosition.y = y;
}

void PolyNode::setPosition(Vector2 position)
{
    mPosition = position;
}

Vector2 PolyNode::getPosition() const
{
    return mPosition;
}

// Update and draw
void PolyNode::update(float dt)
{
    // Smooth animation for scale changes
    const float SCALE_SPEED = 5.0f;
    if (mScale != mTargetScale) {
        float direction = (mTargetScale > mScale) ? 1.0f : -1.0f;
        mScale += direction * SCALE_SPEED * dt;

        // Check if we've reached or overshot the target
        if ((direction > 0 && mScale >= mTargetScale) || (direction < 0 && mScale <= mTargetScale)) {
            mScale = mTargetScale;
        }
    }
}

void PolyNode::draw()
{
    renderNodeShape();
    renderText();
}

// Helper methods
void PolyNode::renderNodeShape()
{
    float scaledRadius = mRadius * mScale;
    DrawCircleV(mPosition, scaledRadius, mCurrentColor);
    DrawCircleLines(mPosition.x, mPosition.y, scaledRadius, BLACK);
}

void PolyNode::renderText()
{
    // Render data text in the center of the node
    if (!mData.empty()) {
        Vector2 textSize = MeasureTextEx(mFont, mData.c_str(), mFontSize, 1);
        Vector2 textPos  = {
            mPosition.x - textSize.x / 2,
            mPosition.y - textSize.y / 2};
        DrawTextEx(mFont, mData.c_str(), textPos, mFontSize, 1, mTextColor);
    }

    // Render label text below the node if it exists
    if (!mLabel.empty()) {
        Vector2 labelSize = MeasureTextEx(mFont, mLabel.c_str(), mFontSize * 0.8f, 1);
        Vector2 labelPos  = {
            mPosition.x - labelSize.x / 2,
            mPosition.y + mRadius * mScale + 5};
        DrawTextEx(mFont, mLabel.c_str(), labelPos, mFontSize * 0.8f, 1, mTextColor);
    }
}