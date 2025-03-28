// LinkedList.hpp
#ifndef LINKEDLIST_HPP
#define LINKEDLIST_HPP

#include "UI.hpp"
#include "raylib.h"
#include <string>

struct Node {
    int data;
    Node* next;
};

class LinkedList : public SceneManager {
public:
    LinkedList();
    virtual ~LinkedList();

    void init() override;
    void update() override;
    void draw() override;
    void clean() override;
    void DrawList(const std::vector<Vector2>& positions, int highlightIndex, Node* head);
    void DrawNode(int value);
    void GetInputFromFile(const std::string& filename);
    void MakeRandomList();
    void SearchNodeValue(int searchValue);
    void DeleteNode(int deleteValue);
    void AddNode(int dest, int newVal);
    void UpdateNode(int dest, int newVal);

    Vector2 GetPosCLLNode(int totalNodes, int index, float centerX, float centerY, float radius);

private:
    void StartSearchOperation(int searchValue);
    void StartDeleteOperation(int deleteValue);
    void ClearList();
    std::vector<std::unique_ptr<Button>> buttons;
    Camera2DComponent* camera;
    Node* head;
};

#endif // LINKEDLIST_HPP
