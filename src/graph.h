#pragma once
#include <string>
#include <glm\glm.hpp>

enum class Protocol {
    TCP, UDP, ICMP, ARP, DNS, HTTP, HTTPS, Unknown
};

struct Node {
    int id;
    std::string ip;
    glm::vec2 position;
    int totalPackets;
    int totalBytes;
};

struct Edge {
    int id;
    int src, dst;       // node ids
    int packetCount;
    int bytesTransferred;
    float activity;     // 0-1, decays over time
};

struct PacketEvent {
    int id;
    int src, dst;       // node ids
    float timestamp;    // seconds, relative to capture start
    Protocol protocol;
    int bytes;
};

struct AnimatedPacket {
    int eventID;
    float progress;
};
