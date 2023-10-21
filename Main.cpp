#pragma once
#include <vector>
#include <atomic>
#include <iostream>
#include <thread>
#include <queue>
#include <algorithm>
#include <windows.h>

static std::size_t max_capacity = 10;
static std::atomic<int> is_created = 0;
const std::size_t NO_DISTANCE = static_cast<std::size_t>(-1);

//Definition for Edge Data Type
struct Node {
	std::pair<std::size_t, std::atomic<std::size_t>> edge;
	std::atomic<Node*> Next;
	Node(std::size_t number, std::size_t label);
};

//definition for LockFreeLinkedList
class LockFreeLinkedList {
private:

public:
	std::atomic<Node*> head;
	std::atomic<Node*> tail;
	//size of the LinkedList
	std::atomic<size_t> size;

	LockFreeLinkedList();

	//add Node before the tail of the LinkedList
	void add_Node(std::size_t i, std::size_t j);
};

//definition for LockFreeGraph
class LockFreeGraph
{
private:

public:
	//current Number of Nodes
	std::atomic<std::size_t> current_capacity = 0;

	//graph data structure
	std::atomic<LockFreeLinkedList*> adjacency_list;

	// Constructor 
	LockFreeGraph();
	//1. create graph and set the given max_capacity
	void create_graph(std::size_t max_capacity);
	//2. create a new vertex if it is not reach the maximum capcity
	void add_vertex();
	//3. add directed edge(i,j) to the graph, if already exists, then set the lebel to 0.
	void add_edge(std::size_t i, std::size_t j);
	//4. increase the label of the edge
	void inc_label(std::size_t i, std::size_t j, std::size_t increment);
	//5. decrease the label of the edge
	void dec_label(std::size_t i, std::size_t j, std::size_t increment);
	//6. check if two vertex are connected, return true if connected��
	//otherwise return false;
	bool are_connected(std::size_t i, std::size_t j);
	//7. check if two vertex are reachable, return true if reachable, else return false
	bool is_reachable(std::size_t i, std::size_t j);
	//8. return the length of shortest path given two vertex, return -1 if the path does not exist
	std::size_t shortest_path(std::size_t i, std::size_t j);
	//10.return the length of diameter of the graph
	std::size_t get_diameter();

	//check the validation of vertices
	bool is_vertex_valid(std::size_t i, std::size_t j);
	//check the validation of graph
	bool is_graph_created();
	void delete_edge(std::size_t i, std::size_t j) {
		if (!is_graph_created()) {
			std::cout << "Graph hasn't been created yet!" << std::endl;
			return;
		}
		if (!is_vertex_valid(i, j)) {
			return;
		}

		Node* cur = adjacency_list.load()[i].head.load();
		Node* prev = cur;
		bool edgeFound = false;

		while (cur) {
			if (cur->edge.first == j) {
				if (prev == cur) {
					if (adjacency_list.load()[i].head.compare_exchange_weak(cur, cur->Next.load())) {
						delete cur;
						edgeFound = true;
					}
				}
				else {
					if (prev->Next.compare_exchange_weak(cur, cur->Next.load())) {
						delete cur;
						edgeFound = true;
					}
				}
			}
			if (edgeFound) {
				break;
			}
			prev = cur;
			cur = cur->Next.load();
		}

		if (edgeFound) {
			std::cout << "Edge (" << i << ", " << j << ") has been deleted successfully." << std::endl;
		}
		else {
			std::cout << "delete_edge: The edge (" << i << ", " << j << ") does not exist!" << std::endl;
		}
	}

	void display_edges(const std::size_t i) {
		Node* cur = adjacency_list.load()[i].head.load();
		std::cout << "Edges for vertex " << i << ": ";
		while (cur) {
			std::cout << "(" << i << ", " << cur->edge.first << ", " << cur->edge.second.load() << ") "<<std::endl;
			cur = cur->Next.load();
		}
		std::cout << std::endl;
	}

};

//*********************************
//Node structure functions
Node::Node(const std::size_t number, const std::size_t label) {
	this->edge.first = number;
	this->edge.second = label;
	this->Next.store(nullptr);
}

