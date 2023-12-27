#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <random>
#include <json/json.h>  // Assuming you have a JSON library, like jsoncpp

std::vector<unsigned short> getInverse(const std::vector<unsigned short>& mapping);

class Puzzle {
public:
    Puzzle(int id, const std::string& puzzle_type, const std::string& solution, const std::string& initial, int num_wildcards);
    Puzzle(const std::string& input_line);

    void initializeMoveList(const std::unordered_map<std::string, std::vector<unsigned short>>& allowed_moves);
    std::vector<std::string> allowedMoveIds() const;
    std::vector<std::string> randomSolution(size_t size) const;
    Puzzle& permutate(const std::string& move_id);
    Puzzle& fullPermutation(const std::vector<std::string>& permutation_list);
    Puzzle clone() const;
    int score() const;
    int countMismatches() const;
    std::vector<std::string> permutations() const;
    const std::string& type() const;
    bool isSolved() const;
    std::string submission() const;
    size_t size() const;
    const std::string& operator[](size_t index) const;
    friend std::ostream& operator<<(std::ostream& os, const Puzzle& puzzle);
    int getId() const { return _id; };
    int getNumWildcards() const { return _num_wildcards; };

private:
    int _id;
    std::string _type;
    std::vector<std::string> _solution;
    std::vector<std::string> _initial;
    std::vector<std::string> _current;
    int _num_wildcards;
    std::vector<std::string> _permutations;
    std::unordered_map<std::string, std::vector<unsigned short>> _allowed_moves;
};


class Permutation {
public:
    Permutation(const std::vector<unsigned short>& mapping, const std::vector<unsigned char>& move_ids);
    Permutation(const std::vector<unsigned short>& mapping, const unsigned char move_id);
    Permutation(int);
    Permutation();

    Permutation until(int, const std::vector<Permutation>&);
    Permutation from(int, const std::vector<Permutation>&);

    const std::vector<unsigned char>& move_ids() const;
    std::vector<Permutation> split(const  std::vector<Permutation>&) const;
    const std::vector<unsigned short>& mapping() const;
    Permutation operator*(const Permutation&) const;

    int size() const;
    int length() const;
    bool operator<(const Permutation&) const;
    bool operator<=(const Permutation&) const;
    friend std::ostream& operator<<(std::ostream& os, const Permutation& );
    bool operator==(const Permutation&) const;
private:
    std::vector<unsigned short> _mapping;
    std::vector<unsigned char> _move_ids;
};