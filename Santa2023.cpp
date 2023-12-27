#include <iostream>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <random>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <json/json.h>  // Assuming you have a JSON library, like jsoncpp
#include "Puzzle.h"  // Assuming you have a Puzzle class defined
#include <boost/algorithm/string.hpp>
using boost::replace_all;
using namespace std;

#include <functional>

namespace std {
    template<> struct hash<std::vector<unsigned short>> {
        size_t operator()(const std::vector<unsigned short>& v) const {
            std::hash<unsigned short> hasher;

            size_t hashValue = 0;
            for (const auto& element : v) {
                // Combine hash values using a simple hash-combining technique
                hashValue ^= hasher(element) + 0x9e3779b9 + (hashValue << 6) + (hashValue >> 2);
            }

            return hashValue;
        }
    };
}


vector<Puzzle> readPuzzles(const string& filename) {
    ifstream file(filename);
    cout << "Reading puzzles from " << filename << endl;
    if (!file.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        exit(EXIT_FAILURE);
    }

    vector<Puzzle> puzzles;
    string line;
    getline(file, line);  // Skip the first line
    while (getline(file, line)) {
        puzzles.emplace_back(line);
    }

    file.close();
    return puzzles;
}

unordered_map<string, pair<vector<Permutation>, unordered_map<string, unsigned char>>> readPuzzleInfo(const string& filename) {
    ifstream file(filename);
    cout << "Reading puzzle info from " << filename << endl;

    if (!file.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        exit(EXIT_FAILURE);
    }

    unordered_map<string, pair<vector<Permutation>, unordered_map<string, unsigned char>>> puzzleInfo;
    unordered_map<string, unordered_map<string, unsigned char>> all_move_id_map;
    string line;
    getline(file, line);  // Skip the first line
    while (getline(file, line)) {
        istringstream iss(line);
        string type, moves;
        getline(iss, type, ',');
        getline(iss, moves);
        replace_all(moves, "\"", ""); 
        replace_all(moves, "'", "\"");  
        Json::Value jsonMoves;
        Json::Reader reader;
        reader.parse(moves, jsonMoves);
        unordered_map<string, unsigned char> move_id_map;
        puzzleInfo[type] = pair<vector<Permutation>, unordered_map<string, unsigned char>>(vector<Permutation>(), unordered_map<string, unsigned char>());
        unsigned char j = 0;
        for (const auto& move_id : jsonMoves.getMemberNames()) {
            vector<unsigned short> mapping;
            for (const auto& i : jsonMoves[move_id]) {
                mapping.push_back((unsigned short)i.asInt());
			}
            puzzleInfo[type].first.emplace_back(mapping, vector<unsigned char>({j}));
            puzzleInfo[type].second[move_id] = j;
            j++;
            vector<unsigned short> inverse = getInverse(mapping);
            puzzleInfo[type].first.emplace_back(inverse, vector<unsigned char>({j}));
            puzzleInfo[type].second["-" + move_id] = j;
            j++;
        }
    }

    file.close();
    return puzzleInfo;
}

vector<Permutation> readSolution(
    string filename,
    vector<Puzzle> puzzles,
    unordered_map <string, pair<vector<Permutation>, unordered_map<string, unsigned char>>> puzzle_info
) {
    cout << "Reading solution from " << filename << endl;

    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        exit(EXIT_FAILURE);
    }

    vector<Permutation> solution;
    string line;
    getline(file, line);  // Skip the first line
    while (getline(file, line)) {
        istringstream iss(line);
        string id, move_ids;
        getline(iss, id, ',');
        getline(iss, move_ids);
        Puzzle puzzle = puzzles[stoi(id)];
        vector<Permutation> allowed_moves = puzzle_info[puzzle.type()].first;
        unordered_map<string, unsigned char> move_id_map = puzzle_info[puzzle.type()].second;

        istringstream iss2(move_ids);
        string move_id;
        Permutation p = Permutation((int)puzzle.size());
        while (getline(iss2, move_id, '.')) {
            p = p * allowed_moves[(int)move_id_map[move_id]];
		}
		solution.push_back(p);
    }
 //   for (int i = 0; i < solution.size(); ++i) {
	//	cout << "Puzzle " << i << ": " << solution[i] << endl;
 //       cout << "Size: " << solution[i].size() << endl;
	//}
    return solution;
}

unordered_map<vector<unsigned short>, Permutation> getPermutations(const vector<Permutation>& allowed_moves, const int depth) {
	unordered_map<vector<unsigned short>, Permutation> permutation_map;
    int size = allowed_moves[0].size();
    vector<unsigned short> identity(size);
    cout << "Getting Permutations" << endl;

    for (unsigned short i = 0; i < size; ++i) {
        identity[i] = i;
    }
    Permutation identity_permutation(size);
    vector<Permutation> previous_permutations({ identity_permutation });
    permutation_map[identity] = identity_permutation;
    for (int i = 0; i < depth; ++i) {
        vector<Permutation> new_permutations;
        for (const auto& entry : previous_permutations) {
            for (const auto& move : allowed_moves) {
				Permutation new_permutation = entry * move;
                if (permutation_map.find(new_permutation.mapping()) == permutation_map.end()) {
					permutation_map[new_permutation.mapping()] = new_permutation;
					new_permutations.push_back(new_permutation);
				}
			}
		}
        previous_permutations = new_permutations;
	}
	return permutation_map;
}


