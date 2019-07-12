#ifndef SETTINGS_H
#define SETTINGS_H

#endif //SETTINGS_H

#ifndef DIFF_HEURISTIC
#define DIFF_HEURISTIC 0

/*
    Possible values: 
        0 "crh" - clause reduction heuristic, 
        1 "wbh" - weighted binaries heuristic,
        2 "bsh" - backbone search heuristic,
        3 "bsrh" - backbone search renormalized heuristic
*/

#endif // DIFF_HEURISTIC

#ifndef DECISION_HEURISTIC
#define DECISION_HEURISTIC 2

/*
    Possible values: 
        0 "kcnfs" - selects 1 if x appears more often then -x, 
        1 "march" - selects 1 if diff(x=1) < diff(x=2)
        2 "posit" - selects 0 if x appears more often then -x, 
        3 "satz" - always select 1 first
*/
#endif // DECISION_HEURISTIC

#ifndef PRESELECT_HEURISTIC
#define PRESELECT_HEURISTIC 0

/*
    Possible values: 
        0 propz
        1 clause reduction approximation
*/
#endif // PRESELECT_HEURISTIC

#ifndef AUTARKY_REASONING
#define AUTARKY_REASONING 1

/*
    Possible values: 
        0 don't use autarky_reasoning
        1 use autarky_reasoning
*/

#endif
