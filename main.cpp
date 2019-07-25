#include "includes.h"
#include "settings.h"
#include "variable.h"
#include "helper_structures.h"
#include "reader.h"
#include "sat_class.h"
#include "diff_heuristics.h"

int look_ahead(SATinstance& instance, double& true_size, double& false_size, int depth);

int double_lookahead(SATinstance& instance, SATinstance& start_instance) {
    #if DOUBLE_LOOKAHEAD >= 1
        double true_size = 0;
        double false_size = 0;
        auto binary_clauses = instance.newly_created_binary_clauses.size();
        if(binary_clauses >= start_instance.trigger) {
            auto next_look_ahed_result = look_ahead(instance, true_size, false_size, 2);
            if (next_look_ahed_result == 0) {
                #if DOUBLE_LOOKAHEAD == 3
                    start_instance.trigger = start_instance.start_tigger;
                #endif
                return 0;
            } else {
                #if DOUBLE_LOOKAHEAD == 3 || DOUBLE_LOOKAHEAD == 4
                    start_instance.trigger = binary_clauses;
                #endif
            }
        }
    #endif
    return 1;
}


int look_ahead(SATinstance& instance, double& true_size, double& false_size, int depth) {
    #if PRESELECT_HEURISTIC == 0
    auto preselect = instance.preselect_propz();
    #else
    auto preselect = instance.preselect_cra();
    #endif
    double decision_heuristic_result = -100; // - infinity
    int selected_var = -1;

    double new_true_size, new_false_size;
    for(auto i: preselect) {
        if(instance.variables[i].value == -1) {
            #if AUTARKY_REASONING != 0
                if(instance.variable_count[i] == 0) { // positive literal does not appear
                    instance.propagation(i, 0);
                    continue;
                } else if(instance.variable_count[-1*i] == 0) { // negative literal does not apper
                    instance.propagation(i, 1);
                    continue;
                }
            #endif

            auto result_of_true_instance = instance;
            auto result_of_false_instance = instance;
            auto res1 = std::async(&SATinstance::propagation, &result_of_true_instance, i, 1);
            auto res2 = std::async(&SATinstance::propagation, &result_of_false_instance, i, 0);
            bool true_result = res1.get();
            bool false_result = res2.get();

            if (!true_result && !false_result) {
                return 0;
            } else {
                if (!true_result) {
                    instance = result_of_false_instance;
                    #if DOUBLE_LOOKAHEAD > 0
                        if(depth == 0) {
                            auto new_look_ahead_result = double_lookahead(instance, instance);
                            if (new_look_ahead_result == 0) {
                                return 0;
                            }
                        }
                    #endif

                } else if (!false_result) {
                    instance = result_of_true_instance;
                    #if DOUBLE_LOOKAHEAD > 0
                        if(depth == 0) { 
                            auto new_look_ahead_result = double_lookahead(instance, instance);
                            if (new_look_ahead_result == 0) {
                                return 0;
                            }
                        }
                    #endif
                } else {

                    #if DOUBLE_LOOKAHEAD > 0
                        if(depth == 0) {
                            auto double_for_true = double_lookahead(result_of_true_instance, instance);
                            auto double_for_false = double_lookahead(result_of_false_instance, instance);
                            if(double_for_true == 0 && double_for_false == 0) {
                                return 0;
                            }
                        }
                    #endif

                    #if DIFF_HEURISTIC != 0
                    double new_heuristic_result = WBH_or_BSH_heuristic(instance, result_of_true_instance,
                                                                    result_of_false_instance, new_true_size, new_false_size);
                    #else 
                    double new_heuristic_result = count_heuristic(instance, result_of_true_instance,
                                                                    result_of_false_instance, new_true_size, new_false_size);
                    #endif

                    if (decision_heuristic_result < new_heuristic_result) {
                        true_size =  new_true_size;
                        false_size = new_false_size;
                        decision_heuristic_result = new_heuristic_result;
                        selected_var = i;
                    }
                }
            }
        }
    }

    #if DOUBLE_LOOKAHEAD == 4
        if(instance.trigger > 0) {
            instance.trigger--;
        }
    #endif
    if(instance.variables[selected_var].value != -1) {
        return -1;
    }
    return selected_var;
}

bool dpll(SATinstance instance) {
    instance.decision_level += 1;
    if(instance.is_satisfied()) {
        return true;
    }
    double true_size = 0;
    double false_size = 0;
    int var = look_ahead(instance, true_size, false_size, 0);

    if(var == 0) {
        return false;
    } else if(var == -1) {
        return dpll(instance);
    } else {
        int first_propagation = 1, second_propagation = 0;

        #if DECISION_HEURISTIC == 0
            // kcnfs
            if(instance.variable_count[-1*var] > instance.variable_count[var]) {
                first_propagation = 0; second_propagation = 1;
            }
        #elif DECISION_HEURISTIC == 1
            // march
            if(true_size >= false_size) {
                first_propagation = 0; second_propagation = 1;
            }
        #elif DECISION_HEURISTIC == 2
            // posit
            if(instance.variable_count[-1*var] <= instance.variable_count[var]) {
                first_propagation = 0; second_propagation = 1;
            }
        #endif

        auto instance_copy = instance;
        bool propagarion_res = instance.propagation(var, first_propagation);
        if(propagarion_res && dpll(instance)) {
            return true;
        } else {
            propagarion_res = instance_copy.propagation(var, second_propagation);
            return propagarion_res &&  dpll(instance_copy);
        }
    }
}

int main() {

    auto formula = std::unordered_map<int, std::vector<int>>();
    auto satisified_clauses = std::unordered_set<int>();
    auto head_tail = std::unordered_map<int, std::pair<int, int>>();
    // Name of variable, variable.
    auto variables = std::unordered_map<int, Variable>();
    auto unasigned_variables = std::unordered_set<int>();
    auto weights = std::unordered_map<int, double>();
    auto variable_count = std::unordered_map<int, int>();
    read_input(formula, variables, head_tail, unasigned_variables, weights, variable_count);

    SATinstance::formula = formula;
    SATinstance problem = SATinstance(satisified_clauses, weights, variable_count, head_tail, unasigned_variables, variables, 0);

    #if DOUBLE_LOOKAHEAD == 1
        problem.trigger = 65;
    #elif DOUBLE_LOOKAHEAD == 2 || DOUBLE_LOOKAHEAD == 3
        problem.trigger = 0.17*problem.variables.size();
        problem.start_tigger = 0.17*problem.variables.size();
    #elif DOUBLE_LOOKAHEAD == 4
        problem.trigger = 0;
    #endif
    
    auto start = std::clock();
    auto result = dpll(problem);
    auto duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
    std::cout << duration << ';';
    //std::cout << "Result: " << result << " " << "duration " << duration  << '\n';
}