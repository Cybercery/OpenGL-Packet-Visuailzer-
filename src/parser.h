#pragma once
#include "graph.h"
#include <vector>
#include <string>

struct ParsedCapture {
    std::vector<Node> nodes;
    std::vector<Edge> edges;
    std::vector<PacketEvent> events;
    float duration; // total capture length in seconds
};

ParsedCapture parseCapture(const std::string& path);