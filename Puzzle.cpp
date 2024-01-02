#include <vector>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <random>
#include <iostream>
#include <fstream>
#include <sstream>
#include <json/json.h>  // Assuming you have a JSON library, like jsoncpp
#include "Puzzle.h"

template class Permutation<PUZZLE_SIZE>;
template class Puzzle<PUZZLE_SIZE>;

template <std::size_t Size>
mapping_t<Size> getInverse(const mapping_t<Size>& permutation) {
    mapping_t<Size> inverse;
    for (int i = 0; i < permutation.size(); ++i) {
		inverse[permutation[i]] = i;
    }
    return inverse;
}

template <std::size_t Size>
Permutation<Size>::Permutation(const mapping_t<Size>& mapping, const move_ids_t& move_ids)
    : _mapping(mapping), _move_ids(move_ids) {}

template <std::size_t Size>
Permutation<Size>::Permutation(const mapping_t<Size>& mapping, const unsigned char move_id)
    : _mapping(mapping), _move_ids(move_ids_t {move_id}) {}

template <std::size_t Size>
Permutation<Size>::Permutation() {
    _mapping = mapping_t<Size>();
    // Initialize the mapping to the identity permutation.
    for (size_t i = 0; i < Size; i++)
    {
        _mapping[i] = i;
    }
    _move_ids = move_ids_t();
}

template <std::size_t Size>
Permutation<Size> Permutation<Size>::from(int start, const std::vector<Permutation>& puzzle_info) {
    Permutation p;
    for (int i = start; i < length(); ++i) {
		p = p * puzzle_info[_move_ids[i]];
	}
    return p;
}

//Permutation<Size>::~Permutation()
//{
//    // Deallocate the memory that was previously reserved for the string.
//    delete[] *_mapping;
//    delete[] _move_ids;
//}

template <std::size_t Size>
Permutation<Size> Permutation<Size>::until(int end, const std::vector<Permutation<Size>>& puzzle_info) {
    Permutation p;
    for (int i = 0; i < end; ++i) {
        p = p * puzzle_info[_move_ids[i]];
    }
    return p;
}

template <std::size_t Size>
const move_ids_t& Permutation<Size>::move_ids() const {
    return _move_ids;
}

template <std::size_t Size>
std::vector<Permutation<Size>> Permutation<Size>::split(const std::vector<Permutation>& puzzle_info) const {
    std::vector<Permutation> result;
    for (const auto& move_id : _move_ids) {
        result.push_back(puzzle_info[move_id]);
    }
    return result;
}

template <std::size_t Size>
const mapping_t<Size>& Permutation<Size>::mapping() const {
    return _mapping;
}

template <std::size_t Size>
Permutation<Size> Permutation<Size>::operator*(const Permutation& other) const {
    mapping_t<Size> result_mapping;
    for (int i = 0; i < _mapping.size(); ++i) {
		result_mapping[i] = _mapping[other._mapping[i]];
	}
    move_ids_t result_move_ids = _move_ids;
    result_move_ids.insert(result_move_ids.end(), other._move_ids.begin(), other._move_ids.end());
    return Permutation(result_mapping, result_move_ids);
}

template <std::size_t Size>
Permutation<Size> Permutation<Size>::inverse() const {
    move_ids_t inverse_move_ids(_move_ids.size(), -1);
    for (int i = 0; i < _move_ids.size(); ++i)
    {
        if (_move_ids[i] % 2 == 0) {
            inverse_move_ids[_move_ids.size() - 1 - i] = _move_ids[i] + 1;
        }
        else {
            inverse_move_ids.push_back(_move_ids[i] - 1);
        }
    }
    return Permutation(getInverse(_mapping), inverse_move_ids);
}

template <std::size_t Size>
bool Permutation<Size>::operator<(const Permutation& other) const {
    if (length() < other.length()) {
        return true;
    } else {
        return false;
    }
}

template <std::size_t Size>
bool Permutation<Size>::operator<=(const Permutation& other) const {
    if (length() <= other.length()) {
        return true;
    } else {
        return false;
    }
}

template <std::size_t Size>
std::ostream& operator<<(std::ostream& os, const Permutation<Size>& permutation) {
    os << (int)permutation._move_ids[0];
    for (int i = 1; i < permutation._move_ids.size(); ++i) {
        os << "." << (int)permutation._move_ids[i];
	}
    return os;
 }

template <std::size_t Size>
bool Permutation<Size>::operator==(const Permutation& other) const {
    return _mapping == other._mapping;
}

std::vector<std::string> splitSolution(const std::string& str)
{
	std::vector<std::string> result;
    std::string currentMove = "";
    for (char c : str) {
        if (c == ',') {
			result.push_back(currentMove);
			currentMove = "";
		}
        else {
			currentMove.push_back(c);
		}
	}
	return result;
}


template <std::size_t Size>
Puzzle<Size>::Puzzle(int id, const std::string& puzzle_type, const std::string& solution, const std::string& initial, int num_wildcards)
    : _id(id), _type(puzzle_type), _solution(splitSolution(solution)), _initial(splitSolution(initial)), _current(splitSolution(initial)), _num_wildcards(num_wildcards) {
}

