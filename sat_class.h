#ifndef SAT_CLASS_H
#define SAT_CLASS_H
#include "includes.h"
#include "variable.h"

class SATinstance {
public:
    std::unordered_map<int, std::pair<int, int>> head_tail;
    std::unordered_map<int, Variable> variables;
    std::unordered_set<int> satisified_clauses;
    std::stack<std::pair<int, int>> assigned_variables;
    std::unordered_set<int> unsigned_varialbes;
    std::unordered_set<int> reducted_clauses;
    std::unordered_set<int> newly_created_binary_clauses;
    std::unordered_map<int, double> weights;
    std::unordered_map<int, int> variable_count;
    double trigger;
    double start_tigger;
    int decision_level;
    static std::unordered_map<int, std::vector<int>> formula;
    SATinstance(std::unordered_set<int> satisified_clauses,
                std::unordered_map<int, double> weights,
                std::unordered_map<int, int> variable_count,
                std::unordered_map<int, std::pair<int, int>> head_tail,
                std::unordered_set<int> unsigned_varialbes,
                std::unordered_map<int, Variable> variables, int decision_level) : variable_count(variable_count), weights(weights), satisified_clauses(satisified_clauses),
                                                                                   head_tail(head_tail), variables(variables), decision_level(decision_level), unsigned_varialbes(unsigned_varialbes) {}

    // Copy constructor
    SATinstance(const SATinstance &p2) {
        head_tail = p2.head_tail;
        variables = p2.variables;
        satisified_clauses = p2.satisified_clauses;
        decision_level = p2.decision_level;
        unsigned_varialbes = p2.unsigned_varialbes;
        weights = p2.weights;
        variable_count = p2.variable_count;
        trigger = p2.trigger;
        start_tigger =p2.start_tigger;
    }

    // Copy assignment operator
    SATinstance& operator=(SATinstance p2)
    {
        head_tail = p2.head_tail;
        variables = p2.variables;
        satisified_clauses = p2.satisified_clauses;
        decision_level = p2.decision_level;
        unsigned_varialbes = p2.unsigned_varialbes;
        weights = p2.weights;
        variable_count = p2.variable_count;
        trigger = p2.trigger;
        start_tigger =p2.start_tigger;
        return *this;
    }

    std::unordered_set<int> preselect_propz() {
        if(decision_level < 5 || unsigned_varialbes.size() <= 10) {
            return unsigned_varialbes;
        }

        auto appears_positive = std::unordered_set<int>();
        auto appears_negative = std::unordered_set<int>();
        for(auto var: unsigned_varialbes) {
            for(auto clause_hash: variables[var].clauses) {
                auto clause = formula[clause_hash];
                if(clause.size() == 2) {
                    clause[0] < 0 ? appears_negative.insert(abs(clause[0])) : appears_positive.insert(abs(clause[0]));
                    clause[1] < 0 ? appears_negative.insert(abs(clause[1])) : appears_positive.insert(abs(clause[1]));
                }
            }
        }
        auto result_set = std::unordered_set<int>();
        std::set_intersection (appears_positive.begin(), appears_positive.end(), appears_negative.begin(), appears_negative.end(), std::inserter(result_set, result_set.begin()));
        auto it = unsigned_varialbes.begin();
        while(result_set.size() < 10 && it != unsigned_varialbes.end()) {
            result_set.insert(*it);
            it++;
        }
        return result_set;
    }

    std::unordered_set<int> preselect_cra() {
        auto occurence_number = std::unordered_map<int, int>();
        auto binary_clause_neighbours = std::unordered_map<int, std::vector<int>>();
        auto cra_map = std::unordered_map<int, int>();
        // iterate throught all not satisfied caluses (they should be in unsigned variables clauses list)
        // prepare binary clause neighbours, occurence number
        int unsigned_varialbes_count = unsigned_varialbes.size();
        for(auto var: unsigned_varialbes) {
            cra_map[var] = 0;
            for(auto clause_hash: variables[var].clauses) {
                if(abs(get_head(clause_hash)) == var) {
                    auto clause = formula[clause_hash];
                    if(clause.size() == 2) {
                        for(auto literal: clause) {
                            if(binary_clause_neighbours.find(literal) == binary_clause_neighbours.end()) {
                                binary_clause_neighbours[literal] = std::vector<int>();
                            }
                        }
                        binary_clause_neighbours[clause[0]].push_back(clause[1]);
                        binary_clause_neighbours[clause[1]].push_back(clause[0]);
                    } else {
                        for(auto literal: clause) {
                            if(occurence_number.find(literal) == occurence_number.end()) {
                                occurence_number[literal] = 0;
                            }
                            occurence_number[literal] += 1;
                        }
                    }
                }
            }
        }

        if(unsigned_varialbes_count > 20) {
            for(auto var: unsigned_varialbes) {
                int first_sum = 0;
                int second_sum = 0;
                for(auto negihbour: binary_clause_neighbours[var]) {
                    first_sum += occurence_number[-1*negihbour];
                }
                for(auto negihbour: binary_clause_neighbours[-1*var]) {
                    second_sum += occurence_number[-1*negihbour];
                }
                cra_map[var] = first_sum*second_sum;
            }
        }

        auto size = std::max(20, unsigned_varialbes_count/10);
        std::vector<std::pair<int, int>> top(size);
        auto result_set = std::unordered_set<int>();
        std::partial_sort_copy(cra_map.begin(),
                            cra_map.end(),
                            top.begin(),
                            top.end(),
                            [](std::pair<int, int> const& l,
                                std::pair<int, int> const& r)
                            {
                                return l.second > r.second;
                            });
        std::transform(top.begin(), top.end(), std::inserter(result_set, result_set.begin()),
                   [](std::pair<int, int> c) { return c.first; });
        return result_set;
    }

