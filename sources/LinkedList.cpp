// LinkedList.cpp
#include "../includes/LinkedList.hpp"
#include "../includes/Animation.hpp"
#include "../includes/Button.hpp"
#include "raylib.h"
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <string>
#include <vector>
#include <memory>
#include <queue>
using namespace std;

#ifndef PI
#define PI 3.14159265358979323846
#endif

namespace {
    AnimationList gAnimList;
    bool gPendingInsertion = false;
    int gPendingInsertValue = 0;
    bool gInsertionDone = false;
    int gInitialNodeCount = 0;
    std::vector<Vector2> gOldPositions;
    std::vector<Vector2> gNewPositions;
    std::vector<Vector2> gCurrentPositions;
    bool gIsAnimating = false;
    bool gSearchActive = false;
    int gSearchValue = 0;
    int gSearchIndex = 0;
    float gSearchTimer = 0.0f;
    const float gSearchThreshold = 0.5f;
    bool gSearchFound = false;
    float gSearchFoundTimer = 0.0f;
    const float gSearchFoundDuration = 1.0f;
    bool gSearchNotFound = false;
    float gSearchNotFoundTimer = 0.0f;
    const float gSearchNotFoundDuration = 1.0f;
    bool gDeleteActive = false;
    int gDeleteValue = 0;
    int gDeleteIndex = 0;
    float gDeleteTimer = 0.0f;
    const float gDeleteThreshold = 0.5f;
    bool gDeleteFound = false;
    bool gDeleteNotFound = false;
    float gDeleteStatusTimer = 0.0f;
    const float gDeleteStatusDuration = 1.0f;
    bool showInsertMenu = false;
    bool showSearchDialog = false;
    bool showDeleteDialog = false;
    std::string manualInputText = "";
    bool manualInputActive = false;
    std::string searchInputText = "";
    bool searchInputActive = false;
    std::string deleteInputText = "";
    bool deleteInputActive = false;
    bool showAddDestinationDialog = false;
    bool showAddValueDialog = false;
    std::string addDestinationText = "";
    std::string addValueText = "";
    bool addDestinationActive = false;
    bool addValueActive = false;
    int addDestinationValue = -1;
    bool showUpdateDestinationDialog = false;
    bool showUpdateNewValueDialog = false;
    std::string updateDestinationText = "";
    std::string updateNewValueText = "";
    bool updateDestinationActive = false;
    bool updateNewValueActive = false;
    int updateDestinationValue = -1;
    std::queue<std::function<void()>> pendingOps;
}

// Helper for text input dialogs
int GetValDialogBox(const char* caption, std::string& inputText, bool& inputActive, Rectangle rect) {
    int posX = (int)rect.x;
    int posY = (int)rect.y;
    int boxWidth = (int)rect.width;
    int boxHeight = (int)rect.height;

    DrawText(caption, posX, posY - 25, 20, BLACK);
    Rectangle inputBox = { (float)posX, (float)posY, (float)boxWidth, (float)boxHeight };

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 mousePos = GetMousePosition();
        inputActive = CheckCollisionPointRec(mousePos, inputBox);
    }

    Color boxColor = inputActive ? RAYWHITE : LIGHTGRAY;
    DrawRectangleRec(inputBox, boxColor);
    DrawRectangleLines(posX, posY, boxWidth, boxHeight, BLACK);
    DrawText(inputText.c_str(), posX + 5, posY + 5, 20, BLACK);

    // Blinking cursor
    if (inputActive && (((int)(GetTime() * 2) % 2) == 0)) {
        int textWidth = MeasureText(inputText.c_str(), 20);
        DrawText("|", posX + 5 + textWidth, posY + 5, 20, BLACK);
    }

    if (inputActive) {
        int key = GetCharPressed();
        while (key > 0) {
            if (key >= 32 && key <= 125 && inputText.length() < 10)
                inputText.push_back((char)key);
            key = GetCharPressed();
        }
        if (IsKeyPressed(KEY_BACKSPACE) && !inputText.empty())
            inputText.pop_back();
        if (IsKeyPressed(KEY_ENTER) && !inputText.empty()) {
            int value = std::atoi(inputText.c_str());
            inputText = "";
            return value;
        }
    }
    return -1;
}

