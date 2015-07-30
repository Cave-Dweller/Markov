#ifndef MARKOVCHAIN_HPP
#define MARKOVCHAIN_HPP
#include <map>
#include <unordered_map>
#include <stack>
#include <random>
#include <cmath>
#include <limits>
#include <iostream>

#include "MarkovSequence.hpp"

template<typename State>
class MarkovChain
{
	public:
		MarkovChain();
		MarkovChain(size_t chainLen);
		~MarkovChain();

		void Train(std::vector<std::vector<State>> trainingData);

		MarkovSequence<State> GenerateSequence();
		MarkovSequence<State> GenerateSequence(State seedValue);
		MarkovSequence<State> GenerateSequence(MarkovSequence<State> partialSequence);

		void AdvanceSequence(MarkovSequence<State>& ms);

	private:

		struct Node {
			Node(State s, size_t d, Node* parent = nullptr);
			~Node();
			State state;
			size_t depth;
			Node* previous;
			std::unordered_map<Node*, float> nextProbabilities;
			std::map<State, Node*> children;

			Node* GetNext();
			Node* GetChild(State s);
			void CalculateProbabilities();
		};

		size_t chainLength;
		Node* root;
		Node* endState;

		static std::random_device rd;
		static std::mt19937 generator;
		static std::uniform_real_distribution<float> sampler;
};

template<typename State> std::random_device MarkovChain<State>::rd;

template<typename State> std::mt19937 MarkovChain<State>::generator(rd());

template<typename State> std::uniform_real_distribution<float> MarkovChain<State>::sampler(0.0f, 100.0f);


template<typename State>
MarkovChain<State>::MarkovChain() : chainLength(3), root(new Node(State{}, 0)), endState(new Node(State{}, -1)) {
	generator.seed(rand());
}

template<typename State>
MarkovChain<State>::MarkovChain(size_t chainLen) : chainLength(std::max(chainLen, (size_t)(1))), root(new Node(State{}, 0)), endState(new Node(State{}, -1)) {
	generator.seed(rand());
}

template<typename State>
MarkovChain<State>::~MarkovChain() {
	delete root;
	delete endState;
}

template<typename State>
void MarkovChain<State>::Train(std::vector<std::vector<State>> trainingData) {

	delete root;
	root = new Node(State{}, 0);

	for(size_t i = 0; i < trainingData.size(); i++) {
		Node* current = root;

		for(size_t j = 0; j < trainingData[i].size(); j++) {

			if(current->depth < chainLength) {
				if(!current->GetChild(trainingData[i][j])) {

					current->children[trainingData[i][j]] = new Node(trainingData[i][j], current->depth + 1, current);
					current->nextProbabilities[current->children[trainingData[i][j]]] = 1.0f;
					current = current->children[trainingData[i][j]];

				} else {

					current->nextProbabilities[current->children[trainingData[i][j]]] += 1.0f;
					current = current->children[trainingData[i][j]];

				}
			} else {
				Node* nextChain = current;
				std::stack<State> chain;

				for(size_t i = 0; chainLength >= 1 && i < chainLength - 1; i++) {
					chain.push(nextChain->state);
					nextChain = nextChain->previous;
				}

				nextChain = root;

				if(!root->GetChild(chain.top())) {

					State topState = chain.top();

					while(!chain.empty()) {
						nextChain->children[chain.top()] = new Node(chain.top(), nextChain->depth + 1, nextChain);
						nextChain->nextProbabilities[nextChain->children[chain.top()]] = 1.0f;
						nextChain = nextChain->children[chain.top()];
						chain.pop();
					}

					current->nextProbabilities[root->GetChild(topState)] = 1.0f;
					current = nextChain;

					current->children[trainingData[i][j]] = new Node(trainingData[i][j], current->depth + 1, current);
					current->nextProbabilities[current->children[trainingData[i][j]]] = 1.0f;
					current = current->children[trainingData[i][j]];

				} else {

					current->nextProbabilities[root->GetChild(chain.top())] += 1.0f;

					while(!chain.empty() && nextChain->GetChild(chain.top())) {
						nextChain->nextProbabilities[nextChain->children[chain.top()]] += 1.0f;
						nextChain = nextChain->GetChild(chain.top());
						chain.pop();
					}

					if(!chain.empty()) {
						while(!chain.empty()) {
							nextChain->children[chain.top()] = new Node(chain.top(), nextChain->depth + 1, nextChain);
							nextChain->nextProbabilities[nextChain->children[chain.top()]] = 1.0f;
							nextChain = nextChain->children[chain.top()];

							chain.pop();
						}
					}

					nextChain->children[trainingData[i][j]] = new Node(trainingData[i][j], nextChain->depth + 1, nextChain);
					nextChain->nextProbabilities[nextChain->children[trainingData[i][j]]] = 1.0f;

					current = nextChain->children[trainingData[i][j]];

				}
			}
		}

		if(current->nextProbabilities.find(endState) != current->nextProbabilities.end()) {
			current->nextProbabilities[endState] += 1.0f;
		} else {
			current->nextProbabilities[endState] = 1.0f;
		}
	}

	root->CalculateProbabilities();
}