    int get_head(int clause) {
        return formula[clause][head_tail[clause].first];
    }

    int get_tail(int clause) {
        return formula[clause][head_tail[clause].second];
    }

    std::pair<int,int> get_head_tail_pair(int clause) {
        return std::make_pair(formula[clause][head_tail[clause].first], formula[clause][head_tail[clause].second]);
    }

    bool clause_is_satisfied(int variable_value, int literal) {
        return (variable_value == 1 && literal > 0) || (variable_value == 0 && literal < 0);
    }

    void resolve_unit_clause(int literal, int variable) {
        int variable_value;
        literal < 0 ? variable_value = 0 : variable_value = 1;
        variables[variable].value = variable_value;
        assigned_variables.push(std::make_pair(variable, variable_value));
    }

    int clause_is_satisfied_or_new_value_assigned(
            int literal, int index, bool is_head,
            std::pair<int, int>& head_tail_pair, int clause_hash) {

        int variable = abs(literal);
        int variable_value = variables[variable].value;
        if (variable_value > -1) { // Variable value was assigned;
            if( clause_is_satisfied(variable_value, literal) ) {
                return -1;
            }
        }
        else {
            // assign new head/tail
            is_head ? head_tail_pair.first = index : head_tail_pair.second = index;
            variables[variable].clauses.insert(clause_hash);
            reducted_clauses.insert(clause_hash);
            if(get_clause_size(clause_hash)==2) {
                newly_created_binary_clauses.insert(clause_hash); 
            }
            // Add to list of that tail, remove from list of current
            return abs(literal);
        }
        return 0;
    }

    int resolve_last_literal(int literal) {
        int variable = abs(literal);
        int variable_value = variables[variable].value;
        if (variable_value > -1) { // Variable value was assigned;
            if( clause_is_satisfied(variable_value, literal) ) {
                return -1;
            } else {
                return -2;
            }
        }

        resolve_unit_clause(literal, variable);
        return -1;
    }

    int find_new_tail(std::vector<int>& clause,
                      std::pair<int, int>& head_tail_pair,
                      int clause_hash) {
        auto i=head_tail_pair.second;
        for(; i>head_tail_pair.first; i--) {
            int assign_value = clause_is_satisfied_or_new_value_assigned(clause[i], i, false, head_tail_pair, clause_hash);
            if(assign_value != 0) {
                return assign_value;
            }
        }

        return resolve_last_literal(clause[i]);
    }

    int find_new_head(std::vector<int>& clause,
                      std::pair<int, int>& head_tail_pair,
                      int clause_hash) {
        auto i=head_tail_pair.first;
        for(; i<head_tail_pair.second; i++) {
            int assign_value = clause_is_satisfied_or_new_value_assigned(clause[i], i, true, head_tail_pair, clause_hash);
            if(assign_value != 0) {
                return assign_value;
            }
        }

        return resolve_last_literal(clause[i]);

    }

    void prepare_satisfied_clause(int clause) {
        satisified_clauses.insert(clause);
        auto head_tail_pair = get_head_tail_pair(clause);
        if(reducted_clauses.find(clause) != reducted_clauses.end()) {
            reducted_clauses.erase(clause);
        }
        if(newly_created_binary_clauses.find(clause) != newly_created_binary_clauses.end()) {
            newly_created_binary_clauses.erase(clause);
        }
        variables[abs(head_tail_pair.first)].clauses.erase(clause);
        variables[abs(head_tail_pair.second)].clauses.erase(clause);
        #if DECISION_HEURISTIC == 0 || DECISION_HEURISTIC == 2
            for(auto var: formula[clause]) {
                variable_count[var] -= 1;
            }
        #endif
    }

    bool propagation(int varaible, int new_value) {
        reducted_clauses = std::unordered_set<int>();
        newly_created_binary_clauses = {};
        assigned_variables = std::stack<std::pair<int, int>>();
        assigned_variables.push(std::make_pair(varaible, new_value));
        while (!assigned_variables.empty())
        {
            std::pair<int, int> var = assigned_variables.top();
            assigned_variables.pop();
            unsigned_varialbes.erase(var.first);
            variables[var.first].value = var.second;
            variables[var.first].assigned_level = decision_level;
            std::list<int> satisified_clauses_in_row = std::list<int>();
            for(auto clause: variables[var.first].clauses) {
                int result;
                if(var.first == abs(get_head(clause))) {
                    result = find_new_head(formula[clause], head_tail[clause], clause);
                } else {
                    result = find_new_tail(formula[clause], head_tail[clause], clause);
                }

                if (result==-1) { // Clause satisfied
                    satisified_clauses_in_row.push_back(clause);
                } else if (result==-2) {
                    return false;
                }
            }
            for(auto clause: satisified_clauses_in_row) {
                prepare_satisfied_clause(clause);
            }
        }
        return true;
    }

    bool is_satisfied() {
        return satisified_clauses.size() == formula.size();
    }
    int get_clause_size(int hash) {
        return head_tail[hash].second - head_tail[hash].first + 1;
    }
};

std::unordered_map<int, std::vector<int>> SATinstance::formula = std::unordered_map<int, std::vector<int>>();

#endif
