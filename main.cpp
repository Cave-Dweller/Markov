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

	std::cout << "Enter a seed for the random number generator.\n";
	std::uint32_t seed;
	std::cin >> seed;

	srand(seed);

	std::mt19937 gen(rand());

	std::cout << "\nEnter the number of characters to keep track of while training (longer is better quality, but shorter is higher quantity).\n";
	size_t len;
	std::cin >> len;

	std::cout << "\nEnter the minimum name length you'd want.\n";
	size_t minLength;
	std::cin >> minLength;

	std::cout << "\nEnter the maximum name length you'd want.\n";
	size_t maxLength;
	std::cin >> maxLength;

	MarkovChain<char> wordGenerator{len};

	std::set<char> capitals = {'A', 'B', 'C', 'D', 'E', 'F', 'G',
	                           'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
	                           'Q', 'R', 'S',
	                           'T', 'U', 'V',
	                           'W', 'X',
	                           'Y',
	                           'Z'};

	std::set<char> lowerCase = {'a', 'b', 'c', 'd', 'e', 'f', 'g',
	                            'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p',
	                            'q', 'r', 's',
	                            't', 'u', 'v',
	                            'w', 'x',
	                            'y',
	                            'z'};

	std::set<char> consonants = {'b', 'c', 'd', 'f', 'g', 'h', 'j',
	                             'k', 'l', 'm', 'n', 'p', 'q', 'r',
	                             's', 't', 'v', 'w', 'x', 'z'};

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

		MarkovSequence<char> sequence = wordGenerator.GenerateSequence();

		if(capitals.find(sequence[0]) == capitals.end()) {
			sequence = MarkovSequence<char>{};
			std::cout << "\tREJECTED: NOT CAPITALIZED\n";
			continue;
		}

		if(sequence.size() < minLength) {
			std::cout << "\tREJECTED: TOO SHORT\n";
			continue;
		}

		if(sequence.size() > maxLength) {
			std::cout << "\tREJECTED: TOO LONG\n";
			continue;
		}

		if(sequence.size() >= 3) {
			char c = sequence[0];
			size_t sameCount = 1;
			size_t consonantCount = 0;
			size_t lastConsonant = 0;
			size_t capitalCount = 0;
			size_t lastCapital = 0;

			for(size_t i = 0; i < sequence.size(); i++) {

				if(lowerCase.find(sequence[i]) != lowerCase.end() && sequence[i-1] == ' ') {
					std::cout << "\tREJECTED: OUT OF PLACE LOWER CASE\n";
					sequence = MarkovSequence<char>{};
				}

				if(capitals.find(sequence[i]) != capitals.end()) {
					if(lastCapital == i-1) {
						++capitalCount;
					} else {
						capitalCount = 0;
					}
					lastCapital = i;

					if(lowerCase.find(sequence[i-1]) != lowerCase.end()) {
						std::cout << "\tREJECTED: OUT OF PLACE CAPITAL\n";
						sequence = MarkovSequence<char>{};
					}
				}

				if(consonants.find(sequence[i]) != consonants.end()) {

					if(lastConsonant == i-1) {
						++consonantCount;
					} else {
						consonantCount = 0;
					}

					lastConsonant = i;

				}

				if(c != sequence[i]) {
					c = sequence[i];
					sameCount = 1;
				} else {
					++sameCount;
				}

				if(capitalCount >= 1) {
					std::cout << "\tREJECTED: TOO MANY CONSECUTIVE CAPITALS\n";
					sequence = MarkovSequence<char>{};
				}

				if(consonantCount >= 3) {
					std::cout << "\tREJECTED: TOO MANY CONSECUTIVE CONSONANTS\n";
					sequence = MarkovSequence<char>{};
				}

				if(sameCount >= 3) {
					std::cout << "\tREJECTED: TOO MANY REPEATED LETTERS\n";
					sequence = MarkovSequence<char>{};
				}
			}
		}

		if(sequence.size() == 0) {
			continue;
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

	std::fstream outFile("./generated names.txt", std::ios::trunc | std::ios::out);
	outFile << "\n";

	for(auto& planetName : generatedNames) {
		std::cout << planetName << "\n";
		outFile << planetName << "\n";
	}

    return 0;
}
