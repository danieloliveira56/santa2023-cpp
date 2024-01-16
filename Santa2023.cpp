#include <random>
#include <set>

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

#define INVERSE_PERMUTATION_SEARCH
#define HEURISTIC_PERMUTATION_SEARCH



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
		string type;
		string moves;
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
		for (auto& move_id : jsonMoves.getMemberNames()) {
			mapping_t<Size> mapping;
			for (size_t i = 0; i < jsonMoves[move_id].size(); ++i) {
				mapping[i] = (unsigned short)jsonMoves[move_id][(int)i].asInt();
			}
			allowed_moves.emplace_back(mapping, move_ids_t({ j }));
			move_id_map[move_id] = j;
			j++;
			mapping_t<Size> inverse = getInverse(mapping);
			if (inverse == mapping) continue;
			allowed_moves.emplace_back(inverse, move_ids_t({ j }));
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
	const int memory_limit
) {
	cout << "Getting Permutations of length " << Size << " generated by " << allowed_moves.size() << " moves..." << endl;

	// We can use a permutation different from the identity
	Permutation<Size> root_permutation = Permutation<Size>();
	//root_permutation = root_permutation * allowed_moves[13] * allowed_moves[13] * allowed_moves[15] * allowed_moves[15];

	unordered_map<mapping_t<Size>, move_ids_t> permutation_map{ {root_permutation.mapping(), root_permutation.move_ids()} };

	vector<mapping_t<Size>> moves_mappings;
	for (const auto& move : allowed_moves) {
		moves_mappings.push_back(move.mapping());
	}

	PROCESS_MEMORY_COUNTERS_EX pmc;
	GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
	SIZE_T current_memory_usage = pmc.WorkingSetSize / 1024.0 / 1024;
	SIZE_T previous_memory_usage = current_memory_usage;

	cout << setw(5) << "Depth"
		<< setw(15) << "Size"
		<< setw(20) << "Increase_Factor"
		<< setw(20) << "Memory_Usage(MB)"
		<< setw(20) << "Next_Mem_Usage(MB)"
		<< setw(15) << "Buckets"
		<< setw(15) << "Load_Factor"
		<< setw(15) << "t(s)" << endl;

	cout << setw(5) << 0
		<< setw(15) << 1
		<< setw(20) << 0
		<< setw(20) << current_memory_usage
		<< setw(20) << "?"
		<< setw(15) << permutation_map.bucket_count()
		<< setw(15) << permutation_map.load_factor() << endl;

	size_t previous_size = 1;
	size_t previous_level_size = 1;
	int i = 1;
	while (true) {
		clock_t start = clock();
		for (const auto& entry : permutation_map) {
			if (entry.second.size() != i - 1) continue;
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

		// Enumerated all permutations
		if (current_level_size == 0)
		{
			cout << "No more permutations found, stopping at depth " << i << endl;
			break;
		}
		double increase_factor = (double)current_level_size / previous_level_size;
		// << "Estimated memory usage: " << permutation_map.bucket_count() * (sizeof(mapping_t<Size>) + sizeof(move_ids_t)) / 1024.0 / 1024 / 1024 << " GB,"

		GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
		current_memory_usage = pmc.WorkingSetSize / 1024.0 / 1024;
		SIZE_T memory_increase = current_memory_usage - previous_memory_usage;
		SIZE_T estimated_next_level_memory_usage = current_memory_usage + memory_increase * increase_factor;

		cout << setw(5) << i
			<< setw(15) << current_size
			<< setw(20) << increase_factor
			<< setw(20) << current_memory_usage
			<< setw(20) << estimated_next_level_memory_usage
			<< setw(15) << permutation_map.bucket_count()
			<< setw(15) << permutation_map.load_factor()
			<< setw(15) << double(clock() - start) / CLOCKS_PER_SEC << endl;

		//permutation_map.reserve(current_size + increase_factor * current_level_size);

		if (estimated_next_level_memory_usage > memory_limit) {
			cout << "Memory limit of " << memory_limit << "MB reached reached, stopping at depth " << i << endl;
			break;
		}

		previous_memory_usage = current_memory_usage;
		previous_level_size = current_size;
		previous_size = current_size;
		i++;
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
Permutation<Size> simulated_annealing(
	Puzzle<Size>& puzzle,
	Permutation<Size>& solution,
	const vector<Permutation<Size>>& allowed_moves,
	double initial_temp,
	double cooling_factor,
	double min_temp,
	int max_retries,
	int max_length
) {
	string puzzle_type = puzzle.type();
	cout << "Simmulated Annealing" << endl;
	cout << "Cooling Factor: " << cooling_factor << endl;
	cout << "Puzzle " << puzzle.getId() << " (" << puzzle_type << ")" << endl;

	vector<Permutation<Size>> best_solutions;
	Permutation<Size> new_solution;
	Permutation<Size> incumbent_solution;
	int current_solution_score = puzzle.countMismatches();
	int best_solution_score = current_solution_score;
	int initial_score = current_solution_score;
	int incumbent_solution_score = current_solution_score;

	cout << "Initial Score: " << current_solution_score << endl;

	random_device rd;  // Will be used to obtain a seed for the random number engine
	mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()
	uniform_real_distribution<> dis(0.0, 1.0);

	vector<int> components_order({ 0 });
	vector<int> components_to_search({ 0 });
	vector<int> restart_counts;
	vector<int> worsening_steps;

	double T = initial_temp;

	while (true) {
		bool found_path = false;
		int trial_ct = 0;
		vector<Permutation<Size>> relevant_permutations;
		for (const auto& p : allowed_moves) {
			if (puzzle.admissablePermutation(p, components_to_search)) {
				relevant_permutations.push_back(p);
			}
		}
		cout << "Number of relevant permutations: " << relevant_permutations.size() << endl;

		while (!found_path && T >= min_temp) {
			for (int p_length = 1; p_length <= max_length; ++p_length) {
				//int p_length = rand() % max_length + 1;
				long long idx1 = pow(relevant_permutations.size(), p_length - 1) + 1;
				long long idx2 = pow(relevant_permutations.size(), p_length);
				//cout << "Permutation length: " << p_length << ", indices: " << idx1 << "-" << idx2 << " (" << idx2 - idx1 + 1 << ")" << endl;

			/*for (long long permutation_idx1 = 0; permutation_idx1 < 10000; permutation_idx1++)
			{*/
#pragma omp parallel for
				for (long long permutation_idx = idx1; permutation_idx < idx2; permutation_idx++)
				{
					//long long  permutation_idx = rand() % (idx2-idx1) + idx1;
					//

					Permutation<Size> p;
					/*
					int previous_move_id = -1;
					for (int i = 0; i < p_length; ++i) {
						int move_id = previous_move_id;
						while (move_id == previous_move_id) {
							move_id = rand() % relevant_permutations.size();
						}
						p = p * relevant_permutations[rand() % relevant_permutations.size()];
						previous_move_id = move_id;
					}*/

					vector<int> permutation_vector = get_base_n_vector(permutation_idx, relevant_permutations.size());
					for (int j = permutation_vector.size(); j < p_length - 1; ++j) {
						if (relevant_permutations[j].isInverse(relevant_permutations[j + 1]))
							continue;
					}

					for (auto j : permutation_vector) {
						p = p * relevant_permutations[j];
					}
					if (p == Permutation<Size>()) continue;
#ifdef HEURISTIC_PERMUTATION_SEARCH
					int score = puzzle.countMismatches((new_solution * p), components_to_search);
					//int score_commutative = puzzle.countMismatches((p * new_solution * p.inverse()), components_to_search);
#pragma omp critical
					{
						if (score < best_solution_score) {
							//cout << "Puzzle " << puzzle.getId() << " Score (" << incumbent_solution_score << ")->(" << score << ") ";
							incumbent_solution_score = score;
							incumbent_solution = p;
							// cout << "Length: " << incumbent_solution.length() << endl;
							found_path = true;
						}
						else if (!found_path && (dis(gen) < exp((incumbent_solution_score - score - 1) / T))) {
							incumbent_solution_score = score;
							incumbent_solution = p;
						}
						//else if (!found_path && score_commutative < best_solution_score) {
						//    cout << "Puzzle " << puzzle.getId() << " Score (" << incumbent_solution_score << ")->(c" << score_commutative << ") ";
						//    incumbent_solution_score = score_commutative;
						//    incumbent_solution = p * new_solution * p.inverse();
						//    cout << "Length: " << incumbent_solution.length() << endl;
						//    found_path = true;
						//}
					}
#endif
#ifdef EXACT_PERMUTATION_SEARCH
					for (int i = 0; i < solution.size(); ++i) {
						if (p.length() >= solution[i].length()) continue;
						vector<string> initial_string = puzzles.at(i).getInitial();
						vector<string> current_string(puzzles.at(i).size());
						for (int j = 0; j < initial_string.size(); ++j) {
							current_string[j] = initial_string[p.mapping()[j]];
						}
						if (current_string == puzzles.at(i).getSolution()) {
#pragma omp critical
							{
								//cout << "Found solution for puzzle " << i << ": " << p << endl;
								if (p.length() < solution[i].length()) {
									solution[i] = p;
									cout << "Puzzle " << i << " (" << solution[i].length() << ")->(" << p.length() << ")                 " << endl;
								}
							}
						}
					}
#endif // EXACT_PERMUTATION_SEARCH
					if (found_path) break;
				}
				if (found_path) break;
				T *= cooling_factor;
				cout << p_length << "/" << max_length << " " << T << "/" << min_temp << endl;
			}
		}
#ifdef HEURISTIC_PERMUTATION_SEARCH
		//cout << "=========Puzzle " << puzzle.getId() << "==================" << endl;
		if (incumbent_solution_score <= puzzle.getNumWildcards()) {
			if (components_to_search.size() < components_order.size()) {
				cout << "Solved component " << components_to_search.back() << endl;
				new_solution = new_solution * incumbent_solution;
				best_solutions.push_back(new_solution);
				components_to_search.push_back(components_order[components_to_search.size()]);
				current_solution_score = puzzle.countMismatches(new_solution, components_to_search);
				best_solution_score = current_solution_score;
				incumbent_solution_score = current_solution_score;
				cout << "Solving components: ";
				for (auto c : components_to_search) {
					cout << c << ", ";
				}
				cout << endl;
				cout << "New Score: " << best_solution_score << endl;
				T = initial_temp;
			}
			else {
				new_solution = new_solution * incumbent_solution;
				best_solutions.push_back(new_solution);
				cout << "Solved all components" << endl;
				for (auto m : new_solution.move_ids()) {
					cout << (int)m << ", ";
				}
				return best_solutions.back();
			}
			continue;
		}

		// cout << "Incumbent Score: " << incumbent_solution_score;
		if (incumbent_solution_score < best_solution_score) {
			new_solution = new_solution * incumbent_solution;
			best_solutions.push_back(new_solution);
			best_solution_score = incumbent_solution_score;
			current_solution_score = incumbent_solution_score;
			restart_counts.push_back(0);
			worsening_steps.push_back(0);
			// cout << " (accepted improving solution)";
		}
		else {
			new_solution = new_solution * incumbent_solution;
			current_solution_score = incumbent_solution_score;
			worsening_steps.back()++;
			// cout << " (accepted worsening solution)";
		}

		if (T < min_temp) {
			for (auto r : restart_counts) {
				cout << r << ", ";
			}
			cout << endl;
			for (auto sol : best_solutions) {
				cout << puzzle.countMismatches(sol) << "/" << sol.length() << ", ";
			}
			while (restart_counts.back() > max_retries) {
				restart_counts.pop_back();
				best_solutions.pop_back();
				worsening_steps.pop_back();
			}
			new_solution = best_solutions.back();
			current_solution_score = puzzle.countMismatches(new_solution, components_to_search);
			best_solution_score = current_solution_score;
			incumbent_solution_score = current_solution_score;
			T = initial_temp;
			restart_counts.back()++;

			//new_solution = Permutation<Size>();
			//best_solution = Permutation<Size>();
			//best_solution_score = 0;
			//current_solution_score = 0;
			//components_to_search = vector<int>({ 2 });
			// cout << "*** Backtracking to solution of length " << best_solution.length() << ", and score " << best_solution_score << endl;
		}
		//for (auto c : incumbent_solution.move_ids()) {
		//    cout << (int)c << ".";
		//}
		//cout << endl;
		//cout << "T: " << T << endl;
		//cout << "Best Score: " << best_solution_score << endl;
		//cout << "Best Solution Length: " << best_solution.length()<< endl;
		//cout << "Current Solution Score: " << current_solution_score << endl;
		//cout << "Current Solution length: " << new_solution.length() << endl;

		cout << " Id      T Best_Score Best_Length Restart_ct Incumbent_Score Incumbent_Length Last_accepted_length Bad_Step_ct Components" << endl;

		cout << setw(3) << puzzle.getId()
			<< " " << setw(6) << setprecision(4) << T
			<< " " << setw(10) << best_solution_score
			<< " " << setw(11) << best_solutions.back().length()
			<< " " << setw(10) << restart_counts.back()
			<< " " << setw(15) << current_solution_score
			<< " " << setw(16) << new_solution.length()
			<< " " << setw(20) << incumbent_solution.length()
			<< " " << setw(11) << worsening_steps.back()
			<< " {";
		for (auto c : components_to_search) {
			cout << (int)c << ", ";
		}
		cout << "}\r" << endl;

		/*for (size_t j = 0; j < best_solution.move_ids().size(); ++j) {
			cout << setw(2) << (int)new_solution.move_ids()[j] << ".";
		}
		cout << "(end of best solution)" << endl;
		for (size_t j = best_solution.move_ids().size(); j < new_solution.move_ids().size(); ++j) {

			cout << (int)new_solution.move_ids()[j] << ".";
		}*/
		//cout << endl << "===========================" << endl;

#endif
	}
}

template <std::size_t Size>
void fast_identities(
	const unordered_map<int, Puzzle<Size>>& puzzles,
	unordered_map<int, Permutation<Size>>& solution,
	const vector<Permutation<Size>>& allowed_moves,
	int memory_limit
) {
	cout << "Running fast identities" << endl;
	string curr_type;
	unordered_map<mapping_t<Size>, move_ids_t> permutation_map;

	for (auto& entry : puzzles) {
		//cout << "Puzzle " << puzzles[i].getId() << " (" << puzzles[i].type() << "), size: " << solution[i].size() << endl;
		int initial_len = solution[entry.first].length();
		if (entry.second.type() != curr_type) {
			curr_type = entry.second.type();
			permutation_map = getPermutations<Size>(allowed_moves, memory_limit);
			cout << "Got permutations for type " << curr_type << ", size: " << permutation_map.size() << endl;
		}

		//      bool found_solution = false;
		//      // Check if any mapping in permutation_map is a solution
		//      for (const auto& permutation : permutation_map) {
		//          if (entry.second.isSolution(permutation.first)) {
		//              cout << "Found solution for puzzle " << entry.first << ": " << permutation.second.size() << endl;
		//              if (permutation.second.size() <= solution[entry.first].length()) {
		//                  Permutation<Size> new_solution = Permutation<Size>();
		//                  for (auto move_id : permutation.second) {
		//                      new_solution = new_solution * allowed_moves[move_id];
		//                  }
			  //	        solution[entry.first] = new_solution;
			  //	        cout << "Puzzle " << entry.first << " (" << solution[entry.first].length() << ")->(" << permutation.second.size() << ")                 " << endl;
		//                  found_solution = true;
		//              }
		//          }
			  //}
		//      if (found_solution) continue;

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
				for (int k = j + 1; k < min(j + 100, (int)permutation_list.size()); ++k) {
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
										<< candidate_permutation.size()
										<< ")" << endl;
								}
							}
						}
					}
#ifdef INVERSE_PERMUTATION_SEARCH
					else {
						Permutation<Size> p_inverse = p.inverse();
						if (permutation_map.find(p_inverse.mapping()) != permutation_map.end()) {
							move_ids_t candidate_permutation = permutation_map[p_inverse.mapping()];
							if (candidate_permutation.size() < p.length()) {
#pragma omp critical
								{
									if (p.length() - candidate_permutation.size() > best_replacement) {
										best_replacement = p.length() - candidate_permutation.size();
										best_permutation = candidate_permutation;

										// reverse the move_ids
										for (int l = 0; l < best_permutation.size() / 2; ++l) {
											unsigned char temp = best_permutation[l];
											best_permutation[l] = best_permutation[best_permutation.size() - l - 1];
											best_permutation[best_permutation.size() - l - 1] = temp;
										}
										best_j = j;
										best_k = k;
										has_improvement = true;
										cout << "Found improvement (reversed): "
											<< best_replacement
											<< " (" << p.length() << ")->("
											<< candidate_permutation.size()
											<< ")" << endl;
									}
								}
							}
						}
					}
#endif // INVERSE_PERMUTATION_SEARCH
#ifdef LEVEL2_MAP_SEARCH
					if (!has_improvement) {
						for (auto& q : permutation_map) {
							if (q.second.size() == 0) continue;
							if (q.second.size() >= p.length() - 1) continue;
							mapping_t<Size> pq_1_mapping = p.mapping();
							for (int l = 0; l < Size; ++l) {
								pq_1_mapping[q.first[l]] = pq_1_mapping[l];
							}
							if (permutation_map.find(pq_1_mapping) != permutation_map.end()) {
								move_ids_t candidate_permutation = permutation_map[pq_1_mapping];
								candidate_permutation.insert(candidate_permutation.end(), q.second.begin(), q.second.end());
								if (candidate_permutation.size() < p.length()) {
#pragma omp critical
									{
										if (p.length() - candidate_permutation.size() > best_replacement) {
											best_replacement = p.length() - candidate_permutation.size();
											best_permutation = candidate_permutation;
											best_j = j;
											best_k = k;
											has_improvement = true;
											cout << "Found improvement (2nd level) -"
												<< best_replacement
												<< " (" << p.length() << ")->("
												<< candidate_permutation.size()
												<< ")" << endl;
											cout << "p: ["; for (auto x : p.mapping()) cout << (int)x << " "; cout << "]" << endl;
											cout << "q: ["; for (auto x : q.first) cout << (int)x << " "; cout << "]" << endl;
											cout << "pq_1: ["; for (auto x : pq_1_mapping) cout << (int)x << " "; cout << "]" << endl;
											cout << "p: ["; for (auto x : p.move_ids()) cout << (int)x << " "; cout << "]" << endl;
											cout << "q: ["; for (auto x : q.second) cout << (int)x << " "; cout << "]" << endl;
											cout << "pq_1: ["; for (auto x : permutation_map[pq_1_mapping]) cout << (int)x << " "; cout << "]" << endl;
											cout << "candidate_permutation: ["; for (auto x : candidate_permutation) cout << (int)x << " "; cout << "]" << endl;
										}
									}
								}
							}
						}
					}
#endif

					//                    else
					//#pragma omp critical
					//                    {
					//                        permutation_map[p.mapping()] = p;
					//                    }
				}
			}
			cout << "\tBest replacement : " << best_replacement << endl;
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

