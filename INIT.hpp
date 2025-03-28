#pragma once
#include "raylib.h"
#include "raymath.h"

#include <algorithm>
#include <cmath>
#include <ctime>
#include <functional>
#include <fstream>
#include <iostream>
#include <memory>
#include <queue>
#include <vector>
#include <string>
#include <sstream>

enum class Scene {
    TITLE = 0,
    MENU,
    HASHTABLE,
    LINKEDLIST,
    AVLTREE,
    GRAPH,
};