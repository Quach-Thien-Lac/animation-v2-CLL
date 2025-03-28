#include "../includes/Edge.hpp"
#include "../includes/Node.hpp"

Edge::Edge(PolyNode* from, PolyNode* to, int type)
    : mFrom(from),
      mTo(to),
      mType(type),
      mWeight(0),
      mColor(BLACK),
      mHighlightColor(RED),
      mThickness(DEFAULT_THICKNESS),
      mHighlighted(false),
      mLabel(""),
      mLabelOffset({0, 0}),
      mDestination({0, 0}),
      mTargetDestination({0, 0}),
      mTargetType(TargetType::Left),
      mIsCircular(false),
      mAnimationSpeed(5.0f)
{
    // If the edge connects to a real node, calculate the destination point
    if (to) {
        mDestination       = calculateTargetPoint(to, mTargetType);
        mTargetDestination = mDestination;
    }

    // Set circular flag if the type includes the Circular flag
    if (type & EdgeType::Circular) {
        setCircular(true);
    }
}

// Edge appearance methods
void Edge::setColor(Color color)
{
    mColor = color;
}

void Edge::setHighlight(bool highlight)
{
    mHighlighted = highlight;
}

void Edge::setType(int type)
{
    mType = type;

    // Update circular flag
    mIsCircular = (type & EdgeType::Circular) != 0;
}

void Edge::setWeight(int weight)
{
    mWeight = weight;

    // If weight is set, make sure the type includes Weighted flag
    if (weight != 0) {
        mType |= EdgeType::Weighted;
    }
}

void Edge::setThickness(float thickness)
{
    mThickness = thickness;
}

// Pointer-specific methods
void Edge::setLabel(const std::string& label)
{
    mLabel = label;
}

void Edge::setLabelOffset(float x, float y)
{
    mLabelOffset.x = x;
    mLabelOffset.y = y;
}

void Edge::setCircular(bool circular)
{
    mIsCircular = circular;

    if (circular) {
        mType |= EdgeType::Circular;
    }
    else {
        mType &= ~EdgeType::Circular;
    }
}

void Edge::setTarget(PolyNode* node, TargetType type)
{
    mTo         = node;
    mTargetType = type;

    if (node) {
        mTargetDestination = calculateTargetPoint(node, type);
    }
}

void Edge::resetDestination()
{
    if (mTo) {
        mTargetDestination = calculateTargetPoint(mTo, mTargetType);
    }
}

void Edge::setTargetPosition(Vector2 position)
{
    mTargetDestination = position;
}

// Getters
PolyNode* Edge::getTo() const
{
    return mTo;
}

PolyNode* Edge::getFrom() const
{
    return mFrom;
}

Vector2 Edge::getDestination() const
{
    return mDestination;
}

// Update and draw
void Edge::update(float dt)
{
    // Smoothly animate destination changes
    if (Vector2Distance(mDestination, mTargetDestination) > 1.0f) {
        mDestination.x = Lerp(mDestination.x, mTargetDestination.x, dt * mAnimationSpeed);
        mDestination.y = Lerp(mDestination.y, mTargetDestination.y, dt * mAnimationSpeed);
    }
    else {
        mDestination = mTargetDestination;
    }

    // Update destination point if target node moved
    if (mTo) {
        mTargetDestination = calculateTargetPoint(mTo, mTargetType);
    }
}

void Edge::draw()
{
    // Skip drawing if hidden
    if (mType & EdgeType::Hidden) {
        return;
    }

    // Get source position (always from the source node)
    Vector2 start = {0, 0};
    if (mFrom) {
        start = mFrom->getPosition();
    }

    // Draw the appropriate edge type
    if (mIsCircular) {
        drawCircularArrow(start, mDestination);
    }
    else {
        drawLine(start, mDestination);

        // Add arrow if directed
        if (mType & EdgeType::Directed) {
            drawArrow(start, mDestination);
        }
    }

    // Draw weight label if weighted
    if (mType & EdgeType::Weighted) {
        drawWeightLabel();
    }

    // Draw edge label if it exists
    if (!mLabel.empty()) {
        drawPointerLabel();
    }
}

// Drawing helper methods
void Edge::drawLine(Vector2 start, Vector2 end)
{
    Color lineColor = mHighlighted ? mHighlightColor : mColor;
    DrawLineEx(start, end, mThickness, lineColor);
}

void Edge::drawArrow(Vector2 start, Vector2 end)
{
    // Calculate direction vector
    float dx     = end.x - start.x;
    float dy     = end.y - start.y;
    float length = sqrtf(dx * dx + dy * dy);

    // Normalize
    if (length > 0) {
        dx /= length;
        dy /= length;
    }

    // Calculate perpendicular vectors for arrow
    float perpX = -dy;
    float perpY = dx;

    // Arrow points
    Vector2 arrowLeft = {
        end.x - dx * ARROW_SIZE - perpX * ARROW_SIZE * 0.5f,
        end.y - dy * ARROW_SIZE - perpY * ARROW_SIZE * 0.5f};

    Vector2 arrowRight = {
        end.x - dx * ARROW_SIZE + perpX * ARROW_SIZE * 0.5f,
        end.y - dy * ARROW_SIZE + perpY * ARROW_SIZE * 0.5f};

    // Draw the arrow
    Color arrowColor = mHighlighted ? mHighlightColor : mColor;
    DrawTriangle(end, arrowLeft, arrowRight, arrowColor);
}

