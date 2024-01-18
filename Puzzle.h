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

#define PUZZLE_CUBE_2_2_2 0
#define PUZZLE_CUBE_3_3_3 1
#define PUZZLE_CUBE_4_4_4 2
#define PUZZLE_CUBE_5_5_5 3
#define PUZZLE_CUBE_6_6_6 4
#define PUZZLE_CUBE_7_7_7 5
#define PUZZLE_CUBE_8_8_8 6
#define PUZZLE_CUBE_9_9_9 7
#define PUZZLE_CUBE_10_10_10 8
#define PUZZLE_CUBE_19_19_19 9
#define PUZZLE_CUBE_33_33_33 10
#define PUZZLE_WREATH_6_6 11
#define PUZZLE_WREATH_7_7 12
#define PUZZLE_WREATH_12_12 13
#define PUZZLE_WREATH_21_21 14
#define PUZZLE_WREATH_33_33 15
#define PUZZLE_WREATH_100_100 16
#define PUZZLE_GLOBE_1_8 17
#define PUZZLE_GLOBE_1_16 18
#define PUZZLE_GLOBE_2_6 19
#define PUZZLE_GLOBE_3_4 20
#define PUZZLE_GLOBE_6_4 21
#define PUZZLE_GLOBE_6_8 22
#define PUZZLE_GLOBE_6_10 23
#define PUZZLE_GLOBE_3_33 24
#define PUZZLE_GLOBE_8_25 25
    
#define PUZZLE_TYPE PUZZLE_CUBE_19_19_19

#if PUZZLE_TYPE == PUZZLE_CUBE_2_2_2
#define PUZZLE_SIZE 24
#define PUZZLE_TYPE_NAME "cube_2/2/2"
#elif PUZZLE_TYPE == PUZZLE_CUBE_3_3_3
#define PUZZLE_SIZE 54
#define PUZZLE_TYPE_NAME "cube_3/3/3"
#elif PUZZLE_TYPE == PUZZLE_CUBE_4_4_4
#define PUZZLE_SIZE 96
#define PUZZLE_TYPE_NAME "cube_4/4/4"
#elif PUZZLE_TYPE == PUZZLE_CUBE_5_5_5
#define PUZZLE_SIZE 150
#define PUZZLE_TYPE_NAME "cube_5/5/5"
#elif PUZZLE_TYPE == PUZZLE_CUBE_6_6_6
#define PUZZLE_SIZE 216
#define PUZZLE_TYPE_NAME "cube_6/6/6"
#elif PUZZLE_TYPE == PUZZLE_CUBE_7_7_7
#define PUZZLE_SIZE 294
#define PUZZLE_TYPE_NAME "cube_7/7/7"
#elif PUZZLE_TYPE == PUZZLE_CUBE_8_8_8
#define PUZZLE_SIZE 384
#define PUZZLE_TYPE_NAME "cube_8/8/8"
#elif PUZZLE_TYPE == PUZZLE_CUBE_9_9_9
#define PUZZLE_SIZE 486
#define PUZZLE_TYPE_NAME "cube_9/9/9"
#elif PUZZLE_TYPE == PUZZLE_CUBE_10_10_10
#define PUZZLE_SIZE 600
#define PUZZLE_TYPE_NAME "cube_10/10/10"
#elif PUZZLE_TYPE == PUZZLE_CUBE_19_19_19
#define PUZZLE_SIZE 2166
#define PUZZLE_TYPE_NAME "cube_19/19/19"
#elif PUZZLE_TYPE == PUZZLE_CUBE_33_33_33
#define PUZZLE_SIZE 6534
#define PUZZLE_TYPE_NAME "cube_33/33/33"
#elif PUZZLE_TYPE == PUZZLE_WREATH_6_6
#define PUZZLE_SIZE 10
#define PUZZLE_TYPE_NAME "wreath_6/6"
#elif PUZZLE_TYPE == PUZZLE_WREATH_7_7
#define PUZZLE_SIZE 12
#define PUZZLE_TYPE_NAME "wreath_7/7"
#elif PUZZLE_TYPE == PUZZLE_WREATH_12_12
#define PUZZLE_SIZE 22
#define PUZZLE_TYPE_NAME "wreath_12/12"
#elif PUZZLE_TYPE == PUZZLE_WREATH_21_21
#define PUZZLE_SIZE 40
#define PUZZLE_TYPE_NAME "wreath_21/21"
#elif PUZZLE_TYPE == PUZZLE_WREATH_33_33
#define PUZZLE_SIZE 64
#define PUZZLE_TYPE_NAME "wreath_33/33"
#elif PUZZLE_TYPE == PUZZLE_WREATH_100_100
#define PUZZLE_SIZE 198
#define PUZZLE_TYPE_NAME "wreath_100/100"
#elif PUZZLE_TYPE == PUZZLE_GLOBE_1_8
#define PUZZLE_SIZE 32
#define PUZZLE_TYPE_NAME "globe_1/8"
#elif PUZZLE_TYPE == PUZZLE_GLOBE_1_16
#define PUZZLE_SIZE 64
#define PUZZLE_TYPE_NAME "globe_1/16"
#elif PUZZLE_TYPE == PUZZLE_GLOBE_2_6
#define PUZZLE_SIZE 36
#define PUZZLE_TYPE_NAME "globe_2/6"
#elif PUZZLE_TYPE == PUZZLE_GLOBE_3_4
#define PUZZLE_SIZE 32
#define PUZZLE_TYPE_NAME "globe_3/4"
#elif PUZZLE_TYPE == PUZZLE_GLOBE_6_4
#define PUZZLE_SIZE 56
#define PUZZLE_TYPE_NAME "globe_6/4"
#elif PUZZLE_TYPE == PUZZLE_GLOBE_6_8
#define PUZZLE_SIZE 112
#define PUZZLE_TYPE_NAME "globe_6/8"
#elif PUZZLE_TYPE == PUZZLE_GLOBE_6_10
#define PUZZLE_SIZE 140
#define PUZZLE_TYPE_NAME "globe_6/10"
#elif PUZZLE_TYPE == PUZZLE_GLOBE_3_33
#define PUZZLE_SIZE 264
#define PUZZLE_TYPE_NAME "globe_3/33"
#elif PUZZLE_TYPE == PUZZLE_GLOBE_8_25
#define PUZZLE_SIZE 450
#define PUZZLE_TYPE_NAME "globe_8/25"
#endif