// Create ActionButtons easily
std::unique_ptr<Button> DrawButton(const Rectangle& bounds,
    const char* label,
    std::function<void()> onClick,
    int fontSize)
{
    return std::make_unique<ActionButton>(
        bounds,
        label,
        fontSize,
        onClick,
        RAYWHITE,  // Normal color
        SKYBLUE,   // Hover color
        BLUE,      // Click color
        BLACK      // Text color
    );
}

// ------------------------------------------------------------------------
// Modified GetPosCLLNode from your code, but if totalNodes=1, place it at center
Vector2 LinkedList::GetPosCLLNode(int totalNodes, int index, float centerX, float centerY, float radius) {
    if (totalNodes == 1)
        return { centerX, centerY };
    else if (totalNodes == 2) {
        return (index == 0)
            ? Vector2{ centerX - radius / 2, centerY }
        : Vector2{ centerX + radius / 2, centerY };
    }
    else {
        float angle = (2 * PI * index) / totalNodes;
        return {
            centerX + radius * cosf(angle),
            centerY + radius * sinf(angle)
        };
    }
}

// ------------------------------------------------------------------------
// LinkedList Implementation
// ------------------------------------------------------------------------
LinkedList::LinkedList() : head(nullptr), camera(nullptr) { }

LinkedList::~LinkedList() {
    ClearList();
}

// We keep the same "head" unless the user deletes it
void LinkedList::DrawList(const std::vector<Vector2>& positions, int highlightIndex, Node* head) {
    const int   nodeRadius = 40;
    const float arrowHeadLength = 10.0f;
    const float arrowHeadAngle = PI / 6;
    int count = positions.size();

    camera->beginMode();

    // Draw lines
    if (count >= 2) {
        for (int i = 0; i < count; i++) {
            int nextIndex = (i + 1) % count;
            Vector2 start = positions[i];
            Vector2 end = positions[nextIndex];
            Vector2 dir = { end.x - start.x, end.y - start.y };
            float length = sqrtf(dir.x * dir.x + dir.y * dir.y);
            if (length != 0) {
                dir.x /= length;
                dir.y /= length;
            }
            Vector2 lineEnd = { end.x - dir.x * nodeRadius, end.y - dir.y * nodeRadius };
            DrawLineEx(start, lineEnd, 2, BLACK);
        }
    }

    // Draw nodes
    for (int i = 0; i < count; i++) {
        Color nodeColor = (i == highlightIndex) ? YELLOW : LIGHTGRAY;
        DrawCircleV(positions[i], (float)nodeRadius, nodeColor);

        if (head != nullptr) {
            // We always start enumerating from "head"
            Node* cur = head;
            for (int j = 0; j < i; j++)
                cur = cur->next;
            std::string text = std::to_string(cur->data);
            int textWidth = MeasureText(text.c_str(), 20);
            DrawText(text.c_str(),
                (int)(positions[i].x - textWidth / 2),
                (int)(positions[i].y - 10),
                20,
                BLACK);
        }
    }

    // Draw arrow heads
    if (count >= 2) {
        for (int i = 0; i < count; i++) {
            int nextIndex = (i + 1) % count;
            Vector2 start = positions[i];
            Vector2 end = positions[nextIndex];
            Vector2 dir = { end.x - start.x, end.y - start.y };
            float length = sqrtf(dir.x * dir.x + dir.y * dir.y);
            if (length != 0) {
                dir.x /= length;
                dir.y /= length;
            }
            Vector2 arrowPos = { end.x - dir.x * nodeRadius, end.y - dir.y * nodeRadius };
            float angle = atan2f(end.y - start.y, end.x - start.x);
            float angle1 = angle + arrowHeadAngle;
            float angle2 = angle - arrowHeadAngle;
            Vector2 arrowPoint1 = {
                arrowPos.x - arrowHeadLength * cosf(angle1),
                arrowPos.y - arrowHeadLength * sinf(angle1)
            };
            Vector2 arrowPoint2 = {
                arrowPos.x - arrowHeadLength * cosf(angle2),
                arrowPos.y - arrowHeadLength * sinf(angle2)
            };
            DrawTriangle(arrowPos, arrowPoint1, arrowPoint2, BLACK);
        }
    }

    camera->endMode();
}

