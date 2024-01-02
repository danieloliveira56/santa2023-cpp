#include <iostream>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <random>
#include <cmath>
#include <iomanip>

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
#include "windows.h"
#include "psapi.h"


namespace std {
    template <std::size_t Size> struct hash<mapping_t<Size>> {
        size_t operator()(const mapping_t<Size>& v) const {
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

template <std::size_t Size>
unordered_map<int, Puzzle<Size>> readPuzzles(const string& filename, const string& puzzle_type) {
    ifstream file(filename);
    cout << "Reading puzzles from " << filename << endl;
    if (!file.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        exit(EXIT_FAILURE);
    }

    unordered_map<int, Puzzle<Size>> puzzles;
    string line;
    getline(file, line);  // Skip the first line
    while (getline(file, line)) {
        istringstream iss(line);
        string id, type, solution, initial, num_wildcards;
        getline(iss, id, ',');
        getline(iss, type, ',');
        if (type != puzzle_type) {
            continue;
        }
        getline(iss, solution, ',');
        replace_all(solution, "\"", "");  // Remove quotes
        replace_all(solution, "'", "\"");  // Replace single quotes with double quotes
        getline(iss, initial, ',');
        replace_all(initial, "\"", "");  // Remove quotes
        replace_all(initial, "'", "\"");  // Replace single quotes with double quotes
        getline(iss, num_wildcards);
        Puzzle<Size> puzzle(stoi(id), type, solution, initial, stoi(num_wildcards));
        puzzles[puzzle.getId()] = puzzle;
    }

    file.close();
    cout << "Read " << puzzles.size() << " puzzles" << endl;
    return puzzles;
}

template <std::size_t Size>
pair<vector<Permutation<Size>>, unordered_map<string, unsigned char>> readPuzzleInfo(const string& filename, const string& puzzle_type) {
    ifstream file(filename);
    cout << "Reading puzzle info from " << filename << endl;

    if (!file.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        exit(EXIT_FAILURE);
    }

    vector<Permutation<Size>> allowed_moves;
    unordered_map<string, unsigned char> move_id_map;
    string line;
    getline(file, line);  // Skip the first line
    while (getline(file, line)) {
        istringstream iss(line);
        string type, moves;
        getline(iss, type, ',');
        if (type != puzzle_type) {
			continue;
		}
        getline(iss, moves);
        replace_all(moves, "\"", ""); 
        replace_all(moves, "'", "\"");  
        Json::Value jsonMoves;
        Json::Reader reader;
        reader.parse(moves, jsonMoves);
        unsigned char j = 0;
        for (const auto& move_id : jsonMoves.getMemberNames()) {
            mapping_t<Size> mapping;
            for (size_t i = 0; i < jsonMoves[move_id].size(); ++i) {
                mapping[i] = (unsigned short)jsonMoves[move_id][(int)i].asInt();
			}
            allowed_moves.emplace_back(mapping, move_ids_t({j}));
            move_id_map[move_id] = j;
            j++;
            mapping_t<Size> inverse = getInverse(mapping);
            if (inverse == mapping) continue;
            allowed_moves.emplace_back(inverse, move_ids_t({j}));
            move_id_map["-" + move_id] = j;
            j++;
        }
    }

    file.close();
    cout << "Read " << allowed_moves.size() << " allowed moves" << endl;
    return make_pair(allowed_moves, move_id_map);
}

template <std::size_t Size>
unordered_map<int, Permutation<Size>> readSolution(
    string filename,
    unordered_map<int, Puzzle<Size>> puzzles,
    vector<Permutation<Size>> allowed_moves,
    unordered_map<string, unsigned char > move_id_map,
    string puzzle_type = "cube_2/2/2"
) {
    cout << "Reading solution from " << filename << endl;

    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        exit(EXIT_FAILURE);
    }

    unordered_map<int, Permutation<Size>> solution;
    string line;
    getline(file, line);  // Skip the first line
    while (getline(file, line)) {
        istringstream iss(line);
        string id, move_ids;
        getline(iss, id, ',');
        getline(iss, move_ids);
        if (puzzles.find(stoi(id)) == puzzles.end()) {
			continue;
		}		
        Puzzle<Size> puzzle = puzzles[stoi(id)];

        istringstream iss2(move_ids);
        string move_id;
        Permutation<Size> p = Permutation<Size>();
        while (getline(iss2, move_id, '.')) {
            p = p * allowed_moves[(int)move_id_map[move_id]];
		}
		solution[stoi(id)] = p;
    }
    return solution;
}

template <std::size_t Size>
unordered_map<mapping_t<Size>, move_ids_t> getPermutations(
    const vector<Permutation<Size>>& allowed_moves, 
    const int depth
) {
    cout << "Getting Permutations of length " << Size << " generated by " << allowed_moves.size() << " moves..." << endl;

    Permutation<Size> identity_permutation = Permutation<Size>();
    unordered_map<mapping_t<Size>, move_ids_t> permutation_map{ {identity_permutation.mapping(), identity_permutation.move_ids()}};

    vector<mapping_t<Size>> moves_mappings;
    for (const auto& move : allowed_moves) {
		moves_mappings.push_back(move.mapping());
	}
    size_t buckets = allowed_moves.size();
    for (int i = 1; i < depth; ++i) {
        buckets *= (allowed_moves.size()-2*i);
    }
    cout << "Reserving  " << buckets << " buckets, estimated memory usage: " << buckets * (sizeof(mapping_t<Size>) + sizeof(move_ids_t)) / 1024.0 / 1024 << " MB" << endl;
    //permutation_map.reserve(buckets);

    PROCESS_MEMORY_COUNTERS_EX pmc;
    GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
    SIZE_T current_memory_usage = pmc.WorkingSetSize / 1024.0 / 1024;
    SIZE_T previous_memory_usage = current_memory_usage;

    cout << setw(5) << "Depth"
        << setw(15) << "Size"
        << setw(15) << "Increase_Factor"
        << setw(20) << "Memory_Usage(MB)"
        << setw(20) << "Next_Mem_Usage(MB)"
        << setw(15) << "Buckets"
        << setw(15) << "Load_Factor" << endl;

    cout << setw(5) << 0 
        << setw(15) << 1 
        << setw(15) << 0 
        << setw(20) << current_memory_usage
        << setw(20) << "?"
        << setw(15) << permutation_map.bucket_count() 
        << setw(15) << permutation_map.load_factor() << endl;

    size_t previous_size = 1;
    size_t previous_level_size = 1;
    for (int i = 1; i <= depth; ++i) {
        for (const auto& entry : permutation_map) {
            if (entry.second.size() != i-1) continue;
            for (int j = 0; j < moves_mappings.size(); ++j) {
                //cout << j << endl;
                mapping_t<Size> new_mapping = entry.first;
                //for (auto k : new_mapping) {
                //    cout << k << " ";
                //}
                //cout << endl;
                for (int k = 0; k < new_mapping.size(); ++k) {
					new_mapping[k] = entry.first[moves_mappings[j][k]];
				}
                //for (auto k : new_mapping) {
                //    cout << k << " ";
                //}
                //cout << endl;
                move_ids_t new_move_ids = entry.second;
                new_move_ids.push_back(j);
                new_move_ids.shrink_to_fit();
                permutation_map.insert({ new_mapping, new_move_ids });
			}
		}
        size_t current_size = permutation_map.size();
        size_t current_level_size = current_size - previous_size;
        double increase_factor = (double)current_level_size / previous_level_size;
            // << "Estimated memory usage: " << permutation_map.bucket_count() * (sizeof(mapping_t<Size>) + sizeof(move_ids_t)) / 1024.0 / 1024 / 1024 << " GB,"

        GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
        current_memory_usage = pmc.WorkingSetSize / 1024.0 / 1024;
        SIZE_T memory_increase = current_memory_usage - previous_memory_usage;
        SIZE_T estimated_next_level_memory_usage = current_memory_usage + memory_increase * increase_factor;

        cout << setw(5) << i
            << setw(15) << current_size
            << setw(15) << increase_factor
            << setw(20) << current_memory_usage
            << setw(20) << estimated_next_level_memory_usage
            << setw(15) << permutation_map.bucket_count()
            << setw(15) << permutation_map.load_factor() << endl;

        //permutation_map.reserve(current_size + increase_factor * current_level_size);

        if (i < depth && estimated_next_level_memory_usage > 80000) {
			cout << "Memory usage > 80000MB, stopping" << endl;
			break;
		}

        previous_memory_usage = current_memory_usage;
        previous_level_size = current_size;
        previous_size = current_size;
    }
    return permutation_map;
}

vector<int> get_base_n_vector(long long n, int base) {
    if (n == 0) {
		return vector<int>({ 0 });
	}
	vector<int> result;
	int i = 0;
	while (n > 0) {
		result.push_back(n % base);
		n /= base;
	}
    return result;
}

template <std::size_t Size>
void enumeration(
    const unordered_map<int, Puzzle<Size>>& puzzles,
    vector<Permutation<Size>>& solution,
    const unordered_map<string, pair<vector<Permutation<Size>>, unordered_map<string, unsigned char>>>& puzzle_info,
    string puzzle_type = "cube_2/2/2"
) {
    cout << "Running enumeration" << endl;
    cout << "Puzzle type: " << puzzle_type << endl;

    int puzzle_idx1, puzzle_idx2;
	puzzle_idx1 = 0;
    while (puzzles[puzzle_idx1].type() != puzzle_type) {
		puzzle_idx1++;
	}
	puzzle_idx2 = puzzle_idx1 + 1;
    while (puzzles[puzzle_idx2].type() == puzzle_type) {
		puzzle_idx2++;
	}	
    cout << "Puzzle indices: " << puzzle_idx1  << "-" << puzzle_idx2-1 << " (" << puzzle_idx2-puzzle_idx1 << ")" << endl;

    int max_length = 0;
    for (int i = puzzle_idx1; i < puzzle_idx2; ++i) {
        max_length = max(max_length, (int)solution[i].length());
    }
    cout << "Max length: " << max_length << endl;
       
    vector<Permutation> allowed_moves = puzzle_info.at(puzzle_type).first;

    int permutation_idx = -1;
    for (int p_length = 2; p_length < max_length; p_length++) {
        long long idx1 = pow(allowed_moves.size(), p_length - 1) + 1;
        long long idx2 = pow(allowed_moves.size(), p_length);
        cout << "Permutation length: " << p_length << ", indices: " << idx1 << "-" << idx2 << " (" << idx2 - idx1 + 1 << ")" << endl;
#pragma omp parallel for
        for (long long permutation_idx = idx1; permutation_idx < idx2; permutation_idx++)
        {
            vector<int> permutation_vector = get_base_n_vector(permutation_idx, allowed_moves.size());
            bool skip = false;
            for (int j = 0; j < permutation_vector.size()-1; ++j) {
                if (permutation_vector[j] % 2) {
                    // odd: example permutation_vector[j]=1 and permutation_vector[j+1]=0 are inverses
                    if (permutation_vector[j] == permutation_vector[j + 1] + 1) {
						skip = true;
					}
                }
                else {
                    // even: example permutation_vector[j]=0 and permutation_vector[j+1]=1 are inverses
                    if (permutation_vector[j] == permutation_vector[j + 1] - 1)
                    {
                        skip = true;
                    }
                }
            }
            if (skip) continue;

            Permutation<Size> p = allowed_moves[permutation_vector[0]];
            for (int j = 1; j < permutation_vector.size(); ++j) {
                p = p * allowed_moves[permutation_vector[j]];
            }
            for (int i = puzzle_idx1; i < puzzle_idx2; ++i) {
                if (p.length() < solution[i].length()) {
                    vector<string> initial_string = puzzles[i].getInitial();
                    vector<string> current_string(puzzles[i].size());
                    for (int j = 0; j < initial_string.size(); ++j) {
                        current_string[j] = initial_string[p.mapping()[j]];
                    }
                    if (current_string == puzzles[i].getSolution()) {
#pragma omp critical
                        {
                            cout << "Found solution for puzzle " << i << ": " << p << endl;
                            if (p.length() < solution[i].length()) {
                                solution[i] = p;
                                cout << "Puzzle " << i << " (" << solution[i].length() << ")->(" << p.length() << ")                 " << endl;
                            }
                        }
                    }
                }
            }
        }
    }
}

template <std::size_t Size>
void fast_identities(
    const unordered_map<int, Puzzle<Size>>& puzzles,
    unordered_map<int, Permutation<Size>>& solution,
    const vector<Permutation<Size>>& allowed_moves,
    int depth=-1
) {
    cout << "Running fast identities" << endl;
    string curr_type;
    unordered_map<mapping_t<Size>, move_ids_t> permutation_map;
    unordered_map<string, int> puzzle_type_depth{
        { "cube_2/2/2", 8},
        { "cube_3/3/3", 7},
        { "cube_4/4/4", 6},
        { "cube_5/5/5", 4},
        { "cube_6/6/6", 4},
        { "cube_7/7/7", 3},
        { "cube_8/8/8", 3},
        { "cube_9/9/9", 3},
        { "cube_10/10/10", 3},
        { "cube_19/19/19", 2},
        { "cube_33/33/33", 2},
        { "wreath_6/6", 16},
        { "wreath_7/7", 16},
        { "wreath_12/12", 16},
        { "wreath_21/21", 16},
        { "wreath_33/33", 16},
        { "wreath_100/100", 16},
        { "globe_1/8", 6},
        { "globe_1/16", 5},
        { "globe_2/6", 8},
        { "globe_3/4", 8},
        { "globe_6/4", 7},
        { "globe_6/8", 6},
        { "globe_6/10", 5},
        { "globe_3/33", 4},
        { "globe_8/25", 4},
    };


    for (auto& entry : puzzles) {
        //cout << "Puzzle " << puzzles[i].getId() << " (" << puzzles[i].type() << "), size: " << solution[i].size() << endl;
        int initial_len = solution[entry.first].length();
        if (entry.second.type() != curr_type) {
            curr_type = entry.second.type();
            int type_depth = depth;
            if (depth == -1) {
                type_depth = puzzle_type_depth[curr_type];
			}
            permutation_map = getPermutations<Size>(allowed_moves, type_depth);
            cout << "Got permutations for type " << curr_type << ", size: " << permutation_map.size() << endl;
        }
        vector<Permutation<Size>> permutation_list = solution[entry.first].split(allowed_moves);
        bool has_improvement = true;
        while (has_improvement) {
            has_improvement = false;
            size_t best_j, best_k = -1;
            size_t best_replacement = 0;
            move_ids_t best_permutation;
#pragma omp parallel for shared(has_improvement) // schedule(dynamic) reduction(min : best_replacement) // shared(permutation_list, permutation_map, solution, has_improvement)
            for (int j = 0; j < permutation_list.size() - 1; ++j) {
                if (has_improvement) continue;
                Permutation<Size> p = permutation_list[j];
                for (int k = j + 1; k < min(j+100, (int)permutation_list.size()); ++k) {
                    if (has_improvement) break;
                    p = p * permutation_list[k];
                    if (permutation_map.find(p.mapping()) != permutation_map.end()) {
                        move_ids_t candidate_permutation = permutation_map[p.mapping()];
                        if (candidate_permutation.size() < p.length()) {
#pragma omp critical
                             {
                                if (p.length() - candidate_permutation.size() > best_replacement) {
                                    best_replacement = p.length() - candidate_permutation.size();
                                    best_permutation = candidate_permutation;
                                    best_j = j;
                                    best_k = k;
                                    has_improvement = true;
                                    cout << "Found improvement: " 
                                        << best_replacement 
                                        << "(" << p.length() << ")->(" 
                                        << permutation_map[p.mapping()].size()
                                        << ")" << endl;
                                }
                            }
                        }
                    }
                    else {
                        Permutation<Size> p_inverse = p.inverse();
                    }
//                    else
//#pragma omp critical
//                    {
//                        permutation_map[p.mapping()] = p;
//                    }
                }
            }
            cout << "Best replacement: " << best_replacement << endl;
            if (has_improvement) {
                Permutation<Size> new_solution = solution[entry.first].until(best_j, allowed_moves);
                for (auto move_id : best_permutation) {
					new_solution = new_solution * allowed_moves[move_id];
				}                
                new_solution = new_solution * solution[entry.first].from(best_k + 1, allowed_moves);
                cout << "Searching puzzle " << entry.first
                    << "[" << entry.second.type() << "]"
                    << ", j=" << best_j
                    << ", k=" << best_k
                    << "/" << permutation_list.size()
                    << " (" << initial_len << ")->(" << new_solution.length() << ")"
                    << endl;
                solution[entry.first] = new_solution;
                permutation_list = new_solution.split(allowed_moves);
            }
        }
        cout << "Puzzle " << entry.first << " (" << initial_len << ")->(" << solution[entry.first].length() << ")                                                                    " << endl;
    }
    cout << "Done" << endl;
}

template <std::size_t Size>
void write_solution(
    const unordered_map<int, Puzzle<Size>>& puzzles,
    const unordered_map<int, Permutation<Size>>& solution,
    const unordered_map<string, unsigned char> move_id_map
) {
    unordered_map<unsigned char, string> inverse_move_id_map;
    for (const auto& move : move_id_map) {
        inverse_move_id_map[move.second] = move.first;
    }

    int solution_length = 0;
    for (const auto& p : solution) {
        solution_length += p.second.length();
    }
    string filename = "solution_" + to_string(solution_length) + ".csv";
    cout << "Writing solution to " << filename << endl;

    ofstream file(filename);
    if (!file.is_open()) {
        cerr << "Error opening file: solution.txt" << endl;
        exit(EXIT_FAILURE);
    }
    file << "id,moves" << endl;
    for (auto& sol : solution) {
        file << sol.first << ",";
        move_ids_t move_ids = sol.second.move_ids();
        file << inverse_move_id_map[(int)move_ids[0]];
        for (int j = 1; j < move_ids.size(); ++j) {
            file << "." << inverse_move_id_map[(int)move_ids[j]];
        }
        file << endl;
    }
}


int main(int argc, char** argv) {
    // Measure execution time
    clock_t start = clock();

    // Get first positional argument as filename
    cout << "Number of arguments:" << argc << endl;
    if (argc < 5) {
		cerr << "Usage: " << argv[0] << "command <puzzle_filename> <puzzle_info_filename> <solution_filename> <puzzle_type> <depth>" << endl;
		exit(EXIT_FAILURE);
	}
    string command = *(++argv);
    string puzzle_filename = *(++argv);
    string puzzle_info_filename = *(++argv);
    string solution_filename = *(++argv);
    string puzzle_type = "all";
    if (argc > 5) {
        puzzle_type = *(++argv);
    }

    int depth = -1;
    if (argc > 6) {
        depth = atoi(*(++argv));
    }

    cout << "Command: " << command << endl;
    cout << "Puzzle filename: " << puzzle_filename << endl;
    cout << "Puzzle info filename: " << puzzle_info_filename << endl;
    cout << "Solution filename: " << solution_filename << endl;
    cout << "Puzzle type: " << PUZZLE_TYPE << endl;
    cout << "Depth: " << depth << endl;

    unordered_map<int, Puzzle<PUZZLE_SIZE>> puzzles = readPuzzles<PUZZLE_SIZE>(puzzle_filename, PUZZLE_TYPE);
    pair<vector<Permutation<PUZZLE_SIZE>>, unordered_map<string, unsigned char>> puzzle_info = readPuzzleInfo<PUZZLE_SIZE>(puzzle_info_filename, PUZZLE_TYPE);
    vector<Permutation<PUZZLE_SIZE>> allowed_moves = puzzle_info.first;
    unordered_map<string, unsigned char> move_id_map = puzzle_info.second;
    unordered_map<int, Permutation<PUZZLE_SIZE>> solution = readSolution<PUZZLE_SIZE>(solution_filename, puzzles, allowed_moves, move_id_map);

  
    //if (command == "enumeration") {
	//	enumeration(puzzles, solution, puzzle_info, PUZZLE_TYPE);
    //}
   // else 
    if (command == "identities") {
		fast_identities(puzzles, solution, allowed_moves, depth);
    }

    write_solution<PUZZLE_SIZE>(puzzles, solution, move_id_map);

    // Measure execution time
    clock_t end = clock();
    double elapsed_secs = double(end - start) / CLOCKS_PER_SEC;
    cout << "Elapsed time: " << elapsed_secs << " seconds" << endl;

    return 0;
}