//template <std::size_t Size>
//void shortcuts(
//    const unordered_map<int, Puzzle<Size>>& puzzles,
//    unordered_map<int, Permutation<Size>>& solution
//) {
//
//    for (auto& entry : puzzles) {
//        bool has_shortcut = true;
//        while (has_shortcut) {
//            has_shortcut = false;
//            unordered_map<string, int> pattern_map;
//            string initial_string;
//            for (const auto& s : = entry.second.getInitial()) initial_string += s;
//            vector<Permutation<Size>> permutation_list = solution[entry.first].split(allowed_moves);
//
//
//        }
//
//    }
//
//
//
//}

template <std::size_t Size>
void solve_replace(string& puzzle_filename, string& puzzle_info_filename, string& solution_filename, string& puzzle_type, int memory_limit) {
	clock_t start = clock();
	unordered_map<int, Puzzle<Size>> puzzles = readPuzzles<Size>(puzzle_filename, puzzle_type);
	pair<vector<Permutation<Size>>, unordered_map<string, unsigned char>> puzzle_info = readPuzzleInfo<Size>(puzzle_info_filename, puzzle_type);
	vector<Permutation<Size>> allowed_moves = puzzle_info.first;
	unordered_map<string, unsigned char> move_id_map = puzzle_info.second;
	unordered_map<int, Permutation<Size>> solution = readSolution<Size>(solution_filename, puzzles, allowed_moves, move_id_map);
	fast_identities(puzzles, solution, allowed_moves, memory_limit);
	write_solution<Size>(puzzles, solution, move_id_map);
	clock_t end = clock();
	double elapsed_secs = double(end - start) / CLOCKS_PER_SEC;
	cout << "Elapsed time: " << elapsed_secs << " seconds" << endl;
}

