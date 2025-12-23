#include "ShaderGraph.h"
#include <imgui.h>
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "node/imgui_node_editor.h"
#include <map>

using namespace std;
namespace ed = ax::NodeEditor;

using NodeID = int;
using PinID = int;
using LinkID = int;

enum PinType { Float, Vec2, Vec3, Vec4, Sampler2D };

struct Pin {
	PinID id;
	string name;
	PinType type;
	string defaultValue;
	bool isOutput = false;
};

struct Node {
	NodeID id;
	string type; // "Add", "Multiply", "TextureSample", "PBRMaterial"
	string title;
	glm::vec2 position{ 100, 100 };
	vector<Pin> inputs;
	vector<Pin> outputs;
	map<string, string> properties;
};

struct Link {
	LinkID id;
	NodeID fromNode;
	PinID fromPin;
	NodeID toNode;
	PinID toPin;
};

struct Graph {
	vector<Node> nodes;
	vector<Link> links;
	NodeID nextNodeId = 1;
	PinID nextPinId = 1;
	LinkID nextLinkId = 1;

	NodeID AddNode(const string& type, const string& title, const ImVec2& pos) {
		Node node;
		node.id = nextNodeId++;
		node.type = type;
		node.title = title;
		node.position = glm::vec2(pos.x, pos.y);
		nodes.push_back(std::move(node));
		return node.id;
	}

	Node* GetNode(NodeID id) {
		for (auto& n : nodes) if (n.id == id) return &n;
		return nullptr;
	}

	PinID AddInput(NodeID nodeId, const string& name, PinType t, const string& def = "0.0") {
		Node* n = GetNode(nodeId);
		if (!n) return -1;
		Pin p;
		p.id = nextPinId++;
		p.name = name;
		p.type = t;
		p.defaultValue = def;
		n->inputs.push_back(p);
		return p.id;
	}

	PinID AddOutput(NodeID nodeId, const string& name, PinType t) {
		Node* n = GetNode(nodeId);
		if (!n) return -1;
		Pin p;
		p.id = nextPinId++;
		p.name = name;
		p.type = t;
		n->outputs.push_back(p);
		return p.id;
	}

	LinkID AddLink(PinID fromPinId, PinID toPinId) {
		Pin* from = nullptr;
		Pin* to = nullptr;
		Node* fromNode = nullptr;
		Node* toNode = nullptr;

		for (auto& n : nodes) {
			for (auto& p : n.outputs) if (p.id == fromPinId) { from = &p; fromNode = &n; }
			for (auto& p : n.inputs) if (p.id == toPinId) { to = &p; toNode = &n; }
		}

		if (!from || !to || !fromNode || !toNode) return -1;
		if (from->type != to->type) return -1;
		if (!from->isOutput || to->isOutput) return -1;

		Link l;
		l.id = nextLinkId++;
		l.fromPin = fromPinId;
		l.toPin = toPinId;
		l.fromNode = fromNode->id;
		l.toNode = toNode->id;
		links.push_back(l);
		return l.id;
	}

	void Clear() {
		nodes.clear(); links.clear();
		nextNodeId = nextPinId = nextLinkId = 1;
	}
};

struct CodeSnippet {
	string declarations; // var decls and helper functions
	string code; // lines that compute outputs
	map<PinID, std::string> outputVarNames; // pinIndex -> varName
};

static std::string PinTypeToGLSL(PinType t) {
	switch (t) {
	case PinType::Float: return "float";
	case PinType::Vec2: return "vec2";
	case PinType::Vec3: return "vec3";
	case PinType::Vec4: return "vec4";
	case PinType::Sampler2D: return "sampler2D";
	}
	return "float";
}

// Build adjacency and do topological sort
static bool TopologicalSort(const Graph& graph, std::vector<NodeID>& outOrder, string& err) {
	// Build map nodeId -> dependencies count (in-degree)
	unordered_map<NodeID, int> indeg;
	unordered_map<NodeID, std::vector<NodeID>> adjs;
	for (auto& n : graph.nodes) { indeg[n.id] = 0; }
	for (auto& l : graph.links) {
		// link: fromNode -> toNode
		adjs[l.fromNode].push_back(l.toNode);
		indeg[l.toNode]++;
	}
	// Kahn
	vector<NodeID> q;
	for (auto& kv : indeg) if (kv.second == 0) q.push_back(kv.first);
	while (!q.empty()) {
		NodeID n = q.back();
		q.pop_back();
		outOrder.push_back(n);
		for (auto nb : adjs[n]) {
			indeg[nb]--;
			if (indeg[nb] == 0) q.push_back(nb);
		}
	}
	if (outOrder.size() != graph.nodes.size()) {
		err = "Cycle detected or disconnected nodes exist.";
		return false;
	}
	return true;
}

Pin* FindPinById(Graph& graph, PinID id) {
	for (auto& n : graph.nodes) {
		for (auto& p : n.inputs)
			if (p.id == id) return &p;
		for (auto& p : n.outputs)
			if (p.id == id) return &p;
	}
	return nullptr;
}

ed::EditorContext* ctx = nullptr;
Graph g_graph;
Moon::Editor::ShaderGraph::ShaderGraph(const string& p_name, bool p_opened, const PanelWindowSettings& p_windowSettings) :
	PanelWindow(p_name, p_opened, p_windowSettings)
{
	ed::Config config;
	ctx = ed::CreateEditor(&config);

	// For destructor
	//ed::DestroyEditor(ctx);
}

void Moon::Editor::ShaderGraph::_Draw_Impl() {
    if (!ctx) {
        ImGui::TextColored(ImVec4(1, 0.2f, 0.2f, 1), "Node Editor context is null!");
        return;
    }

    ed::SetCurrentEditor(ctx);
    ed::Begin("Shader Graph Editor");

    Graph& graph = g_graph;

	ed::BeginNode(1);
	ImGui::Text("Input Node");
	ed::BeginPin(2, ed::PinKind::Output);
	ImGui::Text("Out");
	ed::EndPin();
	ed::EndNode();

	// Node 2
	ed::BeginNode(3);
	ImGui::Text("Output Node");
	ed::BeginPin(4, ed::PinKind::Input);
	ImGui::Text("In");
	ed::EndPin();
	ed::EndNode();

	// Draw link between nodes
	ed::Link(5, 2, 4); // (link_id, from_pin_id, to_pin_id)

    ImGui::End();
	ed::SetCurrentEditor(nullptr);
}
