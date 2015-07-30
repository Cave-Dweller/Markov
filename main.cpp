#include <iostream>
#include "MarkovTrie.hpp"
#include "MarkovChain.hpp"
#include <string>
#include <fstream>
#include <cstdlib>
#include <algorithm>
#include <time.h>
#include <set>

std::vector<std::string> split(const std::string& s, char delimiter = ' ') {
	std::vector<std::string> substrings;

	size_t start, count;
	start = count = 0;

	while(start+count < s.length()) {
		while(s[start+count] != delimiter && start + count < s.length()) {
			++count;
		}

		substrings.push_back(s.substr(start, count));
		++count;
		start = count;
	}

	substrings.push_back(s.substr(start));

	return substrings;
}

int main()
{

	srand(time(NULL));

	std::mt19937 gen(rand());

	std::cout << "Enter the number of characters in a chain.\n";
	size_t len;
	std::cin >> len;

	MarkovChain<char> wordGenerator{len};
	//MarkovTrie<char> wordGenerator;

	std::set<char> capitals = {'A', 'B', 'C', 'D', 'E', 'F', 'G',
	                           'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
	                           'Q', 'R', 'S',
	                           'T', 'U', 'V',
	                           'W', 'X', 'Y',
	                           'Z'};

	std::set<std::string> names;

	std::vector<std::vector<char>> trainingData;

	std::ifstream fileInput;
	fileInput.open("./src/planet names.txt");

	if(!fileInput) {
		std::cerr << "Error opening training data.\n";
		return 1;
	}

	std::vector<std::string> words;

	while(!fileInput.eof()) {
		std::string name;
		std::getline(fileInput, name);
		words.push_back(name);
		names.insert(name);
	}

	std::set<std::string> acceptableStarts;

	std::shuffle(words.begin(), words.end(), gen);

	for(auto& name : words) {
		std::cout << name << "\n";
	}

	std::cout << "\n\n";

	for(size_t i = 0; i < words.size(); i++) {
		trainingData.push_back(std::vector<char>{words[i].begin(), words[i].end()});
	}

	wordGenerator.Train(trainingData);

	std::cout << "\n\n";

	std::set<std::string> generatedNames;

	std::cout << "Enter the number of planet names you want.\n";
	size_t n;
	std::cin >> n;

	while(generatedNames.size() != n) {

		MarkovSequence<char> sequence{};

		size_t previousSize = -1;

		while(sequence.size() != previousSize) {
			previousSize = sequence.size();
			wordGenerator.AdvanceSequence(sequence);

			if(capitals.find(sequence[0]) == capitals.end()) {
				sequence = MarkovSequence<char>{};
				previousSize = -1;
				std::cout << "\tREJECTED: NOT CAPITALIZED\n";
			}

		}

		std::string constructedName = "";

		for(size_t j = 0; j < sequence.size(); j++) {
			constructedName.push_back(sequence[j]);
		}

		if(names.find(constructedName) == names.end()) {
			if(generatedNames.find(constructedName) == generatedNames.end()) {
				std::cout << constructedName << "\n";
				generatedNames.insert(constructedName);

				std::cout << "\nCount: " << generatedNames.size() << "\n";
			} else {
				std::cout << "\tREJECTED: NOT UNIQUE\n";
			}
		} else {
			std::cout << "\tREJECTED: NOT GENERATED\n";
		}

	}

	std::cout << "\n\n";

	std::fstream outFile("generated names.txt", std::ios::trunc | std::ios::out);

	for(auto& planetName : generatedNames) {
		std::cout << planetName << "\n";
		outFile << planetName << "\n";
	}

    return 0;
}
