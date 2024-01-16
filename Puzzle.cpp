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

template class Permutation<24>;
template class Permutation<54>;
template class Permutation<96>;
template class Permutation<150>;
template class Permutation<216>;
template class Permutation<294>;
template class Permutation<384>;
template class Permutation<486>;
template class Permutation<600>;
template class Permutation<2166>;
template class Permutation<6534>;
template class Permutation<10>;
template class Permutation<12>;
template class Permutation<22>;
template class Permutation<40>;
template class Permutation<64>;
template class Permutation<198>;
template class Permutation<32>;
template class Permutation<64>;
template class Permutation<36>;
template class Permutation<32>;
template class Permutation<56>;
template class Permutation<112>;
template class Permutation<140>;
template class Permutation<264>;
template class Permutation<450>;

template class Puzzle<24>;
template class Puzzle<54>;
template class Puzzle<96>;
template class Puzzle<150>;
template class Puzzle<216>;
template class Puzzle<294>;
template class Puzzle<384>;
template class Puzzle<486>;
template class Puzzle<600>;
template class Puzzle<2166>;
template class Puzzle<6534>;
template class Puzzle<10>;
template class Puzzle<12>;
template class Puzzle<22>;
template class Puzzle<40>;
template class Puzzle<64>;
template class Puzzle<198>;
template class Puzzle<32>;
template class Puzzle<64>;
template class Puzzle<36>;
template class Puzzle<32>;
template class Puzzle<56>;
template class Puzzle<112>;
template class Puzzle<140>;
template class Puzzle<264>;
template class Puzzle<450>;

template std::ostream& operator<<(std::ostream& os, const Puzzle<24>& puzzle);

template <std::size_t Size>
mapping_t<Size> getInverse(const mapping_t<Size>& permutation) {
    mapping_t<Size> inverse;
    for (int i = 0; i < permutation.size(); ++i) {
		inverse[permutation[i]] = i;
    }
    return inverse;
}


std::ostream& operator<<(std::ostream& os, const std::vector<std::string> arr) {
       
os << "[";
	for (int i = 0; i < arr.size(); ++i) {
		os << arr[i];
		if (i < arr.size() - 1) {
			os << "";
		}
	}
	os << "]";
	return os;
}

int levenshteinDist(const std::vector<std::string> word1, const std::vector<std::string> word2) {
    int size1 = word1.size();
    int size2 = word2.size();
    std::vector<std::vector<int>> verif(size1 + 1, std::vector<int>(size2 + 1)); // Verification matrix i.e. 2D array which will store the calculated distance.

    // If one of the words has zero length, the distance is equal to the size of the other word.
    if (size1 == 0)
        return size2;
    if (size2 == 0)
        return size1;

    // Sets the first row and the first column of the verification matrix with the numerical order from 0 to the length of each word.
    for (int i = 0; i <= size1; i++)
        verif[i][0] = i;
    for (int j = 0; j <= size2; j++)
        verif[0][j] = j;

    // Verification step / matrix filling.
    for (int i = 1; i <= size1; i++) {
        for (int j = 1; j <= size2; j++) {
            // Sets the modification cost.
            // 0 means no modification (i.e. equal letters) and 1 means that a modification is needed (i.e. unequal letters).
            int cost = (word2[j - 1] == word1[i - 1]) ? 0 : 1;

            // Sets the current position of the matrix as the minimum value between a (deletion), b (insertion) and c (substitution).
            // a = the upper adjacent value plus 1: verif[i - 1][j] + 1
            // b = the left adjacent value plus 1: verif[i][j - 1] + 1
            // c = the upper left adjacent value plus the modification cost: verif[i - 1][j - 1] + cost
            verif[i][j] = std::min(
                std::min(verif[i - 1][j] + 1, verif[i][j - 1] + 1),
                verif[i - 1][j - 1] + cost
            );
        }
    }

    //std::cout << std::endl << "Levenshtein distance(" << word1 << ", " << word2 << ")=" << verif[size1][size2] << std::endl;

    // The last position of the matrix will contain the Levenshtein distance.
    return verif[size1][size2];
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
            inverse_move_ids[_move_ids.size() - 1 - i] = _move_ids[i] - 1;
        }
    }
    return Permutation(getInverse(_mapping), inverse_move_ids);
}

template <std::size_t Size>
bool Permutation<Size>::isInverse(const Permutation& other) const {
	for (int i = 0; i < _mapping.size(); ++i) {
		if (_mapping[other._mapping[i]] != i) {
			return false;
		}
	}
	return true;
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
        if (c == ';') {
			result.push_back(currentMove);
			currentMove = "";
		}
        else {
			currentMove.push_back(c);
		}
	}
    result.push_back(currentMove);
	return result;
}