void LinkedList::ClearList() {
    if (head == nullptr) return;
    Node* current = head;
    Node* nextNode;
    do {
        nextNode = current->next;
        delete current;
        current = nextNode;
    } while (current != head);
    head = nullptr;

    const int delayFrames = 30;
    for (int i = 0; i < delayFrames; i++) {
        ClearBackground(RAYWHITE);
        DrawText("List Cleared", 10, 10, 20, RED);
    }
}

/*
    Insert after the head so that 'head' doesn't change
    (unless list was empty).
*/
void LinkedList::DrawNode(int value) {
    if (gAnimList.isPlaying()) return;

    gPendingInsertion = true;
    gPendingInsertValue = value;

    // Count current nodes
    int count = 0;
    if (head != nullptr) {
        Node* temp = head;
        do {
            count++;
            temp = temp->next;
        } while (temp != head);
    }
    gInitialNodeCount = count;
    int totalNodes = count + 1;

    // We'll center in the middle
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    float centerX = screenWidth / 2.0f;
    float centerY = screenHeight / 2.0f;
    float radius = 250.0f;

    gOldPositions.clear();
    gNewPositions.clear();

    // old positions
    if (count > 0) {
        for (int i = 0; i < count; i++) {
            gOldPositions.push_back(GetPosCLLNode(count, i, centerX, centerY, radius));
        }
    }
    // new positions
    for (int i = 0; i < totalNodes; i++) {
        gNewPositions.push_back(GetPosCLLNode(totalNodes, i, centerX, centerY, radius));
    }

    // Animate from old to new
    Animation animInsertion(
        [=](float progress) {
            std::vector<Vector2> interpPositions;
            for (int i = 0; i < gInitialNodeCount; i++) {
                Vector2 interp = {
                    gOldPositions[i].x + (gNewPositions[i].x - gOldPositions[i].x) * progress,
                    gOldPositions[i].y + (gNewPositions[i].y - gOldPositions[i].y) * progress
                };
                interpPositions.push_back(interp);
            }
            Vector2 newNodePos = {
                centerX + (gNewPositions[gInitialNodeCount].x - centerX) * progress,
                centerY + (gNewPositions[gInitialNodeCount].y - centerY) * progress
            };
            interpPositions.push_back(newNodePos);
            gCurrentPositions = interpPositions;
            gIsAnimating = true;
        },
        1.0f
    );
    gAnimList.push(animInsertion);
    gAnimList.play();
    gInsertionDone = false;
}

// We only reassign 'head' if the list was empty
// or if we are deleting 'head' itself
void LinkedList::GetInputFromFile(const std::string& filename) {
    std::ifstream infile(filename);
    if (!infile.is_open()) {
        const int delayFrames = 60;
        for (int i = 0; i < delayFrames; i++) {
            BeginDrawing();
            ClearBackground(RAYWHITE);
            DrawText("Error: Unable to open file.", 100, 100, 20, RED);
            EndDrawing();
        }
        return;
    }
    int n;
    infile >> n;
    for (int i = 0; i < n; i++) {
        int value;
        infile >> value;
        // Queue each insertion
        pendingOps.push([this, value]() {
            DrawNode(value);
            });
    }
    infile.close();
}

void LinkedList::MakeRandomList() {
    srand((unsigned)time(0));
    const int numNodes = 10;
    for (int i = 0; i < numNodes; i++) {
        int randomValue = rand() % 100;
        pendingOps.push([this, randomValue]() {
            DrawNode(randomValue);
            });
    }
}

void LinkedList::StartSearchOperation(int searchValue) {
    if (head == nullptr) return;
    gSearchActive = true;
    gSearchValue = searchValue;
    gSearchIndex = 0;
    gSearchTimer = 0.0f;
    gSearchFound = false;
    gSearchFoundTimer = 0.0f;
    gSearchNotFound = false;
    gSearchNotFoundTimer = 0.0f;
}

void LinkedList::SearchNodeValue(int searchValue) {
    pendingOps.push([this, searchValue]() {
        StartSearchOperation(searchValue);
        });
}

void LinkedList::StartDeleteOperation(int deleteValue) {
    if (head == nullptr) return;
    gDeleteActive = true;
    gDeleteValue = deleteValue;
    gDeleteIndex = 0;
    gDeleteTimer = 0.0f;
    gDeleteFound = false;
    gDeleteNotFound = false;
    gDeleteStatusTimer = 0.0f;
}