//*********************************
//Linked List structure functions
LockFreeLinkedList::LockFreeLinkedList() {
	Node* dummy_node_head = new Node(0, 0);
	Node* dummy_node_tail = new Node(0, 0);
	this->head.store(dummy_node_head);
	this->tail.store(dummy_node_tail);
	this->head.load()->Next.store(tail);
}

void LockFreeLinkedList::add_Node(const std::size_t i, const std::size_t j) {
	Node* new_node = new Node(j, 0);
	Node* cur = this->head.load();
	Node* tmp_tail = this->tail.load();
	new_node->Next.store(tail);
	while (!cur->Next.compare_exchange_weak(tmp_tail, new_node)) {
		while (cur->Next.load() != this->tail.load()) {
			if (cur->edge.first == j) {
				cur->edge.second.store(0);
				return;
			}
			cur = cur->Next.load();
		}
		tmp_tail = this->tail.load();
	}
}

//*********************************
//LockFreeGraph functions
LockFreeGraph::LockFreeGraph() {
	adjacency_list = nullptr;
}

void LockFreeGraph::create_graph(const std::size_t max_capacity) {
	if (adjacency_list.load() == nullptr) {
		int i = is_created.fetch_add(1);
		if (adjacency_list.load() == nullptr && i == 0) {
			std::cout << "The graph has been created with success!" << std::endl;

			adjacency_list.store(new LockFreeLinkedList[max_capacity + 1]); //+1 make the index exactly match the max_capacity
		}
		else {
			is_created.fetch_sub(1);
			std::cout << "The graph has been created!" << std::endl;
			return;
		}
	}
}

//add_vertex implementation using cas loop
void LockFreeGraph::add_vertex() {
	if (!is_graph_created()) {
		std::cout << "graph hasn't been created yet!" << std::endl;
		return;
	}

	std::size_t temp = current_capacity.load();
	if (temp < max_capacity) {
		while (!current_capacity.compare_exchange_weak(temp, temp + 1)) {
			if (temp + 1 <= max_capacity) {
				continue;
			}
			else {
				std::cout << "1 thread try adding vertex failed due to maximum capacity! Thread return nothing" << std::endl;
				return;
			}
		}
	}
	else {
		std::cout << "1 thread try adding vertex failed due to maximum capacity! Thread return nothing"<< std::endl;
		return;
	}
}

void LockFreeGraph::add_edge(const std::size_t i, const std::size_t j) {
	if (!is_graph_created()) {
		std::cout << "Graph hasn't been created yet!" << std::endl;
		return;
	}
	if (!is_vertex_valid(i, j)) {
		return;
	}
	Node* cur = adjacency_list.load()[i].head.load();

	while (cur) {
		if (cur->edge.first == j) {
			cur->edge.second.store(0);
			return;
		}

		cur = cur->Next.load();
	}
	adjacency_list[i].add_Node(i, j);
}


void LockFreeGraph::inc_label(const std::size_t i, std::size_t j, const std::size_t increment) {
	if (!is_graph_created()) {
		std::cout << "Graph hasn't been created yet!" << std::endl;
		return;
	}
	if (!is_vertex_valid(i, j)) {
		return;
	}
	Node* cur = adjacency_list.load()[i].head.load();
	while (cur) {
		if (cur->edge.first == j) {
			cur->edge.second.fetch_add(increment);
			std::cout << "Inc Label: succeed!" << std::endl;
			return;
		}
		cur = cur->Next.load();
	}
}

void LockFreeGraph::dec_label(const std::size_t i, std::size_t j, const std::size_t decrement) {
	if (!is_graph_created()) {
		std::cout << "Graph hasn't been created yet!" << std::endl;
		return;
	}
	if (!is_vertex_valid(i, j)) {
		return;
	}
	size_t tmp_weight;
	size_t decreased_val;
	Node* cur = adjacency_list.load()[i].head.load();
	while (cur) {
		tmp_weight = cur->edge.second.load();
		decreased_val = tmp_weight - decrement;
		if (cur->edge.first == j && tmp_weight >= decrement) {
			while (!cur->edge.second.compare_exchange_weak(tmp_weight, decreased_val)) {
				tmp_weight = cur->edge.second.load();
				decreased_val = tmp_weight - decrement;
				if (tmp_weight < decrement) {
					std::cout << "Dec_label Error: decrement will result the weight of edge less than 0!" << std::endl;
					return;
				}
			}
			return;
		}
		else if (cur->edge.first == j && cur->edge.second.load() < decrement) {
			std::cout << "Dec_label Error: decrement will result the weight of edge less than 0!" << std::endl;
			return;
		}
		cur = cur->Next.load();
	}
}


