#ifndef MARKOVTRIE_HPP
#define MARKOVTRIE_HPP
#include <vector>
#include <map>
#include <random>
#include <utility>
#include <cstdlib>
#include "MarkovSequence.hpp"

template<typename State>
class MarkovTrie
{
	public:
		MarkovTrie();
		MarkovTrie(std::vector<std::vector<State>> trainData);
		~MarkovTrie();

		void Train(std::vector<std::vector<State>> trainingData);

		MarkovSequence<State> GenerateSequence();
		MarkovSequence<State> GenerateSequence(State seedValue);
		MarkovSequence<State> GenerateSequence(MarkovSequence<State> partialSequence);

		void AdvanceSequence(MarkovSequence<State>& ms);

		void SeedGenerator(uint64_t seed);

	private:

		struct TrieNode {
			TrieNode(State s);
			TrieNode(State s, TrieNode* par);
			TrieNode(State s, TrieNode* par, std::vector<std::vector<State>> trainingData);
			~TrieNode();

			State state;
			std::vector<std::pair<TrieNode*, float>> children;
			TrieNode* parent;

			TrieNode* GetNext();
			TrieNode* GetChild(State s);
			void Train(std::vector<std::vector<State>> trainingData);
		};

		TrieNode* rootNode;

		static std::random_device rd;
		static std::mt19937 generator;
		static std::uniform_real_distribution<float> sampler;
};

template<typename State> std::random_device MarkovTrie<State>::rd;

template<typename State> std::mt19937 MarkovTrie<State>::generator(rd());

template<typename State> std::uniform_real_distribution<float> MarkovTrie<State>::sampler(0.0f, 100.0f);

template<typename State>
MarkovTrie<State>::MarkovTrie() {
	rootNode = new TrieNode(State{});
}

template<typename State>
MarkovTrie<State>::MarkovTrie(std::vector<std::vector<State>> trainingData) {
	rootNode = new TrieNode(State{});

	Train(trainingData);
}

template<typename State>
MarkovTrie<State>::~MarkovTrie() {
	delete rootNode;
}

template<typename State>
MarkovTrie<State>::TrieNode::TrieNode(State s) {
	state = s;
	parent = nullptr;
}

template<typename State>
MarkovTrie<State>::TrieNode::TrieNode(State s, TrieNode* par) {
	state = s;
	parent = par;
}

template<typename State>
MarkovTrie<State>::TrieNode::TrieNode(State s, TrieNode* par, std::vector<std::vector<State>> trainingData) {
	state = s;
	parent = par;

	Train(trainingData);
}

template<typename State>
MarkovTrie<State>::TrieNode::~TrieNode() {
	for(size_t i = 0; i < children.size(); i++) {
		delete children[i].first;
	}
}

template<typename State>
void MarkovTrie<State>::Train(std::vector<std::vector<State>> trainingData) {
	rootNode->Train(trainingData);
}

template<typename State>
MarkovSequence<State> MarkovTrie<State>::GenerateSequence() {
	MarkovSequence<State> ms;

	if(!rootNode) {
		return ms;
	}

	TrieNode* t = rootNode->GetNext();

	while(t) {
		ms.Append(t->state);
		t = t->GetNext();
	}

	return ms;
}

template<typename State>
MarkovSequence<State> MarkovTrie<State>::GenerateSequence(State seedValue) {
	MarkovSequence<State> ms;

	if(!rootNode) {
		return ms;
	}

	TrieNode* t = rootNode->GetChild(seedValue);

	while(t) {
		ms.Append(t->state);
		t = t->GetNext();
	}

	return ms;
}

template<typename State>
MarkovSequence<State> MarkovTrie<State>::GenerateSequence(MarkovSequence<State> partialSequence) {
	MarkovSequence<State> ms;

	if(!rootNode) {
		return ms;
	}

	TrieNode* t = rootNode;

	for(size_t i = 0; (i < partialSequence.size()) && (t !=  nullptr); i++) {
		t = t->GetChild(partialSequence[i]);
		ms.Append(partialSequence[i]);
	}
	t = t->GetNext();

	while(t) {
		ms.Append(t->state);
		t = t->GetNext();
	}

	return ms;
}

template<typename State>
void MarkovTrie<State>::SeedGenerator(uint64_t seed) {
	generator.seed(seed);
}

template<typename State>
void MarkovTrie<State>::AdvanceSequence(MarkovSequence<State>& ms) {
	TrieNode* t = rootNode;

	for(size_t i = 0; (i < ms.size()) && (t !=  nullptr); i++) {
		t = t->GetChild(ms[i]);
	}

	t = t->GetNext();

	if(t) {
		ms.Append(t->state);
	}
}

template<typename State>
typename MarkovTrie<State>::TrieNode* MarkovTrie<State>::TrieNode::GetNext() {
	if(children.size() == 0) {
		return nullptr;
	}

	float choice = MarkovTrie<State>::sampler(MarkovTrie<State>::generator);
	float sum = 0.0f;

	for(size_t i = 0; i < children.size(); i++) {
		sum += children[i].second;

		if(sum >= choice) {
			return children[i].first;
		}
	}

	return children[children.size() - 1].first;
}

template<typename State>
typename MarkovTrie<State>::TrieNode* MarkovTrie<State>::TrieNode::GetChild(State s) {
	if(children.size() == 0) {
		return nullptr;
	}

	for(size_t i = 0; i < children.size(); i++) {
		if(children[i].first->state == s) {
			return children[i].first;
		}
	}

	return nullptr;
}

template<typename State>
void MarkovTrie<State>::TrieNode::Train(std::vector<std::vector<State>> trainingData) {
    std::map<State, size_t> counts;
    std::map<State, std::vector<std::vector<State>>> subTrainingData;
    size_t total = 0;

    for(size_t i = 0; i < trainingData.size(); i++) {
		if(trainingData[i].size() != 0) {
			if(counts.find(trainingData[i][0]) != counts.end()) {
				counts[trainingData[i][0]]++;
			} else {
				counts[trainingData[i][0]] = 1;
			}

			if(subTrainingData.find(trainingData[i][0]) != subTrainingData.end()) {
				subTrainingData[trainingData[i][0]].push_back(std::vector<State>(trainingData[i].begin() + 1, trainingData[i].end()));
			} else {
				subTrainingData[trainingData[i][0]] = std::vector<std::vector<State>>{
					std::vector<State>{trainingData[i].begin() + 1, trainingData[i].end()}
				};
			}

			total++;
		} else {
			break;
		}
    }

    for(auto& p : counts) {
		children.push_back(std::pair<TrieNode*, float>{new TrieNode(p.first, this, subTrainingData[p.first]), 100.0f * (float)(p.second)/(float)(total)});
    }
}

#endif // MARKOVTRIE_HPP