template <std::size_t Size>
Puzzle<Size>::Puzzle()
    : _id(-1), _type(""), _solution(std::vector<std::string>()), _initial(std::vector<std::string>()), _current(std::vector<std::string>()), _num_wildcards(0) {
}

std::vector<std::string> split_string(const std::string& str, char delimiter)
{
	std::vector<std::string> result;
	std::string current_str = "";
    for (char c : str) {
        if (c == delimiter) {
			result.push_back(current_str);
            current_str = "";
		}
        else {
            current_str.push_back(c);
		}
	}
	result.push_back(current_str);
	return result;
}

template <std::size_t Size>
Puzzle<Size>::Puzzle(const std::string& input_line)
{
    std::vector<std::string> line = split_string(input_line, ',');
    _id = std::stoi(line[0]);
    _type = line[1];
    _solution = split_string(line[2], ';');
    _initial = split_string(line[3], ';');
    _current = split_string(line[3], ';');
    _num_wildcards = std::stoi(line[4]);
}

template <std::size_t Size>
void Puzzle<Size>::initializeMoveList(const std::unordered_map<std::string, mapping_t<Size>>& allowed_moves) {
    for (const auto& entry : allowed_moves) {
        _allowed_moves[entry.first] = entry.second;
        mapping_t<Size> inverse = getInverse(entry.second);
        if (inverse != entry.second) {
			_allowed_moves["-" + entry.first] = inverse;
		}
    }
}

template <std::size_t Size>
 std::vector<std::string> Puzzle<Size>::allowedMoveIds() const {
    std::vector<std::string> result;
    for (const auto& entry : _allowed_moves) {
        result.push_back(entry.first);
    }
    return result;
}

template <std::size_t Size>
std::vector<std::string> Puzzle<Size>::randomSolution(size_t size) const {
    std::vector<std::string> result;
    std::vector<std::string> moveIds = allowedMoveIds();
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> dist(0, moveIds.size() - 1);

    for (size_t i = 0; i < size; ++i) {
        result.push_back(moveIds[dist(gen)]);
    }

    return result;
}

template <std::size_t Size>
 Puzzle<Size>& Puzzle<Size>::permutate(const std::string& move_id) {
    _permutations.push_back(move_id);
    const mapping_t<Size>& permutation = _allowed_moves.at(move_id);
    for (size_t i = 0; i < permutation.size(); ++i) {
        _current[i] = _current[permutation[i]];
    }
    return *this;
}

template <std::size_t Size>
Puzzle<Size>& Puzzle<Size>::fullPermutation(const std::vector<std::string>& permutation_list) {
    for (const auto& move_id : permutation_list) {
        permutate(move_id);
    }
    return *this;
}

template <std::size_t Size>
Puzzle<Size> Puzzle<Size>::clone() const {
    Puzzle<Size> cloned_puzzle(_id, _type, "", "", _num_wildcards);
    cloned_puzzle._solution = _solution;
    cloned_puzzle._current = _current;
    cloned_puzzle._initial = _initial;
    cloned_puzzle._allowed_moves = _allowed_moves;
    return cloned_puzzle;
}

template <std::size_t Size>
int Puzzle<Size>::score() const {
    return 2 * (int)_current.size() * std::max(0, countMismatches() - _num_wildcards) + (int)_current.size();
}

template <std::size_t Size>
int Puzzle<Size>::countMismatches() const {
    int ctMistakes = 0;
    for (size_t i = 0; i < _solution.size(); i++)
    {
        if (_solution[i] != _current[i]) {
            ctMistakes++;
		}
    }
    return ctMistakes;
}

template <std::size_t Size>
std::vector<std::string> Puzzle<Size>::permutations() const {
    return _permutations;
}

template <std::size_t Size>
const std::string& Puzzle<Size>::type() const {
    return _type;
}

template <std::size_t Size>
bool Puzzle<Size>::isSolved() const {
    return countMismatches() <= getNumWildcards();
}

template <std::size_t Size>
std::string Puzzle<Size>::submission() const {
    std::string result = std::to_string(_id) + ",";
    for (const auto& move_id : _permutations) {
        result += move_id + ".";
    }
    return result;
}

template <std::size_t Size>
size_t Puzzle<Size>::size() const {
    return _solution.size();
}

template <std::size_t Size>
const std::string& Puzzle<Size>::operator[](size_t index) const {
    return _permutations[index];
}

// Vector osstreams
std::string concatenate_vector(const std::vector<std::string>& vec) {
    std::string result;
    for (std::string value : vec) {
        result += value;
	}
    return result;
}

template <std::size_t Size>
std::ostream& operator<<(std::ostream& os, const Puzzle<Size>& puzzle) {
    os << "----------\n"
        << puzzle._id << ": " << puzzle._type << " [" << puzzle._num_wildcards << "]\n"
        << concatenate_vector(puzzle._initial) << "\n"
        << concatenate_vector(puzzle._current) << "[" << puzzle.score() << "]\n"
        << concatenate_vector(puzzle._solution) << "\n"
        << puzzle.submission() << "\n"
        << "----------";
    return os;
}

