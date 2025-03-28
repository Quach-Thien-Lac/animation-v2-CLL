#pragma once
#include "../INIT.hpp"
#include "../includes/UI.hpp"

class AVLTree : public SceneManager {
public:
    void init() override;
    void update() override;
    void draw() override;
    void clean() override;
};