template <std::size_t Size>
void solve_sa(string& puzzle_filename, string& puzzle_info_filename, string& solution_filename, string& puzzle_type, int puzzle_id, double initial_temp, double cooling_factor, double min_temp, int max_retries, int max_length) {
	clock_t start = clock();
	unordered_map<int, Puzzle<Size>> puzzles = readPuzzles<Size>(puzzle_filename, puzzle_type);
	pair<vector<Permutation<Size>>, unordered_map<string, unsigned char>> puzzle_info = readPuzzleInfo<Size>(puzzle_info_filename, puzzle_type);
	vector<Permutation<Size>> allowed_moves = puzzle_info.first;
	unordered_map<string, unsigned char> move_id_map = puzzle_info.second;
	unordered_map<int, Permutation<Size>> solution = readSolution<Size>(solution_filename, puzzles, allowed_moves, move_id_map);
	solution[puzzle_id] = simulated_annealing(puzzles.at(puzzle_id), solution.at(puzzle_id), allowed_moves, initial_temp, cooling_factor, min_temp, max_retries, max_length);
	write_solution<Size>(puzzles, solution, move_id_map);
	clock_t end = clock();
	double elapsed_secs = double(end - start) / CLOCKS_PER_SEC;
	cout << "Elapsed time: " << elapsed_secs << " seconds" << endl;
}