// Only puzzle of type globe_3/33 and globe_8/25 have more than 255 moves, requiring unsigned short
template <std::size_t Size>
#if PUZZLE_SIZE > 255
using mapping_t = std::array<unsigned short, Size>;
#else
using mapping_t = std::array<unsigned char, Size>;
#endif

template <std::size_t Size>
using solution_t = std::array<unsigned short, Size>;

// No puzzle has more than 255 moves, so we can use unsigned char
typedef std::vector<unsigned char> move_ids_t;

template <std::size_t Size>
mapping_t<Size> getInverse(const mapping_t<Size>& mapping);

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
    mapping_t<Size> operator*(const mapping_t<Size>&) const;

    Permutation inverse() const;
    mapping_t<Size> inverse_map() const;


    int size() const { return (int)_mapping.size(); };
    int length() const { return (int)_move_ids.size(); }
    bool operator<(const Permutation&) const;
    bool operator<=(const Permutation&) const;
    friend std::ostream& operator<<(std::ostream& os, const Permutation&);
    bool operator==(const Permutation&) const;
    bool isInverse(const Permutation&) const;
private:
    mapping_t<Size> _mapping;
    move_ids_t _move_ids;
};

template <std::size_t Size>
class Puzzle {
public:
    Puzzle(int id, const std::string& puzzle_type, const std::string& solution, const std::string& initial, int num_wildcards);
    Puzzle(const std::string& input_line);
    Puzzle();

    std::vector<std::string> allowedMoveIds() const;
    std::vector<std::string> randomSolution(size_t size) const;
    Puzzle clone() const;
    int score() const;
    int countMismatches() const;
    int countMismatches(int) const;

    int countMismatches(const mapping_t<Size>&) const;
    int countMismatches(const Permutation<Size>&) const;

    int countMismatches(const Permutation<Size>&, int) const;
    int countMismatches(const mapping_t<Size >&, int) const;

    int countMismatches(const mapping_t<Size>&, std::vector<int>) const;
    int countMismatches(const Permutation<Size>&, std::vector<int>) const;

    bool admissablePermutation(const Permutation<Size>&, int) const;
    bool admissablePermutation(const Permutation<Size>&, std::vector<int>) const;

    move_ids_t permutations() const;
    const std::string& type() const;
    bool isSolved() const;
    bool isSolution(const mapping_t<Size>& mapping) const;
    std::string submission() const;
    size_t size() const;
    size_t current_solution_length() const { return _permutations.size(); };
    unsigned char operator[](size_t index) const;
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
    move_ids_t _permutations;
    std::unordered_map<std::string, mapping_t<Size>> _allowed_moves;
    std::vector<std::vector<int>> _component;
};