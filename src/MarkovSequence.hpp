#ifndef MARKOVSEQUENCE_HPP
#define MARKOVSEQUENCE_HPP


/* For now, this class is a severely limited wrapper around
 * std::vector<State>. This may change in the future if I
 * decide to link Markov Sequences to the structures used
 * to generate them (e.g. in order to advance the sequence).
*/
template<typename State>
class MarkovSequence
{
	public:
		MarkovSequence();
		MarkovSequence(std::vector<State> states);
		~MarkovSequence();

		inline void Append(State s) {
			sequence.push_back(s);
		}

		const State& operator[](size_t index) const {
			return sequence[index];
		}

		size_t size() const {
			return sequence.size();
		}

	private:
		std::vector<State> sequence;
};

template<typename State>
MarkovSequence<State>::MarkovSequence() {}

template<typename State>
MarkovSequence<State>::MarkovSequence(std::vector<State> states) {
	sequence = states;
}

template<typename State>
MarkovSequence<State>::~MarkovSequence() {}

#endif // MARKOVSEQUENCE_HPP