int main(int argc, char** argv) {
	printf("argc = %d\n", argc);
	for (int i = 0; i < argc; i++)
		printf("argv[%d] = %s\n", i, argv[i]);

	if (argc < 5) {
		cerr << "Usage: " << argv[0] << "command <puzzle_filename> <puzzle_info_filename> <solution_filename> <puzzle_id> command-specific arguments" << endl;
		cerr << "Commands: replace, sa" << endl;
		cerr << "replace: replace sequence of moves using mapping hash table" << endl;
		exit(EXIT_FAILURE);
	}
	string command = argv[1];
	string puzzle_filename = argv[2];
	string puzzle_info_filename = argv[3];
	string solution_filename = argv[4];

	if (command == "replace") {
		if (argc < 6) {
			cerr << "Usage: " << argv[0] << " replace <puzzle_filename> <puzzle_info_filename> <solution_filename> <puzzle_type> <memory limit>" << endl;
			cerr << "memory limit: maximum memory usage in MB" << endl;
			exit(EXIT_FAILURE);
		}
		string puzzle_type = argv[5];
		int memory_limit = atoi(argv[6]);

		if (puzzle_type == "cube_2/2/2") {
			solve_replace<24>(puzzle_filename, puzzle_info_filename, solution_filename, puzzle_type, memory_limit);
		}
		else if (puzzle_type == "cube_3/3/3") {
			solve_replace<54>(puzzle_filename, puzzle_info_filename, solution_filename, puzzle_type, memory_limit);
		}
		else if (puzzle_type == "cube_4/4/4") {
			solve_replace<96>(puzzle_filename, puzzle_info_filename, solution_filename, puzzle_type, memory_limit);
		}
		else if (puzzle_type == "cube_5/5/5") {
			solve_replace<150>(puzzle_filename, puzzle_info_filename, solution_filename, puzzle_type, memory_limit);
		}
		else if (puzzle_type == "cube_6/6/6") {
			solve_replace<216>(puzzle_filename, puzzle_info_filename, solution_filename, puzzle_type, memory_limit);
		}
		else if (puzzle_type == "cube_7/7/7") {
			solve_replace<294>(puzzle_filename, puzzle_info_filename, solution_filename, puzzle_type, memory_limit);
		}
		else if (puzzle_type == "cube_8/8/8") {
			solve_replace<384>(puzzle_filename, puzzle_info_filename, solution_filename, puzzle_type, memory_limit);
		}
		else if (puzzle_type == "cube_9/9/9") {
			solve_replace<486>(puzzle_filename, puzzle_info_filename, solution_filename, puzzle_type, memory_limit);
		}
		else if (puzzle_type == "cube_10/10/10") {
			solve_replace<600>(puzzle_filename, puzzle_info_filename, solution_filename, puzzle_type, memory_limit);
		}
		else if (puzzle_type == "cube_19/19/19") {
			solve_replace<2166>(puzzle_filename, puzzle_info_filename, solution_filename, puzzle_type, memory_limit);
		}
		else if (puzzle_type == "cube_33/33/33") {
			solve_replace<6534>(puzzle_filename, puzzle_info_filename, solution_filename, puzzle_type, memory_limit);
		}
		else if (puzzle_type == "wreath_6/6") {
			solve_replace<10>(puzzle_filename, puzzle_info_filename, solution_filename, puzzle_type, memory_limit);
		}
		else if (puzzle_type == "wreath_7/7") {
			solve_replace<12>(puzzle_filename, puzzle_info_filename, solution_filename, puzzle_type, memory_limit);
		}
		else if (puzzle_type == "wreath_12/12") {
			solve_replace<22>(puzzle_filename, puzzle_info_filename, solution_filename, puzzle_type, memory_limit);
		}
		else if (puzzle_type == "wreath_21/21") {
			solve_replace<40>(puzzle_filename, puzzle_info_filename, solution_filename, puzzle_type, memory_limit);
		}
		else if (puzzle_type == "wreath_33/33") {
			solve_replace<64>(puzzle_filename, puzzle_info_filename, solution_filename, puzzle_type, memory_limit);
		}
		else if (puzzle_type == "wreath_100/100") {
			solve_replace<198>(puzzle_filename, puzzle_info_filename, solution_filename, puzzle_type, memory_limit);
		}
		else if (puzzle_type == "globe_1/8") {
			solve_replace<32>(puzzle_filename, puzzle_info_filename, solution_filename, puzzle_type, memory_limit);
		}
		else if (puzzle_type == "globe_1/16") {
			solve_replace<64>(puzzle_filename, puzzle_info_filename, solution_filename, puzzle_type, memory_limit);
		}
		else if (puzzle_type == "globe_2/6") {
			solve_replace<36>(puzzle_filename, puzzle_info_filename, solution_filename, puzzle_type, memory_limit);
		}
		else if (puzzle_type == "globe_3/4") {
			solve_replace<32>(puzzle_filename, puzzle_info_filename, solution_filename, puzzle_type, memory_limit);
		}
		else if (puzzle_type == "globe_6/4") {
			solve_replace<56>(puzzle_filename, puzzle_info_filename, solution_filename, puzzle_type, memory_limit);
		}
		else if (puzzle_type == "globe_6/8") {
			solve_replace<112>(puzzle_filename, puzzle_info_filename, solution_filename, puzzle_type, memory_limit);
		}
		else if (puzzle_type == "globe_6/10") {
			solve_replace<140>(puzzle_filename, puzzle_info_filename, solution_filename, puzzle_type, memory_limit);
		}
		else if (puzzle_type == "globe_3/33") {
			solve_replace<264>(puzzle_filename, puzzle_info_filename, solution_filename, puzzle_type, memory_limit);
		}
		else if (puzzle_type == "globe_8/25") {
			solve_replace<450>(puzzle_filename, puzzle_info_filename, solution_filename, puzzle_type, memory_limit);
		}
	}
	else if (command == "sa") {
		int puzzle_id = atoi(argv[5]);
		double initial_temp = atof(argv[6]);
		double cooling_factor = atof(argv[7]);
		double min_temp = atof(argv[8]);
		int max_retries = atoi(argv[9]);
		int max_length = atoi(argv[10]);

		cout << "Cooling Factor: " << cooling_factor << endl;

		string puzzle_type;
		if (puzzle_id < 30) {
			puzzle_type = "cube_2/2/2";
			solve_sa<24>(puzzle_filename, puzzle_info_filename, solution_filename, puzzle_type, puzzle_id, initial_temp, cooling_factor, min_temp, max_retries, max_length);
		}
		else if (puzzle_id < 150) {
			puzzle_type = "cube_3/3/3";
			solve_sa<54>(puzzle_filename, puzzle_info_filename, solution_filename, puzzle_type, puzzle_id, initial_temp, cooling_factor, min_temp, max_retries, max_length);

		}
		else if (puzzle_id < 210) {
			puzzle_type = "cube_4/4/4";
			solve_sa<96>(puzzle_filename, puzzle_info_filename, solution_filename, puzzle_type, puzzle_id, initial_temp, cooling_factor, min_temp, max_retries, max_length);
		}
		else if (puzzle_id < 245) {
			puzzle_type = "cube_5/5/5";
			solve_sa<150>(puzzle_filename, puzzle_info_filename, solution_filename, puzzle_type, puzzle_id, initial_temp, cooling_factor, min_temp, max_retries, max_length);

		}
		else if (puzzle_id < 257) {
			puzzle_type = "cube_6/6/6";
			solve_sa<216>(puzzle_filename, puzzle_info_filename, solution_filename, puzzle_type, puzzle_id, initial_temp, cooling_factor, min_temp, max_retries, max_length);

		}
		else if (puzzle_id < 262) {
			puzzle_type = "cube_7/7/7";
			solve_sa<294>(puzzle_filename, puzzle_info_filename, solution_filename, puzzle_type, puzzle_id, initial_temp, cooling_factor, min_temp, max_retries, max_length);

		}
		else if (puzzle_id < 267) {
			puzzle_type = "cube_8/8/8";
			solve_sa<384>(puzzle_filename, puzzle_info_filename, solution_filename, puzzle_type, puzzle_id, initial_temp, cooling_factor, min_temp, max_retries, max_length);

		}
		else if (puzzle_id < 272) {
			puzzle_type = "cube_9/9/9";
			solve_sa<486>(puzzle_filename, puzzle_info_filename, solution_filename, puzzle_type, puzzle_id, initial_temp, cooling_factor, min_temp, max_retries, max_length);

		}
		else if (puzzle_id < 277) {
			puzzle_type = "cube_10/10/10";
			solve_sa<600>(puzzle_filename, puzzle_info_filename, solution_filename, puzzle_type, puzzle_id, initial_temp, cooling_factor, min_temp, max_retries, max_length);

		}
		else if (puzzle_id < 281) {
			puzzle_type = "cube_19/19/19";
			solve_sa<2166>(puzzle_filename, puzzle_info_filename, solution_filename, puzzle_type, puzzle_id, initial_temp, cooling_factor, min_temp, max_retries, max_length);

		}
		else if (puzzle_id < 284) {
			puzzle_type = "cube_33/33/33";
			solve_sa<6534>(puzzle_filename, puzzle_info_filename, solution_filename, puzzle_type, puzzle_id, initial_temp, cooling_factor, min_temp, max_retries, max_length);

		}
		else if (puzzle_id < 304) {
			puzzle_type = "wreath_6/6";
			solve_sa<10>(puzzle_filename, puzzle_info_filename, solution_filename, puzzle_type, puzzle_id, initial_temp, cooling_factor, min_temp, max_retries, max_length);

		}
		else if (puzzle_id < 319) {
			puzzle_type = "wreath_7/7";
			solve_sa<12>(puzzle_filename, puzzle_info_filename, solution_filename, puzzle_type, puzzle_id, initial_temp, cooling_factor, min_temp, max_retries, max_length);

		}
		else if (puzzle_id < 329) {
			puzzle_type = "wreath_12/12";
			solve_sa<22>(puzzle_filename, puzzle_info_filename, solution_filename, puzzle_type, puzzle_id, initial_temp, cooling_factor, min_temp, max_retries, max_length);

		}
		else if (puzzle_id < 334) {
			puzzle_type = "wreath_21/21";
			solve_sa<40>(puzzle_filename, puzzle_info_filename, solution_filename, puzzle_type, puzzle_id, initial_temp, cooling_factor, min_temp, max_retries, max_length);

		}
		else if (puzzle_id < 337) {
			puzzle_type = "wreath_33/33";
			solve_sa<64>(puzzle_filename, puzzle_info_filename, solution_filename, puzzle_type, puzzle_id, initial_temp, cooling_factor, min_temp, max_retries, max_length);

		}
		else if (puzzle_id < 338) {
			puzzle_type = "wreath_100/100";
			solve_sa<198>(puzzle_filename, puzzle_info_filename, solution_filename, puzzle_type, puzzle_id, initial_temp, cooling_factor, min_temp, max_retries, max_length);

		}
		else if (puzzle_id < 348) {
			puzzle_type = "globe_1/8";
			solve_sa<32>(puzzle_filename, puzzle_info_filename, solution_filename, puzzle_type, puzzle_id, initial_temp, cooling_factor, min_temp, max_retries, max_length);

		}
		else if (puzzle_id < 353) {
			puzzle_type = "globe_1/16";
			solve_sa<64>(puzzle_filename, puzzle_info_filename, solution_filename, puzzle_type, puzzle_id, initial_temp, cooling_factor, min_temp, max_retries, max_length);

		}
		else if (puzzle_id < 358) {
			puzzle_type = "globe_2/6";
			solve_sa<36>(puzzle_filename, puzzle_info_filename, solution_filename, puzzle_type, puzzle_id, initial_temp, cooling_factor, min_temp, max_retries, max_length);

		}
		else if (puzzle_id < 373) {
			puzzle_type = "globe_3/4";
			solve_sa<32>(puzzle_filename, puzzle_info_filename, solution_filename, puzzle_type, puzzle_id, initial_temp, cooling_factor, min_temp, max_retries, max_length);

		}
		else if (puzzle_id < 378) {
			puzzle_type = "globe_6/4";
			solve_sa<56>(puzzle_filename, puzzle_info_filename, solution_filename, puzzle_type, puzzle_id, initial_temp, cooling_factor, min_temp, max_retries, max_length);

		}
		else if (puzzle_id < 383) {
			puzzle_type = "globe_6/8";
			solve_sa<112>(puzzle_filename, puzzle_info_filename, solution_filename, puzzle_type, puzzle_id, initial_temp, cooling_factor, min_temp, max_retries, max_length);

		}
		else if (puzzle_id < 388) {
			puzzle_type = "globe_6/10";
			solve_sa<140>(puzzle_filename, puzzle_info_filename, solution_filename, puzzle_type, puzzle_id, initial_temp, cooling_factor, min_temp, max_retries, max_length);

		}
		else if (puzzle_id < 396) {
			puzzle_type = "globe_3/33";
			solve_sa<264>(puzzle_filename, puzzle_info_filename, solution_filename, puzzle_type, puzzle_id, initial_temp, cooling_factor, min_temp, max_retries, max_length);

		}
		else {
			puzzle_type = "globe_8/25";
			solve_sa<450>(puzzle_filename, puzzle_info_filename, solution_filename, puzzle_type, puzzle_id, initial_temp, cooling_factor, min_temp, max_retries, max_length);

		}
	}
	else {
		cerr << "Unknown command: " << command << endl;
		exit(EXIT_FAILURE);
	}

	return 0;
}
