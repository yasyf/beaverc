#pragma once

#include <string>
#include <map>
#include <stack>
#include "types.h"

struct Value;
struct ReferenceValue;

typedef std::map<std::string, std::shared_ptr<Value>> ValueMap;

struct Interpreter {
    std::shared_ptr<Function> main_function;
    std::shared_ptr<ValueMap> global_variables;
    Interpreter(std::shared_ptr<Function> const & main_func) ;
    void interpret();
    std::shared_ptr<Value> run_function(Function const & func,
                                        std::vector<std::shared_ptr<Value>> const & arguments,
                                        std::vector<std::shared_ptr<ReferenceValue>> const & references);
};

#include "values.h"