void LinkedList::DeleteNode(int deleteValue) {
    pendingOps.push([this, deleteValue]() {
        StartDeleteOperation(deleteValue);
        });
}
void LinkedList::AddNode(int dest, int newVal) {
    if (head == nullptr)
        return;
    int index = -1;
    int i = 0;
    Node* cur = head;
    do {
        if (cur->data == dest) { index = i; break; }
        i++;
        cur = cur->next;
    } while (cur != head);
    if (index == -1)
        return;
    int count = 0;
    cur = head;
    vector<Node*> order;
    do {
        order.push_back(cur);
        count++;
        cur = cur->next;
    } while (cur != head);
    int newTotal = count + 1;
    float centerX = GetScreenWidth() / 2.0f;
    float centerY = GetScreenHeight() / 2.0f;
    float radius = 200.0f;
    gOldPositions.clear();
    gNewPositions.clear();
    for (int i = 0; i < count; i++) {
        gOldPositions.push_back(GetPosCLLNode(count, i, centerX, centerY, radius));
    }
    for (int i = 0; i < newTotal; i++) {
        gNewPositions.push_back(GetPosCLLNode(newTotal, i, centerX, centerY, radius));
    }
    Animation animAddition(
        [=](float progress) {
            vector<Vector2> interpPositions;
            for (int i = 0; i <= index; i++) {
                Vector2 interp = { gOldPositions[i].x + (gNewPositions[i].x - gOldPositions[i].x) * progress,
                    gOldPositions[i].y + (gNewPositions[i].y - gOldPositions[i].y) * progress };
                interpPositions.push_back(interp);
            }
            Vector2 newNodePos = gNewPositions[index + 1];
            interpPositions.push_back(newNodePos);
            for (int i = index + 1; i < count; i++) {
                Vector2 interp = { gOldPositions[i].x + (gNewPositions[i + 1].x - gOldPositions[i].x) * progress,
                    gOldPositions[i].y + (gNewPositions[i + 1].y - gOldPositions[i].y) * progress };
                interpPositions.push_back(interp);
            }
            gCurrentPositions = interpPositions;
            gIsAnimating = true;
        }, 1.0f);
    gAnimList.push(animAddition);
    gAnimList.play();
    pendingOps.push([=]() {
        Node* newNode = new Node{ newVal, nullptr };
        Node* target = head;
        for (int i = 0; i < index; i++) target = target->next;
        newNode->next = target->next;
        target->next = newNode;
        gIsAnimating = false;
        gCurrentPositions.clear();
        });
}
void LinkedList::UpdateNode(int dest, int newVal) {
    if (head == nullptr)
        return;
    int index = -1;
    int i = 0;
    Node* cur = head;
    do {
        if (cur->data == dest) { index = i; break; }
        i++;
        cur = cur->next;
    } while (cur != head);
    if (index == -1)
        return;
    pendingOps.push([=]() {
        Node* target = head;
        for (int i = 0; i < index; i++) target = target->next;
        target->data = newVal;
        });
}
// Scene lifecycle
void LinkedList::init() {
    buttons.clear();
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    int panelWidth = screenWidth / 3;
    int margin = 10;
    int panelX = margin;
    int panelY = screenHeight / 2 + margin;
    int panelAvailableHeight = screenHeight / 2 - 2 * margin;
    int buttonHeight = (panelAvailableHeight - 5 * margin) / 6;
    int buttonWidth = panelWidth - 2 * margin;
    auto initBtn = make_unique<ActionButton>(Rectangle{ (float)panelX, (float)panelY, (float)buttonWidth, (float)buttonHeight },
        "Initialize", baseFontSize, [this]() { ClearList(); MakeRandomList(); },
        RAYWHITE, SKYBLUE, BLUE, BLACK);
    auto addBtn = make_unique<ActionButton>(Rectangle{ (float)panelX, (float)(panelY + (buttonHeight + margin) * 1), (float)buttonWidth, (float)buttonHeight },
        "Add", baseFontSize, [this]() { showAddDestinationDialog = true; },
        RAYWHITE, SKYBLUE, BLUE, BLACK);
    auto updateBtn = make_unique<ActionButton>(Rectangle{ (float)panelX, (float)(panelY + (buttonHeight + margin) * 2), (float)buttonWidth, (float)buttonHeight },
        "Update", baseFontSize, [this]() { showUpdateDestinationDialog = true; },
        RAYWHITE, SKYBLUE, BLUE, BLACK);
    auto searchBtn = make_unique<ActionButton>(Rectangle{ (float)panelX, (float)(panelY + (buttonHeight + margin) * 3), (float)buttonWidth, (float)buttonHeight },
        "Search", baseFontSize, [this]() { showSearchDialog = true; },
        RAYWHITE, SKYBLUE, BLUE, BLACK);
    auto deleteBtn = make_unique<ActionButton>(Rectangle{ (float)panelX, (float)(panelY + (buttonHeight + margin) * 4), (float)buttonWidth, (float)buttonHeight },
        "Delete", baseFontSize, [this]() { showDeleteDialog = true; },
        RAYWHITE, SKYBLUE, BLUE, BLACK);
    auto clearBtn = make_unique<ActionButton>(Rectangle{ (float)panelX, (float)(panelY + (buttonHeight + margin) * 5), (float)buttonWidth, (float)buttonHeight },
        "Clear List", baseFontSize, [this]() { ClearList(); },
        RAYWHITE, SKYBLUE, BLUE, BLACK);
    buttons.push_back(move(initBtn));
    buttons.push_back(move(addBtn));
    buttons.push_back(move(updateBtn));
    buttons.push_back(move(searchBtn));
    buttons.push_back(move(deleteBtn));
    buttons.push_back(move(clearBtn));
    addComponent<ReturnButtonComponent>(Scene::MENU, baseFontSize)->init();
    camera = addComponent<Camera2DComponent>();
    camera->init();
    head = nullptr;
    gAnimList.clear();
    gPendingInsertion = false;
    while (!pendingOps.empty()) { pendingOps.pop(); }
}

