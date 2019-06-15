#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <sstream>
#include <iterator>
#include <list>
#include <algorithm>
#include <stack>
#include <fstream>
#include "reader.h"
#include <future>


class SATinstance {
public:
    std::unordered_map<int, std::pair<int, int>> head_tail;
    std::unordered_map<int, Variable> variables;
    std::unordered_set<int> satisified_clauses;
    std::stack<std::pair<int, int>> assigned_variables;
    std::unordered_set<int> unsigned_varialbes;
    int decision_level;
    static std::unordered_map<int, std::vector<int>> formula;
    SATinstance(std::unordered_set<int> satisified_clauses,
                std::unordered_map<int, std::pair<int, int>> head_tail,
                std::unordered_set<int> unsigned_varialbes,
                std::unordered_map<int, Variable> variables, int decision_level) : satisified_clauses(satisified_clauses),
                                                                                   head_tail(head_tail), variables(variables), decision_level(decision_level), unsigned_varialbes(unsigned_varialbes) {}

    // Copy constructor
    SATinstance(const SATinstance &p2) {
        head_tail = p2.head_tail;
        variables = p2.variables;
        satisified_clauses = p2.satisified_clauses;
        decision_level = p2.decision_level;
        unsigned_varialbes = p2.unsigned_varialbes;
    }

    // Copy assignment operator
    SATinstance& operator=(SATinstance p2)
    {
        head_tail = p2.head_tail;
        variables = p2.variables;
        satisified_clauses = p2.satisified_clauses;
        decision_level = p2.decision_level;
        unsigned_varialbes = p2.unsigned_varialbes;
        return *this;
    }

    std::unordered_set<int> preselect() {
        return unsigned_varialbes;
    }

    int get_head(int clause) {
        return formula[clause][head_tail[clause].first];
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
        variables[abs(head_tail_pair.first)].clauses.erase(clause);
        variables[abs(head_tail_pair.second)].clauses.erase(clause);
    }

    bool propagation(int varaible, int new_value) {
        assigned_variables = std::stack<std::pair<int, int>>();
        assigned_variables.push(std::make_pair(varaible, new_value));
        while (!assigned_variables.empty())
        {
            std::pair<int, int> var = assigned_variables.top();
            unsigned_varialbes.erase(var.first);
            assigned_variables.pop();
            //std::cout << "Working with: " << var.first << " " << var.second << '\n';
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
                    // DO something, clause can not be satisfied
                    //std::cout << "result: -2" << '\n';
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
};


std::unordered_map<int, std::vector<int>> SATinstance::formula = std::unordered_map<int, std::vector<int>>();

int find_next_variable(std::unordered_map<int, Variable>& variables) {
    for(auto i: variables) {
        if(i.second.value == -1) {
            return i.first;
        }
    }
    return -1;
}

double decision_heuristic(SATinstance& start_instance, SATinstance& true_instance, SATinstance& flase_instance) {
    return 2*start_instance.unsigned_varialbes.size() - true_instance.unsigned_varialbes.size() -
           flase_instance.unsigned_varialbes.size();
}


int look_ahead(SATinstance& instance, int& true_size, int& false_size) {
    auto preselect = instance.preselect();
    //std::cout << preselect.size() << '\n';
    double decision_heuristic_result = -100;
    int selected_var = -1;
    for(auto i: preselect) {
        if(instance.variables[i].value == -1) {
            auto result_of_true_instance = instance;
            auto result_of_false_instance = instance;
            auto res1 = std::async(&SATinstance::propagation, &result_of_true_instance, i, 1);
            auto res2 = std::async(&SATinstance::propagation, &result_of_false_instance, i, 0);
            bool true_result = res1.get();
            bool false_result = res2.get();

            double new_heuristic_result = decision_heuristic(instance, result_of_true_instance,
                                                             result_of_false_instance);

            if (!true_result && !false_result) {
                return 0;
            } else if (!true_result) {
                //std::cout << "result_of_false_instance (only true possible) " << i << '\n'; 
                instance = result_of_false_instance;
            } else if (!false_result) {
                instance = result_of_true_instance;
            } else if (decision_heuristic_result < new_heuristic_result) {
                true_size =  result_of_true_instance.unsigned_varialbes.size();
                false_size = result_of_false_instance.unsigned_varialbes.size();
                decision_heuristic_result = new_heuristic_result;
                selected_var = i;
            }
        }
    }
    //std::cout << "Sizes: " << true_size << " " << false_size << "\n";
    return selected_var;
}

bool dpll(SATinstance instance) {
    instance.decision_level += 1;
    if(instance.is_satisfied()) {
        return true;
    }
    int true_size = 0;
    int false_size = 0;
    int var = look_ahead(instance, true_size, false_size);
    //std::cout << "selected new war: " << var << '\n';
    if(var == 0) {
        return false;
    } else if(var == -1) {
        return dpll(instance);
    } else {
        int first_propagation = 1, second_propagation = 0;
        if(false_size < true_size) {
            first_propagation = 0;
            second_propagation = 1;
        }
        auto instance_copy = instance;
        bool propagarion_res = instance.propagation(var, 0);
        //std::cout << "var: " << var << " value: 0 is " << propagarion_res << '\n'; 
        //std::cout << "propagation result " << propagarion_res << " var: (" << var << ", " << 0 << ")"  <<  instance.decision_level << '\n';
        if(propagarion_res && dpll(instance)) {
            return true;
        } else {
            propagarion_res = instance_copy.propagation(var, 1);
            //std::cout << "var: " << var << " value: 1 is " << propagarion_res << '\n'; 
            return propagarion_res &&  dpll(instance_copy);
        }
    }
}

int main() {

    //std::ifstream in("/home/olga/licencjat/input/hole10.cnf");
    //std::cin.rdbuf(in.rdbuf());

    auto formula = std::unordered_map<int, std::vector<int>>();
    auto satisified_clauses = std::unordered_set<int>();
    auto head_tail = std::unordered_map<int, std::pair<int, int>>();
    // Name of variable, variable.
    auto variables = std::unordered_map<int, Variable>();
    auto unasigned_variables = std::unordered_set<int>();
    read_input(formula, variables, head_tail, unasigned_variables);

    SATinstance::formula = formula;
    SATinstance problem = SATinstance(satisified_clauses, head_tail, unasigned_variables, variables, 0);

    auto res = dpll(problem);
    std::cout << "Result: " << res << '\n';
}