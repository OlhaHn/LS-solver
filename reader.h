#ifndef READER_H
#define READER_H

/*
    Set default values for all nessesary strucutures
*/
void prepare_variables(int number, std::unordered_map<int, Variable>& variables,
                       std::unordered_set<int>& unsigned_variables, 
                       std::unordered_map<int, double>& weights,
                       std::unordered_map<int, int>& variable_count) {
    for(int i=1; i<=number; i++) {
        Variable var{-1, -1, {}};
        variables[i] = var;
        weights[i] = 0.0;
        weights[-1*i] = 0.0;
        variable_count[i] = 0;
        variable_count[-1*i] = 0;
        unsigned_variables.insert(i);
    }
}

void read_input(std::unordered_map<int, std::vector<int>>& formula,
                std::unordered_map<int, Variable>& variables,
                std::unordered_map<int, std::pair<int, int>>& head_tail,
                std::unordered_set<int>& unsigned_variables,
                std::unordered_map<int, double>& weights,
                std::unordered_map<int, int>& variable_count) {

    std::string line;
    // skip comments
    while (std::getline(std::cin, line)) {
        if (line[0] == 'p') {
            break;
        }
    }

    std::istringstream iss(line);
    std::vector<std::string> info(std::istream_iterator<std::string>{iss},
                                  std::istream_iterator<std::string>());

    int variables_number = std::stoi(*(info.end() - 2));
    int clauses_number = std::stoi(*(info.end() - 1));

    // insert default values
    prepare_variables(variables_number, variables, unsigned_variables, weights, variable_count);

    int max_clause_size = 0;

    // insert real values into the datastructure
    for(int i=0; i<clauses_number; i++) {
        std::string input;
        std::getline(std::cin, input);
        std::stringstream stream(input);

        std::vector<int> clause = std::vector<int>();

        int n;
        stream >> n;
        int clause_size = 1;
        while(n) {
            clause.push_back(n);
            variable_count[n] += 1;
            stream >> n;
            clause_size++;
        }
        if(clause_size > max_clause_size) {
            max_clause_size = clause_size;
        }

        formula[i] = clause;
        head_tail[i] = std::make_pair(0, clause.size()-1);
        variables[abs(clause[0])].clauses.insert(i);
        variables[abs(*(clause.end()-1))].clauses.insert(i);
    }
    // Do additional computations depending on selected heuristics
    #if DIFF_HEURISTIC != 0
        #if DIFF_HEURISTIC == 1
            double current_pow_value_wbh = 125.0;
            for(int i=0; i<=max_clause_size; i++) {
                powers[i] = current_pow_value_wbh;
                current_pow_value_wbh /= 5.0;       
            }

        #elif DIFF_HEURISTIC >= 2
            double current_pow_value_bsh = 8.0;
            for(int i=0; i<=max_clause_size; i++) {
                powers[i] = current_pow_value_bsh;
                current_pow_value_bsh /= 2.0;          
            }
        #endif
        
        
        for(auto clause_pair: formula) {
            auto clause = clause_pair.second;
            int size = clause.size();

            auto pow_value = powers[size];
            for(auto var: clause) {
                weights[var] += pow_value;
            }
        }
    #endif

}

#endif //READER_H