void LinkedList::update() {
    updateComponents();
    if (IsWindowResized()) { updateFontSize(); handleComponentsResize(); }
    for (auto& button : buttons) { button->update(); }
    camera->update();
    gAnimList.update(GetFrameTime());
    if (gPendingInsertion && !gAnimList.isPlaying() && !gInsertionDone) {
        Node* newNode = new Node{ gPendingInsertValue, nullptr };
        if (head == nullptr) { head = newNode; newNode->next = head; }
        else { Node* last = head; while (last->next != head) last = last->next; last->next = newNode; newNode->next = head; }
        gInsertionDone = true;
        gPendingInsertion = false;
        gIsAnimating = false;
        gCurrentPositions.clear();
    }
    if (!gAnimList.isPlaying() && !pendingOps.empty()) { auto op = pendingOps.front(); pendingOps.pop(); op(); }
    if (gSearchActive) {
        float dt = GetFrameTime();
        int count = 0;
        if (head != nullptr) { Node* temp = head; do { count++; temp = temp->next; } while (temp != head); }
        if (count > 0) {
            if (gSearchIndex < count) {
                gSearchTimer += dt;
                if (gSearchTimer >= gSearchThreshold) {
                    Node* cur = head;
                    for (int j = 0; j < gSearchIndex; j++) cur = cur->next;
                    if (cur->data == gSearchValue) { gSearchFound = true; }
                    else { gSearchIndex++; gSearchTimer = 0.0f; }
                }
            }
            else { gSearchNotFound = true; }
            if (gSearchFound) { gSearchFoundTimer += dt; if (gSearchFoundTimer >= gSearchFoundDuration) gSearchActive = false; }
            if (gSearchNotFound) { gSearchNotFoundTimer += dt; if (gSearchNotFoundTimer >= gSearchNotFoundDuration) gSearchActive = false; }
        }
    }
    if (gDeleteActive) {
        float dt = GetFrameTime();
        int count = 0;
        if (head != nullptr) { Node* temp = head; do { count++; temp = temp->next; } while (temp != head); }
        if (count > 0) {
            if (gDeleteIndex < count) {
                gDeleteTimer += dt;
                if (gDeleteTimer >= gDeleteThreshold) {
                    Node* cur = head;
                    for (int j = 0; j < gDeleteIndex; j++) cur = cur->next;
                    if (cur->data == gDeleteValue) {
                        if (head->next == head) { delete head; head = nullptr; }
                        else {
                            if (cur == head) { Node* last = head; while (last->next != head) last = last->next; head = head->next; last->next = head; delete cur; }
                            else { Node* prev = head; for (int j = 0; j < gDeleteIndex - 1; j++) prev = prev->next; prev->next = cur->next; delete cur; }
                        }
                        gDeleteFound = true;
                    }
                    else { gDeleteIndex++; gDeleteTimer = 0.0f; }
                }
            }
            else { gDeleteNotFound = true; }
            gDeleteStatusTimer += dt;
            if (gDeleteStatusTimer >= gDeleteStatusDuration) { gDeleteActive = false; gDeleteStatusTimer = 0.0f; }
        }
    }
    int panelY = GetScreenHeight() / 2 + 10;
    int dialogY = panelY - 40;
    Rectangle dialogRect = { 10, (float)dialogY, 150, 30 };
    if (showAddDestinationDialog) {
        int val = GetValDialogBox("Destination:", addDestinationText, addDestinationActive, dialogRect);
        if (val != -1) { addDestinationValue = val; showAddDestinationDialog = false; showAddValueDialog = true; }
    }
    if (showAddValueDialog) {
        int val = GetValDialogBox("New Value:", addValueText, addValueActive, dialogRect);
        if (val != -1) { AddNode(addDestinationValue, val); showAddValueDialog = false; }
    }
    if (showUpdateDestinationDialog) {
        int val = GetValDialogBox("Update Target:", updateDestinationText, updateDestinationActive, dialogRect);
        if (val != -1) { updateDestinationValue = val; showUpdateDestinationDialog = false; showUpdateNewValueDialog = true; }
    }
    if (showUpdateNewValueDialog) {
        int val = GetValDialogBox("New Value:", updateNewValueText, updateNewValueActive, dialogRect);
        if (val != -1) { UpdateNode(updateDestinationValue, val); showUpdateNewValueDialog = false; }
    }
    if (showSearchDialog) {
        int val = GetValDialogBox("Search:", searchInputText, searchInputActive, dialogRect);
        if (val != -1) { SearchNodeValue(val); showSearchDialog = false; }
    }
    if (showDeleteDialog) {
        int val = GetValDialogBox("Delete:", deleteInputText, deleteInputActive, dialogRect);
        if (val != -1) { DeleteNode(val); showDeleteDialog = false; }
    }
}
void LinkedList::draw() {
    ClearBackground(RAYWHITE);
    float centerX = GetScreenWidth() / 2.0f;
    float centerY = GetScreenHeight() / 2.0f;
    float radius = 200.0f;
    vector<Vector2> positions;
    int count = 0;
    if (head != nullptr) {
        Node* temp = head;
        do {
            count++;
            positions.push_back(GetPosCLLNode(count, positions.size(), centerX, centerY, radius));
            temp = temp->next;
        } while (temp != head);
    }
    if (gSearchActive) { DrawList(positions, gSearchIndex, head); }
    else if (gDeleteActive) { DrawList(positions, gDeleteIndex, head); }
    else if (gIsAnimating && !gCurrentPositions.empty()) { DrawList(gCurrentPositions, -1, head); }
    else { DrawList(positions, -1, head); }
    if (gSearchActive) {
        if (gSearchFound) { DrawText("Node Found!", 10, GetScreenHeight() / 2 - 40, 20, GREEN); }
        else if (gSearchNotFound) { DrawText("Node Not Found!", 10, GetScreenHeight() / 2 - 40, 20, RED); }
        else { DrawText(TextFormat("Searching for %d...", gSearchValue), 10, GetScreenHeight() / 2 - 40, 20, BLACK); }
    }
    if (gDeleteActive) {
        if (gDeleteFound) { DrawText("Node Deleted!", 10, GetScreenHeight() / 2 - 40, 20, GREEN); }
        else if (gDeleteNotFound) { DrawText("Node Not Found!", 10, GetScreenHeight() / 2 - 40, 20, RED); }
        else { DrawText(TextFormat("Deleting %d...", gDeleteValue), 10, GetScreenHeight() / 2 - 40, 20, BLACK); }
    }
    DrawText("Linked List Visualization", 10, 10, 20, BLACK);
    for (auto& button : buttons) { button->draw(); }
    drawComponents();
}
void LinkedList::clean() {
    buttons.clear();
    cleanComponents();
    ClearList();
}