void fast_identities(
    const vector<Puzzle>& puzzles, 
    vector<Permutation>& solution,
    const unordered_map<string, pair<vector<Permutation>, unordered_map<string, unsigned char>>>& puzzle_info
) {
    cout << "Running fast identities" << endl;
    int depth = 3;
    string curr_type = puzzles[0].type();
    vector<Permutation> allowed_moves = puzzle_info.at(curr_type).first;
    unordered_map<vector<unsigned short>, Permutation> permutation_map = getPermutations(allowed_moves, depth);
    cout << "Got permutations for type " << curr_type << ", size: " << permutation_map.size() << endl;
    for (int i = 0; i < puzzles.size(); ++i) {
        //cout << "Puzzle " << puzzles[i].getId() << " (" << puzzles[i].type() << "), size: " << solution[i].size() << endl;
        int initial_len = solution[i].length();
        if (puzzles[i].type() != curr_type) {
            curr_type = puzzles[i].type();
            allowed_moves = puzzle_info.at(curr_type).first;
            permutation_map = getPermutations(allowed_moves, depth);
            cout << "Got permutations for type " << curr_type << ", size: " << permutation_map.size() << endl;
        } 
        vector<Permutation> permutation_list = solution[i].split(allowed_moves);
        for (int j = 0; j < permutation_list.size()-1; ++j) {
            Permutation p = permutation_list[j];
            for (int k = j + 1; k < permutation_list.size(); ++k) {
                cout <<  "\r";
                p = p * permutation_list[k];
                if (permutation_map.find(p.mapping()) != permutation_map.end()) {
                    if (permutation_map[p.mapping()] < p) {
                        // Found a shorter permutation and can replace
                        Permutation new_solution = solution[i].until(j, allowed_moves) * permutation_map[p.mapping()] * solution[i].from(k+1, allowed_moves);
                        cout << "Searching puzzle " << puzzles[i].getId()
                             << "[" << puzzles[i].type() << "]"
                             << ", j=" << j 
                             << ", k=" << k 
                             << "/" << permutation_list.size() 
                             << " (" << initial_len << ")->(" << new_solution.length() << ")" 
                             << endl;
                        solution[i] = new_solution;
                        permutation_list = new_solution.split(allowed_moves);
                        j--;
                        break;
                    }
                }
            }
        }
        cout << "Puzzle " << i << " (" << initial_len << ")->(" << solution[i].length() << ")                                                                    " << endl;
        //if (i == 10) break;
    }
    cout << "Done" << endl;
}

int main(int argc, char** argv) {
    // Get first positional argument as filename
    cout << argc << endl;
    if (argc < 3) {
		cerr << "Usage: " << argv[0] << " <puzzle_filename> <puzzle_info_filename>" << endl;
		exit(EXIT_FAILURE);
	}
    vector<Puzzle> puzzles = readPuzzles(argv[2]);
    unordered_map <string, pair<vector<Permutation>, unordered_map<string, unsigned char>>> puzzle_info = readPuzzleInfo(argv[3]);
    vector<Permutation> solution = readSolution(argv[1], puzzles, puzzle_info);

    fast_identities(puzzles, solution, puzzle_info);

    unordered_map <string, unordered_map<unsigned char, string>> move_ids_by_type;
    for (const auto& entry : puzzle_info) {
		string type = entry.first;
		unordered_map<string, unsigned char> move_id_map = entry.second.second;
        for (const auto& move_id : move_id_map) {
			move_ids_by_type[type][move_id.second] = move_id.first;
		}
	}

    // Write solution to file
    int solution_length = 0;
    for (const auto& p : solution) {
		solution_length += p.length();
    }
    // Filename with solution length
    string filename = "solution_" + to_string(solution_length) + ".csv";
    cout << "Writing solution to " << filename << endl;
    ofstream file(filename);
    if (!file.is_open()) {
		cerr << "Error opening file: solution.txt" << endl;
		exit(EXIT_FAILURE);
	}
    file << "id,move_ids" << endl;
    for (int i = 0; i < solution.size(); ++i) {
		file << puzzles[i].getId() << ",";
        string puzzle_type = puzzles[i].type();
        vector<unsigned char> move_ids = solution[i].move_ids();
        file << move_ids_by_type[puzzle_type][(int)move_ids[0]];
        for (int j = 1; j < move_ids.size(); ++j) {
            file << "." << move_ids_by_type[puzzle_type][(int)move_ids[j]];
        }
        file << endl;
    }
    return 0;
}
