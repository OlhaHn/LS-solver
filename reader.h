//
// Created by olga on 5/29/19.
//

#ifndef LICENCJAT_READER_H
#define LICENCJAT_READER_H

#endif //LICENCJAT_READER_H



struct Variable
{
    short value; // -1 for unsigned
    int assigned_level;
    std::unordered_set<int> clauses;
    friend std::ostream & operator << (std::ostream &out, const Variable &c);
};

std::ostream & operator << (std::ostream &out, const Variable &c) {
    out << "value: " << c.value << " level: " << c.assigned_level << " clauses: [ ";
    for(auto i: c.clauses) {
        out << i << " ";
    }
    out << "]";
    return out;
}

void prepare_variables(int number, std::unordered_map<int, Variable>& variables,
                       std::unordered_set<int>& unsigned_variables) {
    for(int i=1; i<=number; i++) {
        Variable var{-1, -1, {}};
        variables[i] = var;
        unsigned_variables.insert(i);
    }
}

void print_variables(std::unordered_map<int, Variable>& variables) {
    for(auto i: variables) {
        std::cout << "( " << i.first << " ) " << i.second << '\n';
    }
}


void read_input(std::unordered_map<int, std::vector<int>>& formula,
                std::unordered_map<int, Variable>& variables,
                std::unordered_map<int, std::pair<int, int>>& head_tail,
                std::unordered_set<int>& unsigned_variables) {
    std::string line;
    for (line; std::getline(std::cin, line);) {
        if (line[0] == 'p') {
            break;
        }
    }

    std::istringstream iss(line);
    std::vector<std::string> info(std::istream_iterator<std::string>{iss},
                                  std::istream_iterator<std::string>());
    int variables_number = std::stoi(*(info.end() - 2));
    int clauses_number = std::stoi(*(info.end() - 1));

    prepare_variables(variables_number, variables, unsigned_variables);

    for(int i=0; i<clauses_number; i++) {
        std::string input;
        std::getline(std::cin, input);
        std::stringstream stream(input);

        std::vector<int> clause = std::vector<int>();

        int n;
        stream >> n;

        while(n) {
            clause.push_back(n);
            stream >> n;
        }

        formula[i] = clause;
        head_tail[i] = std::make_pair(0, clause.size()-1);
        variables[abs(clause[0])].clauses.insert(i);
        variables[abs(*(clause.end()-1))].clauses.insert(i);
    }

}
