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

std::vector<unsigned short> getInverse(const std::vector<unsigned short>& permutation) {
    std::vector<std::pair<unsigned short, unsigned short>> indexedPermutation;
    for (unsigned short i = 0; i < permutation.size(); ++i) {
        indexedPermutation.emplace_back(i, permutation[i]);
    }
    std::sort(indexedPermutation.begin(), indexedPermutation.end(), [](const auto& a, const auto& b) {
        return a.second < b.second;
    });

    std::vector<unsigned short> inverse;
    for (const auto& pair : indexedPermutation) {
        inverse.push_back(pair.first);
    }

    return inverse;
}

Permutation::Permutation(const std::vector<unsigned short>& mapping, const std::vector<unsigned char>& move_ids)
    : _mapping(mapping), _move_ids(move_ids) {}

Permutation::Permutation(const std::vector<unsigned short>& mapping, const unsigned char move_id)
    : _mapping(mapping), _move_ids(std::vector<unsigned char> {move_id}) {}

Permutation::Permutation(int size) {
    _mapping.reserve(size);
    for (unsigned short i = 0; i < size; ++i) {
        _mapping.push_back(i);
    }
    _move_ids = std::vector<unsigned char>();
}

Permutation::Permutation() {
    _mapping = std::vector<unsigned short>();
    _move_ids = std::vector<unsigned char>();
}

Permutation Permutation::from(int start, const std::vector<Permutation>& puzzle_info) {
    Permutation p((int)puzzle_info[0].size());
    for (int i = start; i < length(); ++i) {
		p = p * puzzle_info[_move_ids[i]];
	}
    return p;
}


Permutation Permutation::until(int end, const std::vector<Permutation>& puzzle_info) {
    Permutation p((int)puzzle_info[0].size());
    std::cout << "End: " << end << ", length " << length() << std::endl;
    for (int i = 0; i < end; ++i) {
        p = p * puzzle_info[_move_ids[i]];
    }
    return p;
}

const std::vector<unsigned char>& Permutation::move_ids() const {
    return _move_ids;
}

std::vector<Permutation> Permutation::split(const std::vector<Permutation>& puzzle_info) const {
    std::vector<Permutation> result;
    for (const auto& move_id : _move_ids) {
        result.push_back(puzzle_info[move_id]);
    }
    return result;
}

const std::vector<unsigned short>& Permutation::mapping() const {
    return _mapping;
}

Permutation Permutation::operator*(const Permutation& other) const {
    std::vector<unsigned short> result_mapping;
    result_mapping.reserve(_mapping.size());
    for (int i : other._mapping) {
        result_mapping.push_back(_mapping[i]);
    }
    std::vector<unsigned char> result_move_ids = _move_ids;
    result_move_ids.insert(result_move_ids.end(), other._move_ids.begin(), other._move_ids.end());
    return Permutation(result_mapping, result_move_ids);
}


int Permutation::size() const {
    return (int)_mapping.size();
}

int Permutation::length() const {
    return (int)_move_ids.size();
}

bool Permutation::operator<(const Permutation& other) const {
    if (length() < other.length()) {
        return true;
    } else {
        return false;
    }
}

bool Permutation::operator<=(const Permutation& other) const {
    if (length() <= other.length()) {
        return true;
    } else {
        return false;
    }
}

std::ostream& operator<<(std::ostream& os, const Permutation& permutation) {
    os << (int)permutation._move_ids[0];
    for (int i = 1; i < permutation._move_ids.size(); ++i) {
        os << "." << (int)permutation._move_ids[i];
	}
    return os;
 }

bool Permutation::operator==(const Permutation& other) const {
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


Puzzle::Puzzle(int id, const std::string& puzzle_type, const std::string& solution, const std::string& initial, int num_wildcards)
    : _id(id), _type(puzzle_type), _solution(splitSolution(solution)), _initial(splitSolution(initial)), _current(splitSolution(initial)), _num_wildcards(num_wildcards) {
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

Puzzle::Puzzle(const std::string& input_line)
{
    std::vector<std::string> line = split_string(input_line, ',');
    _id = std::stoi(line[0]);
    _type = line[1];
    _solution = split_string(line[2], ';');
    _initial = split_string(line[3], ';');
    _current = split_string(line[3], ';');
    _num_wildcards = std::stoi(line[4]);
}

void Puzzle::initializeMoveList(const std::unordered_map<std::string, std::vector<unsigned short>>& allowed_moves) {
    for (const auto& entry : allowed_moves) {
        _allowed_moves[entry.first] = entry.second;
        _allowed_moves["-" + entry.first] = getInverse(entry.second);
    }
}

std::vector<std::string> Puzzle::allowedMoveIds() const {
    std::vector<std::string> result;
    for (const auto& entry : _allowed_moves) {
        result.push_back(entry.first);
    }
    return result;
}

std::vector<std::string> Puzzle::randomSolution(size_t size) const {
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

Puzzle& Puzzle::permutate(const std::string& move_id) {
    _permutations.push_back(move_id);
    const std::vector<unsigned short>& permutation = _allowed_moves.at(move_id);
    //std::cout << "Permutation: " << move_id << ", size: " << permutation.size() << std::endl;
    for (size_t i = 0; i < permutation.size(); ++i) {
        //std::cout << i << " " << permutation[i] << std::endl;
        //std::cout << i << " " << _current[i] << std::endl;
        _current[i] = _current[permutation[i]];
    }
    return *this;
}

Puzzle& Puzzle::fullPermutation(const std::vector<std::string>& permutation_list) {
    for (const auto& move_id : permutation_list) {
        permutate(move_id);
    }
    return *this;
}

Puzzle Puzzle::clone() const {
    Puzzle cloned_puzzle(_id, _type, "", "", _num_wildcards);
    cloned_puzzle._solution = _solution;
    cloned_puzzle._current = _current;
    cloned_puzzle._initial = _initial;
    cloned_puzzle._allowed_moves = _allowed_moves;
    return cloned_puzzle;
}

int Puzzle::score() const {
    return 2 * (int)_current.size() * std::max(0, countMismatches() - _num_wildcards) + (int)_current.size();
}

int Puzzle::countMismatches() const {
    int ctMistakes = 0;
    for (size_t i = 0; i < _solution.size(); i++)
    {
        if (_solution[i] != _current[i]) {
            ctMistakes++;
		}
    }
    return ctMistakes;
}

std::vector<std::string> Puzzle::permutations() const {
    return _permutations;
}

const std::string& Puzzle::type() const {
    return _type;
}

bool Puzzle::isSolved() const {
    return countMismatches() <= getNumWildcards();
}

std::string Puzzle::submission() const {
    std::string result = std::to_string(_id) + ",";
    for (const auto& move_id : _permutations) {
        result += move_id + ".";
    }
    return result;
}

size_t Puzzle::size() const {
    return _solution.size();
}

const std::string& Puzzle::operator[](size_t index) const {
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

std::ostream& operator<<(std::ostream& os, const Puzzle& puzzle) {
    os << "----------\n"
        << puzzle._id << ": " << puzzle._type << " [" << puzzle._num_wildcards << "]\n"
        << concatenate_vector(puzzle._initial) << "\n"
        << concatenate_vector(puzzle._current) << "[" << puzzle.score() << "]\n"
        << concatenate_vector(puzzle._solution) << "\n"
        << puzzle.submission() << "\n"
        << "----------";
    return os;
}

