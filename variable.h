#ifndef VARIABLE_H
#define VARIABLE_H

#include "includes.h"

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

#endif
 