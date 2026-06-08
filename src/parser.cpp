#include "parser.h"
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <algorithm>
#include "json.hpp"

using json = nlohmann::json;

// helper functions

static Protocol protocolFromString(const std::string& protocols)
{
	if (protocols.find("tcp") != std::string::npos)
		return Protocol::TCP;
	if (protocols.find("udp") != std::string::npos)
		return Protocol::UDP;
	if (protocols.find("icmp") != std::string::npos)
		return Protocol::ICMP;
	if (protocols.find("arp") != std::string::npos)
		return Protocol::ARP;
	if (protocols.find("dns") != std::string::npos)
		return Protocol::DNS;
	if (protocols.find("http") != std::string::npos)
		return Protocol::HTTP;
	if (protocols.find("https") != std::string::npos)
		return Protocol::HTTPS;
	return Protocol::Unknown;
}

static int getOrCreateNode(const std::string& ip, std::vector<Node>& nodes, std::unordered_map<std::string, int>& ipToId)
{
	auto it = ipToId.find(ip);
	if (it != ipToId.end())
		return it->second;

	int id = (int)nodes.size();
	Node n;
	n.id = id;
	n.ip = ip;
	n.position = glm::vec2(0.0f); // do it LATEr
	n.totalPackets = 0;
	n.totalBytes = 0;
	nodes.push_back(n);
	ipToId[ip] = id;
	return id;
}
static int getOrCreateEdge(int src, int dst, std::vector<Edge>& edges, std::unordered_map<uint64_t, int>& edgeMap)
{
	// storing smaller id first to avoid duplicates (A->B and B->A)
	int a = std::min(src, dst);
	int b = std::max(src, dst);
	uint64_t key = ((uint64_t)a << 32) | (uint64_t)b;
	auto it = edgeMap.find(key);

	if (it != edgeMap.end())
		return it->second;

	// create
	int id = (int)edges.size();
	Edge e;
	e.id = id;
	e.src = a;
	e.dst = b;
	e.packetCount = 0;
	e.bytesTransferred = 0;
	e.activity = 0.0f;
	edges.push_back(e);
	edgeMap[key] = id;
	return id;
}
// making a circle hopefully
static void applyCircleLayout(std::vector<Node>& nodes)
{
	int n = (int)nodes.size();
	if (n == 0) return;
	for (int i = 0; i < n; i++)
	{
		float angle = 2.0f * 3.14159265f * i / (float)n;
		nodes[i].position = glm::vec2(std::cos(angle) * 0.75f, std::sin(angle) * 0.75f);
	}
}
// parsingfunction
ParsedCapture parseCapture(const std::string& path)
{
	ParsedCapture result;

	// errors for plebs (i was forced to type this)
	std::ifstream file(path);
	if (!file.is_open())
	{
		std::cerr << "failed to open capture file: " << path << "\n";
		return result;
	}
	json j;
	try {
		file >> j;
	}
	catch (const std::exception& e) {
		std::cerr << "failed json parsing: " << e.what() << "\n";
		return result;
	}

	if (!j.is_array())
	{
		std::cerr << "expected json array of packets\n";
		return result;
	}

	std::unordered_map<std::string, int> ipToId;
	std::unordered_map<uint64_t, int> edgeMap;

	float firstTimestamp = -1.0f;
	float lastTimestamp = 0.0f;
	int eventId = 0;

	for (auto& entry : j) // took some help from GPT for this part, not gonna lie
	{
		// must have _source.layers
		if (!entry.contains("_source"))
			continue;
		auto& layers = entry["_source"]["layers"];

		//must have frame
		if (!layers.contains("frame"))
			continue;
		auto& frame = layers["frame"];

		float timeRelative = std::stof(frame.value("frame.time_relative", "0"));
		int frameLen = std::stoi(frame.value("frame.len", "0"));
		std::string protocols = frame.value("frame.protocols", "");

		if (firstTimestamp < 0) firstTimestamp = timeRelative;
		lastTimestamp = timeRelative;

		// ip adress
		std::string srcIP, dstIP;

		if (layers.contains("ip"))
		{
			srcIP = layers["ip"].value("ip.src", "");
			dstIP = layers["ip"].value("ip.dst", "");
		}
		else if (layers.contains("ipv6"))
		{
			srcIP = layers["ipv6"].value("ipv6.src", "");
			dstIP = layers["ipv6"].value("ipv6.dst", "");
		}
		else
		{
			continue; // skip non-ip packets
		}
		if (srcIP.empty() || dstIP.empty())
			continue;

			// nodes
			int srcId = getOrCreateNode(srcIP, result.nodes, ipToId);
			int dstId = getOrCreateNode(dstIP, result.nodes, ipToId);

			result.nodes[srcId].totalPackets++;
			result.nodes[srcId].totalBytes += frameLen;
			result.nodes[dstId].totalPackets++;
			result.nodes[dstId].totalBytes += frameLen;

			// deges
			int edgeId = getOrCreateEdge(srcId, dstId, result.edges, edgeMap);
			result.edges[edgeId].packetCount++;
			result.edges[edgeId].bytesTransferred += frameLen;

			// packet event
			PacketEvent ev;
			ev.id = eventId++;
			ev.src = srcId;
			ev.dst = dstId;
			ev.timestamp = timeRelative - firstTimestamp;
			ev.protocol = protocolFromString(protocols);
			ev.bytes = frameLen;
			result.events.push_back(ev);
	}

	// sort events by timestamp just in case
	std::sort(result.events.begin(), result.events.end(), [](const PacketEvent& a, const PacketEvent& b) {
		return a.timestamp < b.timestamp;
		});

	result.duration = lastTimestamp - firstTimestamp;

	// assign circle layout
	applyCircleLayout(result.nodes);

	// print
	std::cout << "[parser] loaded: " << path << "\n";
	std::cout << "packets: " << result.events.size() << "\n";
	std::cout << "nodes: " << result.nodes.size() << "\n";
	std::cout << "edges: " << result.edges.size() << "\n";
	std::cout << "duration: " << result.duration << "s\n";

	return result;
}
