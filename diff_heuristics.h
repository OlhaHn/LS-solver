#ifndef DIFF_HEURISTICS_H
#define DIFF_HEURISTICS_H

#include "sat_class.h"

/*
Clause reduction heuristic
*/
double count_heuristic(SATinstance& new_instance) {
    double coef[] = {1,1,1,0.2,0.05,0.01,0.003,0};
    double res = 0;
    for(auto clause_hash: new_instance.reducted_clauses) {
        double clause_value = coef[std::min(new_instance.get_clause_size(clause_hash), 7)];
        res += clause_value;
    }
    return res;
}

/*
Function for counting WBH, BSH, BSHR heuristics
*/
double get_WBH_or_BSH_count(SATinstance& start_instance, SATinstance& new_instance) {
    auto new_satisfied_clauses = std::unordered_set<int>();
    // new_satisfied_clauses = new_instance.satisified_clauses - start_instance.start_instance
    std::set_difference(new_instance.satisified_clauses.begin(), new_instance.satisified_clauses.end(),
        start_instance.satisified_clauses.begin(), start_instance.satisified_clauses.end(), 
        std::inserter(new_satisfied_clauses, new_satisfied_clauses.begin()));

    // As clauses are satisified they are "removed" from formula, update weights according to this
    for(auto clause_hash: new_satisfied_clauses) {
        auto size = start_instance.get_clause_size(clause_hash);
        for(auto var: new_instance.formula[clause_hash]) {
            new_instance.weights[var] -= powers[size-3];
        }
    }

    // Update weights according to newly reducted clauses 
    for(auto clause_hash: new_instance.reducted_clauses) {
        int new_size = new_instance.get_clause_size(clause_hash);
        int old_size = start_instance.get_clause_size(clause_hash);

        for(auto var: new_instance.formula[clause_hash]) {
            new_instance.weights[var] = new_instance.weights[var] - powers[old_size-3] + powers[new_size-3];
        }
    }
    double result = 0;
    
    #if DIFF_HEURISTIC == 3 
        // For backbone search normalized heuristic count normalization coeff
        double bsrh_coeff = 0.0;
        int total_size = 0;
        for(auto clause_hash: new_instance.reducted_clauses) {
            total_size += new_instance.get_clause_size(clause_hash);
            for(auto var: new_instance.formula[clause_hash]) {
                bsrh_coeff += new_instance.weights[-1*var];
            }
        }
        bsrh_coeff /= total_size;

        for(auto clause_hash: new_instance.reducted_clauses) {
            double clause_result = powers[new_instance.get_clause_size(clause_hash) - 3];
            for(auto var: new_instance.formula[clause_hash]) {
                clause_result = clause_result * new_instance.weights[-1*var] / bsrh_coeff;
            }
        }

    #else 
        for(auto clause: new_instance.reducted_clauses) {
            if(new_instance.get_clause_size(clause)==2) {
                auto head = new_instance.get_head(clause);
                auto tail = new_instance.get_tail(clause);
                result += new_instance.weights[-1*head] + new_instance.weights[-1*tail]; 
            }
        }
    #endif
    return result;
}

double WBH_or_BSH_heuristic(SATinstance& start_instance, 
        SATinstance& true_instance, SATinstance& false_instance, double& true_size, double& false_size) {
        true_size = get_WBH_or_BSH_count(start_instance, true_instance);
        false_size = get_WBH_or_BSH_count(start_instance, false_instance);
    return true_size*false_size;
}

double count_heuristic(SATinstance& start_instance, SATinstance& true_instance, SATinstance& false_instance, double& true_size, double& false_size) {
        true_size = count_heuristic(true_instance);
        false_size = count_heuristic(false_instance);
    return true_size*false_size;
}


#endif
