#include "../includes/UI.hpp"
#include "../includes/Graph.hpp"

Graph::Graph()
    : SceneManager(),
      mMaxForce(0),
      mCoolDown(COOL_DOWN),
      mTime(0),
      mIsDirected(true),
      mIsWeighted(false),
      camera(nullptr)
{
    // Seed the random number generator
    SetRandomSeed(static_cast<unsigned int>(time(nullptr)));
}

void Graph::init()
{
    buttons.clear();
    addComponent<ReturnButtonComponent>(Scene::MENU, baseFontSize)->init();
    camera = addComponent<Camera2DComponent>();
    camera->init();

    // Set GraphNode boundaries based on screen size
    // Leave margins around the edges
    float margin = GraphNode::MARGIN;
    GraphNode::setScreenBoundaries(
        margin,                    // left
        GetScreenWidth() - margin, // right
        margin,                    // top
        GetScreenHeight() - margin // bottom
    );

    // Create Add Node button
    int buttonWidth  = 120;
    int buttonHeight = 40;
    int padding      = 10;
    int buttonY      = GetScreenHeight() - buttonHeight - padding;

    // Add Node button
    auto addNodeBtn = std::make_unique<ActionButton>(
        Rectangle{static_cast<float>(GetScreenWidth() - buttonWidth * 2 - padding * 2),
                  static_cast<float>(buttonY),
                  static_cast<float>(buttonWidth),
                  static_cast<float>(buttonHeight)},
        "Add Node",
        baseFontSize,
        [this]() {
            // Add a new node to the graph
            int newNodeIndex = getNumNodes();

            // If we haven't reached the maximum number of nodes
            if (newNodeIndex < MAX_SIZE) {
                // First build a new node
                Font defaultFont = GetFontDefault();
                auto node        = std::make_unique<GraphNode>(defaultFont);
                node->setData(std::to_string(newNodeIndex));

                // Position it near the center with a small random offset
                Vector2 center = {GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f};
                float offsetX  = static_cast<float>(GetRandomValue(-GetScreenWidth() / 4, GetScreenWidth() / 4));
                float offsetY  = static_cast<float>(GetRandomValue(-GetScreenHeight() / 4, GetScreenHeight() / 4));
                node->setPosition(center.x + offsetX, center.y + offsetY);

                // Add the node to the graph
                mNodes.push_back(std::move(node));

                // Reset layout process to reposition nodes
                arrangeNodes();
            }
        },
        LIGHTGRAY, // normal color
        GRAY,      // hover color
        DARKGRAY,  // click color
        BLACK      // text color
    );

    // Remove Node button
    auto removeNodeBtn = std::make_unique<ActionButton>(
        Rectangle{static_cast<float>(GetScreenWidth() - buttonWidth - padding),
                  static_cast<float>(buttonY),
                  static_cast<float>(buttonWidth),
                  static_cast<float>(buttonHeight)},
        "Remove Node",
        baseFontSize,
        [this]() {
            // Remove the last node if we have any
            if (!mNodes.empty()) {
                // First, remove all edges connected to this node
                int nodeToRemove = getNumNodes() - 1;

                // Remove all edges that involve this node
                mEdges.erase(
                    std::remove_if(mEdges.begin(), mEdges.end(),
                                   [nodeToRemove](const EdgeTuple& edge) {
                                       return edge.from == nodeToRemove || edge.to == nodeToRemove;
                                   }),
                    mEdges.end());

                // Remove the node
                mNodes.pop_back();

                // Reset layout process
                arrangeNodes();
            }
        },
        LIGHTGRAY, // normal color
        GRAY,      // hover color
        DARKGRAY,  // click color
        BLACK      // text color
    );

    // Add Edge button
    auto addEdgeBtn = std::make_unique<ActionButton>(
        Rectangle{static_cast<float>(GetScreenWidth() - buttonWidth * 3 - padding * 3),
                  static_cast<float>(buttonY),
                  static_cast<float>(buttonWidth),
                  static_cast<float>(buttonHeight)},
        "Add Edge",
        baseFontSize,
        [this]() {
            // Only add edge if we have at least 2 nodes
            int numNodes = getNumNodes();
            if (numNodes >= 2) {
                // Get two random nodes to connect
                int from = GetRandomValue(0, numNodes - 1);
                int to   = GetRandomValue(0, numNodes - 1);

                // Make sure they're different nodes
                while (from == to) {
                    to = GetRandomValue(0, numNodes - 1);
                }

                // Add edge between the nodes
                int weight = mIsWeighted ? GetRandomValue(1, MAX_WEIGHT) : 1;
                addEdge(from, to, weight);
            }
        },
        LIGHTGRAY, // normal color
        GRAY,      // hover color
        DARKGRAY,  // click color
        BLACK      // text color
    );

    buttons.push_back(std::move(addNodeBtn));
    buttons.push_back(std::move(removeNodeBtn));
    buttons.push_back(std::move(addEdgeBtn));
}