bool LockFreeGraph::are_connected(const std::size_t i, const std::size_t j) {
	if (!is_graph_created()) {
		std::cout << "Graph hasn't been created yet!" << std::endl;
		return false;
	}
	if (!is_vertex_valid(i, j)) {
		return false;
	}
	Node* cur = adjacency_list.load()[i].head.load();
	while (cur) {
		if (cur->edge.first == j) {
			return true;
		}
		cur = cur->Next.load();
	}
	return false;
}

bool LockFreeGraph::is_reachable(const std::size_t i, const std::size_t j) {
	if (!is_graph_created()) {
		std::cout << "Graph hasn't been created yet!" << std::endl;
		return false;
	}
	if (!is_vertex_valid(i, j)) {
		return false;
	}
	std::queue<std::size_t> BFSqueue;
	std::vector<std::size_t> is_visited;
	BFSqueue.push(i);
	while (!BFSqueue.empty()) {
		size_t front = BFSqueue.front();
		is_visited.emplace_back(front);
		BFSqueue.pop();
		Node* tmp = adjacency_list.load()[front].head.load()->Next.load();
		while (tmp != adjacency_list.load()[front].tail.load()) {
			if (tmp->edge.first == j) {
				return true;
			}
			else {
				auto it = std::find(is_visited.begin(), is_visited.end(), tmp->edge.first);
				if (it != is_visited.end()) {
					BFSqueue.push(tmp->edge.first);
				}
			}
			tmp = tmp->Next.load();
		}
	}
	return false;
}

//cmp function for shortest_path's priority queue
struct cmp {
	bool operator()(std::vector<std::size_t>& a, std::vector<std::size_t>& b) {
		return a[2] > b[2];
	}
};

std::size_t LockFreeGraph::shortest_path(std::size_t i, std::size_t j) {
	if (!is_graph_created()) {
		std::cout << "Graph hasn't been created yet!" << std::endl;
		return -1;
	}
	if (!is_vertex_valid(i, j)) {
		return -1;
	}
	if (!is_reachable(i, j)) {
		std::cout << "Shortest path: The path from i to j does not exist!" << std::endl;
		return 0;
	}

	// Used to store the distance from src to all other nodes
	std::vector<std::size_t> distances(max_capacity + 1, NO_DISTANCE); 
	distances[i] = 0;

	std::vector<std::vector<std::size_t>> pq;
	Node* tmp = adjacency_list.load()[i].head.load()->Next.load();

	while (tmp != adjacency_list.load()[i].tail.load()) {
		size_t weight = tmp->edge.second.load();
		distances[tmp->edge.first] = weight;
		std::vector<std::size_t> src_adjacent = { i, tmp->edge.first, weight };
		pq.emplace_back(src_adjacent);
		tmp = tmp->Next.load();
	}

	std::make_heap(pq.begin(), pq.end(), cmp());

	while (!pq.empty()) {
		std::pop_heap(pq.begin(), pq.end(), cmp());
		std::vector<std::size_t> nearest_vertex = pq.back();
		pq.pop_back();
		Node* tmp = adjacency_list.load()[nearest_vertex[1]].head.load()->Next.load();

		while (tmp != adjacency_list.load()[nearest_vertex[1]].tail.load()) {
			size_t weight = tmp->edge.second.load();

			if (distances[tmp->edge.first] == NO_DISTANCE) {
				distances[tmp->edge.first] = distances[nearest_vertex[1]] + weight;
				std::vector<std::size_t> adjacent = { nearest_vertex[1], tmp->edge.first, weight };
				pq.push_back(adjacent);
				std::push_heap(pq.begin(), pq.end(), cmp());
			}
			else if (distances[nearest_vertex[1]] + weight < distances[tmp->edge.first]) {
				distances[tmp->edge.first] = distances[nearest_vertex[1]] + weight;
			}

			tmp = tmp->Next.load();
		}
	}

	size_t result = distances[j];
	return result == NO_DISTANCE ? -1 : result;
}

