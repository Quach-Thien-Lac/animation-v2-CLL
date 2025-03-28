#pragma once
#include "../INIT.hpp"
#include "../includes/Button.hpp"
#include "../includes/UI.hpp"
#include "../includes/Animation.hpp"
#include "../includes/GraphNode.hpp"

class Graph : public SceneManager {
public:
    // Constants
    static constexpr int MAX_SIZE   = 10;
    static constexpr int MAX_EDGES  = 50;
    static constexpr int MAX_WEIGHT = 100;

    // Force-directed graph layout constants
    static constexpr int UPDATE_LOOPS    = 100;
    static constexpr float FORCE_EPSILON = 0.01f;
    static constexpr float COOL_DOWN     = 0.95f;

    // Edge representation
    struct EdgeTuple {
        int from, to, weight;

        EdgeTuple(int from = 0, int to = 0, int weight = 0)
            : from(from), to(to), weight(weight) {}

        bool operator<(const EdgeTuple& edge) const
        {
            if (from != edge.from)
                return from < edge.from;
            return to < edge.to;
        }
    };

public:
    Graph();
    virtual ~Graph() = default;

    // SceneManager implementation
    void init() override;
    void update() override;
    void draw() override;
    void clean() override;

    // Graph management
    void clear();
    void clearHighlight();
    void loadFromFile(const std::string& fileDir);
    void randomize(int nodes, int edges);
    void build(int nodes);
    void addEdge(int from, int to, int weight = 1);
    void removeEdge(int from, int to);

    // Graph properties
    void setDirected(bool isDirected);
    void setWeighted(bool isWeighted);

    // Algorithms with animation
    //std::vector<Animation> CCAnimation();  // Connected Components
    //std::vector<Animation> MSTAnimation(); // Minimum Spanning Tree
    //std::vector<Animation> DijkstraAnimation(int start);

    // Getters
    int getNumNodes() const;
    int getNumEdges() const;

    private:
    // Helper methods
    void rearrange();
    //void DFS(const GraphNode* node, std::vector<int>& components);
    void arrangeNodes();

private:
    float mMaxForce;
    float mCoolDown;
    int mTime;

    bool mIsDirected;
    bool mIsWeighted;

    std::vector<std::unique_ptr<GraphNode>> mNodes;
    std::vector<EdgeTuple> mEdges;

    Camera2DComponent* camera;
    std::vector<std::unique_ptr<Button>> buttons;
};