void Graph::update()
{
    float dt = GetFrameTime();

    updateComponents();

    if (IsWindowResized()) {
        updateFontSize();
        handleComponentsResize();

        // Update node boundaries when the window is resized
        float margin = GraphNode::MARGIN;
        GraphNode::setScreenBoundaries(
            margin,
            GetScreenWidth() - margin,
            margin,
            GetScreenHeight() - margin);
    }

    for (auto& button : buttons) {
        button->update();
    }

    // Force-directed layout update
    rearrange();

    // Update nodes
    for (auto& node : mNodes) {
        node->update(dt);
    }
}

void Graph::draw()
{
    // Begin camera mode for graph rendering
    camera->beginMode();
    // First, draw all edges
    for (auto& node : mNodes) {
        // We need to iterate through each node's outgoing edges
        for (const auto& edge : node->getOutEdges()) {
            // Update and draw the edge
            edge->update(GetFrameTime());
            edge->draw();
        }
    }

    // Draw graph elements
    for (auto& node : mNodes) {
        node->draw();
    }
    camera->endMode();
    // Draw other information
    std::string infoText = TextFormat("Nodes: %d, Edges: %d", getNumNodes(), getNumEdges());
    DrawText(infoText.c_str(), 10, GetScreenHeight() - 30, 20, BLACK);

    DrawText("This is Graph", 300, 300, 20, BLACK);

    for (auto& button : buttons) {
        button->draw();
    }
    drawComponents();
}

void Graph::clean()
{
    clear();
    buttons.clear();
    cleanComponents();
}

void Graph::clear()
{
    mNodes.clear();
    mEdges.clear();
    mTime = 0;
}

void Graph::clearHighlight()
{
    for (auto& node : mNodes) {
        node->highlight(PolyNode::Highlight::None);
        node->clearEdgeHighlights();
    }
}

void Graph::loadFromFile(const std::string& fileDir)
{
    std::ifstream file(fileDir);
    if (!file.is_open()) {
        return;
    }

    clear();

    int n, m;
    file >> n >> m;

    // Build nodes
    build(n);

    // Add edges
    for (int i = 0; i < m; i++) {
        int from, to, weight = 1;
        file >> from >> to;

        if (mIsWeighted) {
            file >> weight;
        }

        addEdge(from, to, weight);
    }

    file.close();
    arrangeNodes();
}

void Graph::randomize(int nodes, int edges)
{
    clear();

    // Create nodes
    build(nodes);

    // Create edges
    std::vector<EdgeTuple> allPossibleEdges;
    for (int i = 0; i < nodes; i++) {
        for (int j = 0; j < nodes; j++) {
            if (i != j) {
                int weight = GetRandomValue(1, MAX_WEIGHT);
                allPossibleEdges.emplace_back(i, j, weight);
            }
        }
    }

    // Shuffle and select random edges
    int edgeCount = std::min(edges, static_cast<int>(allPossibleEdges.size()));

    // Shuffle algorithm
    for (size_t i = 0; i < allPossibleEdges.size() - 1; i++) {
        size_t j = i + GetRandomValue(0, static_cast<int>(allPossibleEdges.size() - i - 1));
        std::swap(allPossibleEdges[i], allPossibleEdges[j]);
    }

    // Add selected edges
    for (int i = 0; i < edgeCount; i++) {
        addEdge(allPossibleEdges[i].from, allPossibleEdges[i].to, allPossibleEdges[i].weight);
    }

    arrangeNodes();
}

void Graph::build(int nodes)
{
    // Create nodes with proper positioning
    float radius   = std::min(GetScreenWidth(), GetScreenHeight()) * 0.4f; // Use 40% of screen
    Vector2 center = {GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f};

    Font defaultFont = GetFontDefault();

    for (int i = 0; i < nodes; i++) {
        auto node = std::make_unique<GraphNode>(defaultFont);

        // Set node data
        node->setData(std::to_string(i));

        // Circular layout initially
        float angle = (float)i / nodes * 2 * PI;
        float x     = center.x + radius * cosf(angle);
        float y     = center.y + radius * sinf(angle);
        node->setPosition(x, y);

        mNodes.push_back(std::move(node));
    }
}

void Graph::addEdge(int from, int to, int weight)
{
    // Validate indices
    if (from < 0 || from >= getNumNodes() || to < 0 || to >= getNumNodes()) {
        return;
    }

    // Check if edge already exists
    auto edgeIt = std::find_if(mEdges.begin(), mEdges.end(),
                               [from, to](const EdgeTuple& edge) {
                                   return edge.from == from && edge.to == to;
                               });

    if (edgeIt != mEdges.end()) {
        // Update weight if edge exists
        edgeIt->weight = weight;

        // Update visual edge weight
        if (mIsWeighted) {
            mNodes[from]->setEdgeWeight(mNodes[to].get(), weight);
        }
        return;
    }

    // Add new edge
    mEdges.emplace_back(from, to, weight);

    // Create visual connection
    mNodes[from]->makeAdjacent(mNodes[to].get());

    // Set edge properties
    int edgeType = mIsDirected ? EdgeType::Directed : 0;
    if (mIsWeighted) {
        edgeType |= EdgeType::Weighted;
        mNodes[from]->setEdgeWeight(mNodes[to].get(), weight);
    }

    mNodes[from]->setEdgeType(mNodes[to].get(), edgeType);

    // If undirected, add reverse edge too
    if (!mIsDirected) {
        // Only add if it doesn't exist
        auto reverseEdgeIt = std::find_if(mEdges.begin(), mEdges.end(),
                                          [from, to](const EdgeTuple& edge) {
                                              return edge.from == to && edge.to == from;
                                          });

        if (reverseEdgeIt == mEdges.end()) {
            mEdges.emplace_back(to, from, weight);
            mNodes[to]->makeAdjacent(mNodes[from].get());

            if (mIsWeighted) {
                mNodes[to]->setEdgeWeight(mNodes[from].get(), weight);
            }

            mNodes[to]->setEdgeType(mNodes[from].get(), edgeType);
        }
    }
}