template<typename State>
MarkovSequence<State> MarkovChain<State>::GenerateSequence() {
	MarkovSequence<State> ms;

	Node* n = root->GetNext();

	while(n && n != endState) {
		ms.Append(n->state);
		n = n->GetNext();
	}

	return ms;
}

template<typename State>
MarkovSequence<State> MarkovChain<State>::GenerateSequence(State seedValue) {
	MarkovSequence<State> ms;
	ms.Append(seedValue);

	Node* n = root->GetChild(seedValue);

	while(n && n != endState) {
		ms.Append(n->state);
		n = n->GetNext();
	}

	return ms;
}

template<typename State>
MarkovSequence<State> MarkovChain<State>::GenerateSequence(MarkovSequence<State> partialSequence) {
	MarkovSequence<State> ms;

	Node* n = root;

	for(size_t i = 0; i < partialSequence.size(); i++) {
		if(n) {
			n = n->GetChild(partialSequence[i]);
		}

		ms.Append(partialSequence[i]);
	}

	if(n) {
		n = n->GetNext();
	}

	while(n && n != endState) {
		ms.Append(n->state);
		n = n->GetNext();
	}

	return ms;
}

template<typename State>
void MarkovChain<State>::AdvanceSequence(MarkovSequence<State>& ms) {
	size_t start = 0;

	if(ms.size() > chainLength) {
		start = ms.size() - chainLength;
	}

	Node* n = root;

	for(size_t index = start; n && index < ms.size(); index++) {
		n = n->GetChild(ms[index]);
	}

	if(n) {
		n = n->GetNext();
	}

	if(n && n != endState) {
		ms.Append(n->state);
	}
}

template<typename State>
MarkovChain<State>::Node::Node(State s, size_t d, Node* parent) {
	state = s;
	depth = d;
	previous = parent;
}

template<typename State>
MarkovChain<State>::Node::~Node() {
	for(auto& p : children) {
		delete p.second;
	}
}

template<typename State>
typename MarkovChain<State>::Node* MarkovChain<State>::Node::GetNext() {
	if(nextProbabilities.size() == 0) {
		return nullptr;
	}

	float choice = MarkovChain<State>::sampler(MarkovChain<State>::generator);
	float sum = 0.0f;

	Node* n = nullptr;

	for(auto& p : nextProbabilities) {
		sum += p.second;
		n = p.first;

		if(sum >= choice) {
			return p.first;
		}
	}

	return n;
}

template<typename State>
typename MarkovChain<State>::Node* MarkovChain<State>::Node::GetChild(State s) {
	auto it = children.find(s);
	if(it != children.end()) {
		return it->second;
	}

	return nullptr;
}

template<typename State>
void MarkovChain<State>::Node::CalculateProbabilities() {
	float total = 0.0f;

	for(auto& p : nextProbabilities) {
		total += p.second;
	}

	for(auto& p : nextProbabilities) {
		p.second = 100.0f * (p.second / total);
	}

	for(auto& p : children) {
		p.second->CalculateProbabilities();
	}
}

#endif // MARKOVCHAIN_HPP