std::size_t LockFreeGraph::get_diameter() {
	size_t longest_distance = 0;
	size_t capacity = current_capacity.load();
	for (size_t i = 1; i <= capacity; ++i) {
		for (size_t j = 1; j <= capacity; ++j) {
			if (i != j && is_reachable(i, j)) {
				size_t tmp = shortest_path(i, j);
				if (longest_distance < tmp) {
					longest_distance = tmp;
				}
			}
		}
	}
	return longest_distance;
}

bool LockFreeGraph::is_vertex_valid(const std::size_t i, const std::size_t j) {
	size_t tmp = current_capacity.load();
	if (i > tmp || j > tmp || i == 0 || j == 0) {
		std::cout << "add_edge error: one of the vertex has not yet been created!" << std::endl;
		return false;
	}
	else if (i == j) {
		std::cout << "add_edge error: the edge are connecting the same vertex!" << std::endl;
		return false;
	}
	return true;
}

bool LockFreeGraph::is_graph_created() {
	return is_created.load() > 0;


}

int main() {
	LockFreeGraph LFG;

	std::vector<std::thread> threadPool;

	for (int i = 0; i < 10; ++i) {
		threadPool.push_back(std::thread(&LockFreeGraph::create_graph, &LFG, max_capacity));
	
	}
	for (auto it = threadPool.begin(); it != threadPool.end(); it++) {
		it->join();
	}

	threadPool.clear();

	for (int i = 0; i < 11; ++i) {
		//threadPool.push_back(std::thread(&LockFreeGraph::add_vertex, &LFG));
		LFG.add_vertex();
	}
	for (auto it = threadPool.begin(); it != threadPool.end(); it++) {
		it->join();
	}

	std::cout << "Number of vertex created: " << LFG.current_capacity.load() << std::endl;

	threadPool.clear();
	
	for (int i = 1; i < 10; ++i) {
		threadPool.push_back(std::thread(&LockFreeGraph::add_edge, &LFG, 1, i));
	}
	for (auto it = threadPool.begin(); it != threadPool.end(); it++) {
		it->join();
	}
	threadPool.clear();
	LFG.display_edges(1);
	LFG.inc_label(1, 2, 0);
	LFG.add_edge(2, 3);
	LFG.inc_label(2, 3, 4);
	LFG.add_edge(3, 4);
	LFG.inc_label(3, 4, 10);
	for (int i = 0; i < 5; ++i) {
		if (i % 2 == 0) {
			//threadPool.push_back(std::thread(&LockFreeGraph::inc_label, &LFG, 1, 4, 3));
			LFG.inc_label(1, 4, 3);
		}
		else {
			LFG.dec_label(1, 3, 1);
			//threadPool.push_back(std::thread(&LockFreeGraph::dec_label, &LFG, 1, 3, 1));
		}
	}
	for (auto it = threadPool.begin(); it != threadPool.end(); it++) {
		it->join();
	}

	threadPool.clear();

	//std::size_t i = 1;
	//std::size_t j = 8;
	for (int t = 0; t < 5; ++t) {
		LFG.delete_edge(1, 8);
		//threadPool.push_back(std::thread(&LockFreeGraph::delete_edge, &LFG, i, j));
	}
	for (auto it = threadPool.begin(); it != threadPool.end(); it++) {
		it->join();
	}

		LFG.display_edges(1);
		std::cout << "is Connected 1->3: " << LFG.are_connected(1, 3) << std::endl;
		std::cout << "is Connected 1->8: " << LFG.are_connected(1, 8) << std::endl;
		std::cout << "is Reachable 1->4: " << LFG.is_reachable(1, 4) << std::endl;
		std::cout << "shortest path 1->4: " << LFG.shortest_path(1, 4) << std::endl;
		std::cout << "The diameter of graph: " << LFG.get_diameter() << std::endl;


		return 0;

	}
