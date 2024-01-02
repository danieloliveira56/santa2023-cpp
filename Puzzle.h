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



//#ifdef PUZZLE_TYPE "cube_2/2/2"
//#define PUZZLE_SIZE 24
//#define PUZZLE_TYPE "cube_3/3/3"
//#define PUZZLE_SIZE 54
//#define PUZZLE_TYPE "cube_4/4/4"
//#define PUZZLE_SIZE 96
//#define PUZZLE_TYPE "cube_5/5/5"
//#define PUZZLE_SIZE 150
//#define PUZZLE_TYPE "cube_6/6/6"
//#define PUZZLE_SIZE 216
//#define PUZZLE_TYPE "cube_7/7/7"
//#define PUZZLE_SIZE 294
//#define PUZZLE_TYPE "cube_8/8/8"
//#define PUZZLE_SIZE 384
//#define PUZZLE_TYPE "cube_9/9/9"
//#define PUZZLE_SIZE 486
//#define PUZZLE_TYPE "cube_10/10/10"
//#define PUZZLE_SIZE 600
//#define PUZZLE_TYPE "cube_19/19/19"
//#define PUZZLE_SIZE 2166
//#define PUZZLE_TYPE "cube_33/33/33"
//#define PUZZLE_SIZE 6534
//#define PUZZLE_TYPE "wreath_6/6"
//#define PUZZLE_SIZE 10
//#define PUZZLE_TYPE "wreath_7/7"
//#define PUZZLE_SIZE 12
//#define PUZZLE_TYPE "wreath_12/12"
//#define PUZZLE_SIZE 22
//#define PUZZLE_TYPE "wreath_21/21"
//#define PUZZLE_SIZE 40
//#define PUZZLE_TYPE "wreath_33/33"
//#define PUZZLE_SIZE 64
#define PUZZLE_TYPE "wreath_100/100"
#define PUZZLE_SIZE 198
//#define PUZZLE_TYPE "globe_1/8"
//#define PUZZLE_SIZE 32
//#define PUZZLE_TYPE "globe_1/16"
//#define PUZZLE_SIZE 64
//#define PUZZLE_TYPE "globe_2/6"
//#define PUZZLE_SIZE 36
//#define PUZZLE_TYPE "globe_3/4"
//#define PUZZLE_SIZE 32
//#define PUZZLE_TYPE "globe_6/4"
//#define PUZZLE_SIZE 56
//#define PUZZLE_TYPE "globe_6/8"
//#define PUZZLE_SIZE 112
//#define PUZZLE_TYPE "globe_6/10"
//#define PUZZLE_SIZE 140
//#define PUZZLE_TYPE "globe_3/33"
//#define PUZZLE_SIZE 264
//#define PUZZLE_TYPE "globe_8/25"
//#define PUZZLE_SIZE 450


template <std::size_t Size>
using mapping_t = std::array<unsigned char, Size>;
typedef std::vector<unsigned char> move_ids_t;

template <std::size_t Size>
mapping_t<Size> getInverse(const mapping_t<Size>& mapping);

template <std::size_t Size>
class Puzzle {
public:
    Puzzle(int id, const std::string& puzzle_type, const std::string& solution, const std::string& initial, int num_wildcards);
    Puzzle(const std::string& input_line);
    Puzzle();

    void initializeMoveList(const std::unordered_map<std::string, mapping_t<Size>>& allowed_moves);
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
    std::vector<std::string> getInitial() const { return _initial; };
    std::vector<std::string> getSolution() const { return _solution; };

private:
    int _id;
    std::string _type;
    std::vector<std::string> _solution;
    std::vector<std::string> _initial;
    std::vector<std::string> _current;
    int _num_wildcards;
    std::vector<std::string> _permutations;
    std::unordered_map<std::string, mapping_t<Size>> _allowed_moves;
};

template <std::size_t Size>
class Permutation {
public:
    Permutation(const mapping_t<Size>& mapping, const move_ids_t& move_ids);
    Permutation(const mapping_t<Size>& mapping, const unsigned char move_id);
    Permutation();

    Permutation until(int, const std::vector<Permutation>&);
    Permutation from(int, const std::vector<Permutation>&);

    const move_ids_t& move_ids() const;
    std::vector<Permutation> split(const  std::vector<Permutation>&) const;
    const mapping_t<Size>& mapping() const;
    Permutation operator*(const Permutation&) const;
    Permutation inverse() const;

    int size() const { return (int)_mapping.size(); };
    int length() const { return (int)_move_ids.size(); }
    bool operator<(const Permutation&) const;
    bool operator<=(const Permutation&) const;
    friend std::ostream& operator<<(std::ostream& os, const Permutation&);
    bool operator==(const Permutation&) const;
private:
    mapping_t<Size> _mapping;
    move_ids_t _move_ids;
};