void Edge::drawCircularArrow(Vector2 start, Vector2 end)
{
    if (!mFrom)
        return;

    float radius      = mFrom->getRadius();
    Color circleColor = mHighlighted ? mHighlightColor : mColor;

    // Calculate center position for the circular arrow
    Vector2 center = {
        start.x + CIRCULAR_OFFSET,
        start.y + CIRCULAR_OFFSET};

    // Draw a semi-circle
    float startAngle = 0.0f;
    float endAngle   = 270.0f * DEG2RAD;
    DrawCircleSectorLines(center, radius * 0.8f, startAngle, endAngle, 20, circleColor);

    // Draw arrow at the end of the semi-circle
    Vector2 arrowEnd = {
        center.x + radius * 0.8f * cosf(endAngle),
        center.y + radius * 0.8f * sinf(endAngle)};

    Vector2 arrowDir = {
        cosf(endAngle + PI / 4),
        sinf(endAngle + PI / 4)};

    Vector2 arrowPoint1 = {
        arrowEnd.x + arrowDir.x * ARROW_SIZE,
        arrowEnd.y + arrowDir.y * ARROW_SIZE};

    Vector2 arrowPoint2 = {
        arrowEnd.x + cosf(endAngle - PI / 4) * ARROW_SIZE,
        arrowEnd.y + sinf(endAngle - PI / 4) * ARROW_SIZE};

    DrawTriangle(arrowEnd, arrowPoint1, arrowPoint2, circleColor);
}

void Edge::drawWeightLabel()
{
    if (!mFrom)
        return;

    Vector2 start = mFrom->getPosition();

    // Calculate midpoint for the weight label
    Vector2 mid = {
        (start.x + mDestination.x) * 0.5f,
        (start.y + mDestination.y) * 0.5f};

    // Draw weight text
    std::string weightText = std::to_string(mWeight);
    Color textColor        = mHighlighted ? mHighlightColor : BLACK;

    // Draw with a background for better visibility
    float fontSize   = 20.0f;
    Vector2 textSize = MeasureTextEx(GetFontDefault(), weightText.c_str(), fontSize, 1);
    DrawRectangle(mid.x - textSize.x / 2 - 3, mid.y - textSize.y / 2 - 3,
                  textSize.x + 6, textSize.y + 6, WHITE);
    DrawText(weightText.c_str(), mid.x - textSize.x / 2, mid.y - textSize.y / 2, fontSize, textColor);
}

void Edge::drawPointerLabel()
{
    if (!mFrom)
        return;

    Vector2 start = mFrom->getPosition();

    // Calculate position for the label
    Vector2 labelPos = {
        (start.x + mDestination.x) * 0.5f + mLabelOffset.x,
        (start.y + mDestination.y) * 0.5f + mLabelOffset.y};

    // Draw label text
    Color textColor  = mHighlighted ? mHighlightColor : BLACK;
    float fontSize   = 18.0f;
    Vector2 textSize = MeasureTextEx(GetFontDefault(), mLabel.c_str(), fontSize, 1);

    // Draw with a background for better visibility
    DrawRectangle(labelPos.x - textSize.x / 2 - 3, labelPos.y - textSize.y / 2 - 3,
                  textSize.x + 6, textSize.y + 6, WHITE);
    DrawText(mLabel.c_str(), labelPos.x - textSize.x / 2, labelPos.y - textSize.y / 2, fontSize, textColor);
}

// Calculation helpers
Vector2 Edge::calculateTargetPoint(PolyNode* node, TargetType type)
{
    if (!node || !mFrom)
        return {0, 0};

    Vector2 fromPos = mFrom->getPosition();
    Vector2 toPos   = node->getPosition();
    float toRadius  = node->getRadius();

    // Calculate direction vector
    float dx     = toPos.x - fromPos.x;
    float dy     = toPos.y - fromPos.y;
    float length = sqrtf(dx * dx + dy * dy);

    // Normalize
    if (length > 0) {
        dx /= length;
        dy /= length;
    }

    Vector2 result = toPos;

    // Adjust based on target type
    switch (type) {
        case Left:
            result.x -= toRadius;
            break;
        case Right:
            result.x += toRadius;
            break;
        case Top:
            result.y -= toRadius;
            break;
        case Bottom:
            result.y += toRadius;
            break;
        default:
            // Otherwise, calculate intersection with circle
            result.x = toPos.x - dx * toRadius;
            result.y = toPos.y - dy * toRadius;
            break;
    }

    return result;
}