template <std::size_t Size>
Puzzle<Size>::Puzzle(int id, const std::string& puzzle_type, const std::string& solution, const std::string& initial, int num_wildcards)
    : _id(id), _type(puzzle_type), _solution(splitSolution(solution)), _initial(splitSolution(initial)), _current(splitSolution(initial)), _num_wildcards(num_wildcards) {

    if (Size == 96) {
        _component = std::vector<std::vector<int>>(4);
        _component[0] = { 0, 3, 12, 15, 16, 19, 28, 31, 32, 35, 44, 47, 48, 51, 60, 63, 64, 67, 76, 79, 80, 83, 92, 95 };
        _component[1] = { 1, 7, 8, 14, 17, 23, 24, 30, 33, 39, 40, 46, 49, 55, 56, 62, 65, 71, 72, 78, 81, 87, 88, 94 };
        _component[2] = { 2, 4, 11, 13, 18, 20, 27, 29, 34, 36, 43, 45, 50, 52, 59, 61, 66, 68, 75, 77, 82, 84, 91, 93 };
        _component[3] = { 5, 6, 9, 10, 21, 22, 25, 26, 37, 38, 41, 42, 53, 54, 57, 58, 69, 70, 73, 74, 85, 86, 89, 90 };
    }
    else if (Size == 54) {
        _component = std::vector<std::vector<int>>(3);
        _component[0] = { 0, 2, 6, 8, 9, 11, 15, 17, 18, 20, 24, 26, 27, 29, 33, 35, 36, 38, 42, 44, 45, 47, 51, 53 };
        _component[1] = { 1, 3, 5, 7, 10, 12, 14, 16, 19, 21, 23, 25, 28, 30, 32, 34, 37, 39, 41, 43, 46, 48, 50, 52 };
        _component[2] = { 4, 40, 13, 49, 22, 31 };

    }
    else if (Size == 64) {
        _component = std::vector<std::vector<int>>(3);
        _component[0] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63 };
    }
    else {
		_component = std::vector<std::vector<int>>(1);
		for (int i = 0; i < Size; ++i) {
			_component[0].push_back(i);
		}
    }
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
int Puzzle<Size>::countMismatches(int component_idx) const {
    // return countMismatches();
    int ctMistakes = 0;
    for (size_t i : _component[component_idx])
    {
        if (_solution[i] != _current[i]) {
            ctMistakes++;
		}
    }
    return ctMistakes;
}

template <std::size_t Size>
int Puzzle<Size>::countMismatches() const {
    // return levenshteinDist(_current, _solution);
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
int Puzzle<Size>::countMismatches(const Permutation<Size>& p) const {
    /*std::vector<std::string> result(Size);
    for (int i = 0; i < Size; ++i) {
        result[i] = _current[p.mapping()[i]];
    }
    return levenshteinDist(result, _solution);*/
    int ctMistakes = 0;
    for (size_t i = 0; i < _solution.size(); i++)
    {
        if (_current[p.mapping()[i]] != _solution[i])  {
            ctMistakes++;
		}
    }
    return ctMistakes;
}

template <std::size_t Size>
int Puzzle<Size>::countMismatches(const Permutation<Size>& p, int component_idx) const {
    // return countMismatches(p);
    int ctMistakes = 0;
    for (size_t i: _component[component_idx])
    {
        if (_current[p.mapping()[i]] != _solution[i])  {
            ctMistakes++;
		}
    }
    return ctMistakes;
}

template <std::size_t Size>
int Puzzle<Size>::countMismatches(const Permutation<Size>& p, std::vector<int> component_idxs) const {
    // return countMismatches(p);

    int ctMistakes = 0;
    for (int component_idx : component_idxs) {
        for (size_t i : _component[component_idx])
        {
            if (_current[p.mapping()[i]] != _solution[i]) {
                ctMistakes++;
            }
        }
    }
    return ctMistakes;
}

template <std::size_t Size>
bool Puzzle<Size>::admissablePermutation(const Permutation<Size>& p, int component_idx) const {
    for (size_t i : _component[component_idx])
    {
        if (p.mapping()[i] != i) {
            return true;
        }
    }
    return false;
}

template <std::size_t Size>
bool Puzzle<Size>::admissablePermutation(const Permutation<Size>& p, std::vector<int> component_idxs) const {
    for (int component_idx : component_idxs)
    {
        for (size_t i : _component[component_idx])
        {
            if (p.mapping()[i] != i)
            {
                return true;
            }
        }
    }
    return false;
}


template <std::size_t Size>
move_ids_t Puzzle<Size>::permutations() const {
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
bool Puzzle<Size>::isSolution(const mapping_t<Size>& mapping) const {
    int ct_mismatches = 0;
    for (size_t i = 0; i < Size; ++i) {
        if (_initial[mapping[i]] != _solution[i] && ++ct_mismatches > _num_wildcards) {
            return false;
		}
	}
    return (ct_mismatches <= _num_wildcards);
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
unsigned char Puzzle<Size>::operator[](size_t index) const {
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

std::string red(std::string str) {
    return "\033[1;31m" + str + "\033[0m";
}

std::string green(std::string str) {
    return "\033[1;32m" + str + "\033[0m";
}

std::string blue(std::string str) {
	return "\033[1;34m" + str + "\033[0m";
}

template <std::size_t Size>
std::ostream& operator<<(std::ostream& os, const Puzzle<Size>& puzzle) {
    os << "----------\n"
        << puzzle._id << ": " << puzzle._type << " [" << puzzle._num_wildcards << "]\n"
        << concatenate_vector(puzzle._initial) << "\n";

        for (size_t i = 0; i < puzzle._solution.size(); ++i) {
            if (puzzle._solution[i] == puzzle._current[i]) {
                os << green(puzzle._current[i]);
            }
            else {
                os << red(puzzle._current[i]);
            }
        }
        for (size_t i = 0; i < puzzle._solution.size(); ++i) {
            if (puzzle._solution[i] == puzzle._current[i]) {
                os << " ";
            }
            else {
                os << blue(puzzle._solution[i]);
            }
        }
    os << puzzle.submission() << "\n"
        << "----------";
    return os;
}