void Graph::removeEdge(int from, int to)
{
    // Validate indices
    if (from < 0 || from >= getNumNodes() || to < 0 || to >= getNumNodes()) {
        return;
    }

    // Find and remove the edge from the list
    auto edgeIt = std::find_if(mEdges.begin(), mEdges.end(),
                               [from, to](const EdgeTuple& edge) {
                                   return edge.from == from && edge.to == to;
                               });

    if (edgeIt != mEdges.end()) {
        mEdges.erase(edgeIt);

        // Remove the visual connection
        mNodes[from]->removeEdgeOut(mNodes[to].get());
    }

    // If undirected, remove the reverse edge too
    if (!mIsDirected) {
        auto reverseEdgeIt = std::find_if(mEdges.begin(), mEdges.end(),
                                          [from, to](const EdgeTuple& edge) {
                                              return edge.from == to && edge.to == from;
                                          });

        if (reverseEdgeIt != mEdges.end()) {
            mEdges.erase(reverseEdgeIt);

            // Remove the visual connection
            mNodes[to]->removeEdgeOut(mNodes[from].get());
        }
    }
}

void Graph::setDirected(bool isDirected)
{
    if (mIsDirected == isDirected)
        return;

    mIsDirected = isDirected;

    // Recreate edges with new direction setting
    std::vector<EdgeTuple> oldEdges = mEdges;
    clear();

    // Rebuild nodes
    build(oldEdges.empty() ? 0 : getNumNodes());

    // Rebuild edges
    for (const auto& edge : oldEdges) {
        addEdge(edge.from, edge.to, edge.weight);
    }
}

void Graph::setWeighted(bool isWeighted)
{
    if (mIsWeighted == isWeighted)
        return;

    mIsWeighted = isWeighted;

    // Update edge type for all edges
    for (auto& edge : mEdges) {
        int from = edge.from;
        int to   = edge.to;

        int edgeType = mIsDirected ? EdgeType::Directed : 0;
        if (mIsWeighted) {
            edgeType |= EdgeType::Weighted;
            mNodes[from]->setEdgeWeight(mNodes[to].get(), edge.weight);
        }

        mNodes[from]->setEdgeType(mNodes[to].get(), edgeType);
    }
}

void Graph::rearrange()
{
    // Don't rearrange if there are no nodes or we've reached the iteration limit
    if (mNodes.size() <= 1 || mTime >= UPDATE_LOOPS) {
        return;
    }

    mTime++;
    mMaxForce = 0;

    // Calculate repulsive and attractive forces for each node
    for (size_t i = 0; i < mNodes.size(); i++) {
        Vector2 totalForce = {0, 0};

        // Apply repulsive forces from all other nodes
        for (size_t j = 0; j < mNodes.size(); j++) {
            if (i != j) {
                Vector2 force = mNodes[i]->getRepulsion(*mNodes[j]);
                totalForce    = Vector2Add(totalForce, force);
            }
        }

        // Apply attractive forces from adjacent nodes
        totalForce = Vector2Add(totalForce, mNodes[i]->getTotalAttraction());

        // Scale force by cooldown factor
        totalForce = Vector2Scale(totalForce, mCoolDown);

        // Update maximum force
        float forceMag = Vector2Length(totalForce);
        mMaxForce      = std::max(mMaxForce, forceMag);

        // Apply force to node
        mNodes[i]->setVelocity(totalForce);
    }

    // Reduce cooldown for next iteration
    mCoolDown *= 0.98f;

    // Stop if forces become very small
    if (mMaxForce < FORCE_EPSILON) {
        mTime = UPDATE_LOOPS;
    }
}

void Graph::arrangeNodes()
{
    // Reset the layout process
    mTime     = 0;
    mCoolDown = COOL_DOWN;

    // Give nodes some initial velocity to help them spread out
    for (auto& node : mNodes) {
        Vector2 randomVel = {
            static_cast<float>(GetRandomValue(-50, 50)),
            static_cast<float>(GetRandomValue(-50, 50))};
        node->setVelocity(randomVel);
    }
}

int Graph::getNumNodes() const
{
    return static_cast<int>(mNodes.size());
}

int Graph::getNumEdges() const
{
    return static_cast<int>(mEdges.size());
}