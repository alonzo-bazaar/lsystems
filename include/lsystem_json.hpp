#ifndef LSYSTEMS_JSON_HPP_
#define LSYSTEMS_JSON_HPP_

#include <fstream>
#include <exception>
#include <memory>
#include <functional>
#include <algorithm> // std::ranges::find_if
#include <string>

#include <cassert>

#include "raylib.h"
#include "utils.hpp"
#include "result.hpp"
#include "rewrite.hpp"

#include "json.hpp"
#include "turtle.hpp"
using json = nlohmann::json;

std::string json_type_to_str(const json::value_t t) {
    switch(t) {
    case json::value_t::null:
        return "null";
    case json::value_t::boolean:
        return "boolean";
    case json::value_t::string:
        return "string";
    case json::value_t::number_integer:
        return "integer";
    case json::value_t::number_unsigned:
        return "unsigned integer";
    case json::value_t::number_float:
        return "float";
    case json::value_t::object:
        return "object";
    case json::value_t::array:
        return "array";
    case json::value_t::binary:
        return "binary";
    case json::value_t::discarded:
        return "discarded";
    default:
        assert(0 && "unreachable");
    }
}

res_fn(read_json_file, (const char* filename), json, std::string) {
    // https://github.com/nlohmann/json#read-json-from-a-file
    std::ifstream ifs(filename);
    try {
        const json j = json::parse(ifs);
        ifs.close();
        return OK(j);
    }
    catch(std::exception &e) {
        ifs.close();
        return ERR(e.what());
    }
    assert(0 && "unreachable");
}

// tree json contains these fields
struct ParsedTree {
    std::vector<std::array<float, 2>> texcoords_table;
    std::vector<float> thickness_table;
    std::vector<instruction> axiom;
    std::map<char, RewriteTarget> rewrite_rules;
    size_t rewrite_times;
    std::map<std::string, float> globals;
};
// let's parse them

/*
  texcoords table and thickness table formats have a fair bit in common
  for texcoords:
  accepted texcoords_table formats are
  1. list of paris of numbers
  2. object containing theese 3 keys and these 3 keys only:
     - start : array of 2 numbers
     - end : array of 2 numbers
     - steps : integer greater than 0

  for thickness_table same shit, but s/array of 2 numbers/a number
*/

// utilities for key checking 
// (to be utilized in conjunction with serialize_vec for error reporting)
std::vector<std::string> missing_keys
(const json& j, const std::vector<std::string>& required_keys) {
    assert(j.is_object());

    std::vector<std::string> res;
    for(const auto &k : required_keys)
        if(!j.contains(k))
            res.push_back(k);

    return res;
}

std::vector<std::string> excess_keys
(const json& j, const std::vector<std::string>& required_keys) {
    assert(j.is_object());

    std::vector<std::string> res;
    for(const auto &[k, v] : j.items())
        if(!contains(required_keys, k))
            res.push_back(k);

    return res;
}

// to avoid writing two parse_thicnkess_table and parse_texcoords_table
// functions with basically identical code save for 2 calls being different
// we're instead going to write one generic function to express whatever
// parsing for something formatted as specified above
// then express both parse_thicnkess_table and parse_texcoords_table
// as functions of this single generic function
template<typename T>
const std::function<Result<std::vector<T>, std::string>(const json& j)>
parse_table
(std::function<bool(const json& j)> validator,
 std::function<T(const json& j)> transformer,
 std::string short_data_name,
 std::string long_data_name) {
    // to make the ERR and RET&Co. macros work
    // they expect a ret_t type to be visible when they're called
    // using res_fn(...) this is guaranteed by the surrounding __hack struct
    // but given that this function's a bit too involved for that
    // we're gonna have to define it manually
    typedef Result<std::vector<T>, std::string> ret_t;

    return [validator, transformer, short_data_name, long_data_name]
        (const json& j) {
        try {
            // parse data table expressed as array
            if(j.is_array()) {
                auto f = std::find_if
                    (j.begin(), j.end(),
                     complement<const json&>(validator));
                if(f != j.end())
                    return ERR
                        ("cannot parse "
                         + short_data_name
                         + " table json array: element ["
                         + f->dump()
                         + "] at index ["
                         + std::to_string(f-j.begin())
                         + "] of the array, is not a valid"
                         + long_data_name);
                return OK(mapcar<json, T>(j, transformer));
            }

            // parse data table expressed as object
            else if(j.is_object()) {
                // required keys
                const std::vector<std::string>
                    required_keys = {"start", "end", "steps"};
                // contains all and only the required keys
                // all
                if(auto missing = missing_keys(j, required_keys);
                   !missing.empty())
                    return ERR
                        ("malformed " + short_data_name + " table object, "
                         "missing the following required keys: "
                         + serialize_vec(missing));
                // only
                if(auto excess = excess_keys(j, required_keys);
                   !excess.empty())
                    return ERR
                        ("malformed " + short_data_name + " table object, "
                         "contains the following invalid keys: "
                         + serialize_vec(excess));

                // get fields
                auto start = j["start"];
                auto end = j["end"];
                auto steps =  j["steps"];
                if(!validator(start))
                    return ERR
                        (short_data_name + " table start coordinate ["
                         + start.dump()
                         + "] is not a valid "
                         + long_data_name);
                if(!validator(end))
                    return ERR
                        (short_data_name + " table end coordinate ["
                         + start.dump()
                         + "] is not a valid "
                         + long_data_name);
                if(!steps.is_number())
                    return ERR
                        (short_data_name + " number of steps ["
                         + steps.dump()
                         + "] is not a valid integer");

                return OK(map_range(transformer(start),
                                    transformer(end),
                                    static_cast<size_t>(steps)));
            }
            else
                return ERR
                    (short_data_name + " table json of incorrect type: "
                     + json_type_to_str(j.type())
                     + " expected either an array or an object");
        } catch (std::exception& e) {
            return ERR_FURTHER
                ("unexpected error while parsing "
                 + short_data_name
                 + " table json",
                 {e.what()});
        }
        assert(0 && "unreachable");
    };
}

bool is_json_valid_texcoords(const json& j) {
    return j.is_array()
        && j.size() == 2
        && j[0].is_number()
        && j[1].is_number();
}

std::array<float, 2> valid_json_to_texcoords (const json& j) {
    assert(is_json_valid_texcoords(j));
    return {j[0], j[1]};
}

bool is_json_valid_thickness(const json& j) {
    return j.is_number() && static_cast<float>(j) >= 0;
}

float valid_json_to_thickness(const json& j) {
    assert(is_json_valid_thickness(j));
    return static_cast<float>(j);
}

const auto parse_texcoords_table = parse_table<std::array<float, 2>>
    (is_json_valid_texcoords,
     valid_json_to_texcoords,
     "texcoords",
     "texture coordinate (ie: an array of 2 numbers)");

const auto parse_thickness_table = parse_table<float>
    (is_json_valid_thickness,
     valid_json_to_thickness,
     "thickness",
     "thickness value (ie: any non negative number)");

/*
  parse rewrite_rules json
  rewrite rules are an object where keys are strings of length 1, and values
  express a rewrite target.

  rewrite targets can be expressed as
  - strings (default parameters, if any, inferred from globals map)
  - lists of parameterized instructions interspersed with strings
    parameterized instrucions are arrays starting with a string of length 1
    followed by zero or more arithmetic expressions that compute the
    zero or more parameters that accompany the instruction marked by
    that letter, with one arithmetic expression per parameter

  arithmetic expressions are expressed in a sort of lisp inspired dsl embedded
  within json, valid arithmetic expressions are:
  - all numbers (literals)
  - all strings (variable lookup)

  then we can json arrays expressings arithmetic expressions
  - ["+", args...]
  - ["*", args...]
  - ["-", arg1, arg2]
  - ["/", arg1, arg2]

  conditional expressions
  - ["if", cond, then, else]

  and compairisons
  - [">", arg1, arg2]
  - [">=", arg1, arg2]
  - ["<", arg1, arg2]
  - ["<=", arg1, arg2]
  - ["==", arg1, arg2]
  - ["<=", arg1, arg2, tolerance]
  - [">=", arg1, arg2, tolerarnce]
  - ["==", arg1, arg2, tolerance]

  to access parameters of parametric expressions we also provide the
  following "special form", if one may so call it
  - ["param", parameter_index]

  note for conditional compairison operators:
    when no tolerance is provided compairisons are performed with a rather
    high default tolerance because floating point numbers do be like that
    and this application does not require that much precision

  note for conditional expressions:
    in conditional expressions zero is falsey and all other values are
    truthy, values are compared to zero with a rather high tolerance
    (the same default tolerance used for compairison operators)
    so many low enough values are still gonna be zero

  the default tolerance used can be found here as the
  constexpr global variable `default_tolerance`
 */

// preliminary syntactic ensurance
res_fn(validate_arithmetic_expression, (const json& j), bool, std::string) {
    if(j.is_number())
        return OK(true);
    if(j.is_string())
        return OK(true);
    else if(j.is_array() && j.size()>0) { // composite expression
        auto op = j[0];
        if(!op.is_string())
            return ERR("operator " + op.dump()
                       + " of expression " + j.dump()
                       + " is not a string!");
        if(!contains({"+", "-", "*", "/",
                      ">", "<", "==", ">=", "<=",
                      "if", "param"}, op))
            return ERR("unrecognized operator " + op.dump());

        for(size_t i = 1; i<j.size(); i++)
            if(auto r = validate_arithmetic_expression(j[i]);
               !r.is_ok())
                return ERR_FURTHER("invalid operand at index "
                                   + std::to_string(i)
                                   + " of json arithmetic expression "
                                   + j.dump(),
                                   {r.err_trace()});

        auto rands_size = j.size()-1;
        bool right_size = ((op == "param") && (rands_size == 1))
            || (contains({"+", "*"}, op) && (rands_size >= 1))
            || (contains({"-", "/", ">", "<", "=="}, op) && (rands_size == 2))
            || (contains({"if", "==", ">=", "<="}, op) && (rands_size == 3));

        if(!right_size)
            return ERR("operator "
                       + op.dump()
                       + " called with incorrect number of arguments "
                       + std::to_string(rands_size)
                       + " in json arithmetic expression "
                       + j.dump());
        return OK(true);
    }
    return ERR("expected arithmetic expression to be either "
               " literal(number), variable (string), or expression(array) "
               " was given an expression of invalid type " +
               json_type_to_str(j.type()));
}

// wrapper around validate_arithmetic_expression that just returns
// a boolean for caller convenience
// but also logs any potential errors recieved by
// validate_arithmetic_expression for user conveience
// (ie: for my own convenience, I'm the user of this thing)
bool is_valid_arithmetic_expression(const json& j) {
    bool log_invalid=true;
    auto r = validate_arithmetic_expression(j);
    if(r.is_ok())
        return r.get();
    if(log_invalid) {
        std::cerr<<"invalid arithmetic expression json!\n";
        r.log_trace();
    }
    return false;
}

// https://stackoverflow.com/a/4598865
constexpr float default_tolerance = 0.0001;
bool float_eq(float a, float b, float tolerance=default_tolerance) {
    return (std::abs(a - b)
            <= tolerance * std::max(std::abs(a), std::abs(b)));
}

bool rands_eq(std::vector<float> rands) {
    if(rands.size() == 2)
        return float_eq(rands[0], rands[1]);
    else return float_eq(rands[0], rands[1], rands[2]);
}

// a json generated rewrite target will contain one or more lambdas
// invoking this function
res_fn(evaluate_arithmetic_expression,
       (const json& expr,
        const std::map<std::string, float>& globals,
        const std::vector<float>& params),
       float, std::string) {
    // numbers are self evaluating in arithmetic expressions
    if(expr.is_number())
        return OK(expr);

    // strings are variable lookups
    if(expr.is_string()) {
        if(auto f = globals.find(expr); f != globals.end())
            return OK(f->second);
        else
            return ERR("variable "
                       + expr.dump()
                       + " is not defined within this lsystem specification");
    }

    // arrays are composite expressions
    if(expr.is_array()) {
        // operator
        const json &op_j = expr[0];
        if(!op_j.is_string()) return ERR
                                  ("arithmetic expression operator "
                                   + op_j.dump()
                                   + " of expression "
                                   + expr.dump()
                                   + " is not a string, and can therefore "
                                   "not be interpreted as an operator");
        std::string op = op_j.get<std::string>();

        // operands
        auto rand_results = mapcar<size_t, ret_t>
            (iota(expr.size(), 1),
             [expr, globals, params](size_t i) {
                 return evaluate_arithmetic_expression
                     (expr[i], globals, params);
             });
        auto first_rand_err = std::find_if(rand_results.cbegin(),
                                           rand_results.cend(),
                                           [](ret_t r){return r.is_err();});
        if(first_rand_err != rand_results.end()) {
            size_t first_err_ind = first_rand_err - rand_results.cbegin();
            std::string first_err_str = expr[first_err_ind].dump();
            return ERR_FURTHER
                ("cannot evaluate arithmetic expression:\n"
                 + expr.dump()
                 + "\nencountered an error while evaluating operand expression\n"
                 + first_err_str
                 + "\nat index [" + std::to_string(first_err_ind) + "] "
                 "of the aphorementioned expression",
                 first_rand_err->err_trace());
        }

        // if we get here all results are ok
        std::vector<float> rands = mapcar<ret_t, ret_t::ok_t>
            (rand_results, ([](const ret_t& t){ return t.get(); }));

        // bit rudimentary but this helps avoid any heisenbugs caused by
        // macro name collisions
#ifdef expect_size
#error "lol change this macro's name then"
#endif
#define expect_size(pred, desc)                                 \
        if(!(rands.size() pred))                                \
            return ERR("\"" +  op +  "\" expression "           \
                       "expected " +  desc + " arguments"       \
                       " but " + std::to_string(rands.size())   \
                       + " arguments were provided instead")
        if(op == "+") {
            float acc = 0;
            for(auto f : rands) acc += f;
            return OK(acc);
        }
        if(op == "*") {
            float acc = 1;
            for(auto f : rands) acc *= f;
            return OK(acc);
        }
        if(op == "-") {
            expect_size(==2, "exactly 2");
            return OK(rands[0] - rands[1]);
        }
        if(op == "/") {
            expect_size(==2, "exactly 2");
            if(float_eq(rands[1], 0))
                return ERR("division by zero!");
            return OK(rands[0] / rands[1]);
        }
        if(op == "if") {
            expect_size(==3, "exactly 3");
            if(float_eq(rands[0], 0))
                return OK(rands[2]);
            return OK(rands[1]);
        }
        if(op == "==") {
            expect_size(>=2, "either 2 or 3");
            expect_size(<=3, "either 2 or 3");
            return OK(static_cast<float>(rands_eq(rands)));
        }
        if(op == ">=") {
            expect_size(>=2, "either 2 or 3");
            expect_size(<=3, "either 2 or 3");
            return OK(static_cast<float>
                      (rands[0]>=rands[1]) || rands_eq(rands));
        }
        if(op == "<=") {
            expect_size(>=2, "either 2 or 3");
            expect_size(<=3, "either 2 or 3");
            return OK(static_cast<float>
                      (rands[0]<=rands[1]) || rands_eq(rands));
        }
        if(op == ">") {
            expect_size(==2, "exactly 2");
            return OK(static_cast<float>(rands[0]>rands[1]));
        }
        if(op == "<") {
            expect_size(==2, "exactly 2");
            return OK(static_cast<float>(rands[0]<rands[1]));
        }
        if(op == "param") {
            expect_size(==1, "exactly one");
            const size_t i = static_cast<size_t>(rands[0]);
            if (i >= params.size())
                return ERR("\"param\" expression tried accessing parameter at"
                           " index " + std::to_string(i)
                           + " of parameter array with length "
                           + std::to_string(params.size()));
            return OK(params[i]);
        }
        return ERR("unrecognized operator [" + op + "]");
#undef expect_size
    }
    return ERR("this bit was supposed to be unreachable,"
               " ask gdb how you got here, I have no clue");
}

// now that we have functions to validate, parse, and evaluate
// the various (possibly arithmetic) expressions a rewrite target
// can be made out of, we can use them to create functions to validate,
// parse, and evaluate the rewrite targets in question
res_fn(validate_rewrite_target, (const json& j), bool, std::string) {
    if(j.is_string())
        return OK(true);
    if(j.is_array()) {
        bool is_stochastic = true;
        for(const auto&[_, v] : j.items())
            if(!(v.is_array() && v[0].is_number())) {
                is_stochastic = false;
                break;
            }

        if(!is_stochastic) {
            auto are_kids_valid = mapcar<const json&, bool>
                (j, [](const json& jj){
                    return jj.is_string() ||
                        (jj.is_array()
                         && jj[0].is_string()
                         && (jj[0].size() == 1)
                         && all<json>(suffix<json>(1, jj),
                                      is_valid_arithmetic_expression));
                });

            auto first_error = std::ranges::find(are_kids_valid, false);

            if(first_error != are_kids_valid.end())
                return ERR
                    ("deterministic rewrite target\n"
                     + j.dump()
                     + " \nis invalid as its subexpression at index "
                     + std::to_string((first_error - are_kids_valid.begin()))
                     + " (namely "
                     + j[(first_error - are_kids_valid.begin())].dump()
                     + " ) is invalid");
            return OK(true);
        }
        else {
            // probabilities all expressed as literals
            // and must all be in the range [0, 1] to be valid probabilities
            auto kid_probabilities = mapcar<const json&, float>
                (j, [](const json& j){return j[0].get<float>();});
            for(float f : kid_probabilities) {
                if(f < 0)
                    return ERR("stochastic rewrite target rule given with"
                               " negative probability " + std::to_string(f));
                if(f > 1)
                    return ERR("stochastic rewrite target rule given with"
                               " probability "
                               + std::to_string(f)
                               +" greater than 1");
            }

            auto kid_rewrite_targets = mapcar<const json&, json>
                (j, [](const json& j){return json(suffix<json>(1, j));});
            auto validate_kids = mapcar<const json&, ret_t>
                (kid_rewrite_targets, validate_rewrite_target);
            auto first_error = std::ranges::find_if
                (validate_kids, [](const ret_t&t){ return t.is_err(); });

            if(first_error != validate_kids.cend())
                return ERR_FURTHER
                    ("stochastic rewrite target\n"
                     + j.dump()
                     + "\nis invalid as its branch at index "
                     + std::to_string((first_error - validate_kids.cbegin()))
                     + " (namely "
                     + j[(first_error - validate_kids.cbegin())].dump()
                     + " ) is invalid",
                     first_error->err_trace());
            return OK(true);
        }
    }
    else
        return ERR("expected rewrite target to be either string or list"
                   " of rewrite target subexpressions, received rewrite"
                   " target:\n" + j.dump() + "\nis of invalid type "
                   + json_type_to_str(j.type()));
}

// sed 's/arithmetic_expression/rewrite_target/g' \
// < is_valid_arithmetic_expression
bool is_valid_rewrite_target(const json& j) {
    bool log_invalid=true;
    auto r = validate_rewrite_target(j);
    if(r.is_ok())
        return r.get();
    if(log_invalid) {
        std::cerr<<"invalid rewrite target json!\n";
        r.log_trace();
    }
    return false;
}

// when providing a series of l-system instructions as a string in a rewrite
// target, we have this conveience feature that, some instructions will be
// provided with their required parameters if the l-system spec has a specific
// tree-level global variable within it
// 
// to put it less abstractly, I have the string
// "[f///f]" in my l-system json
// but the underlying l-system I want to create is parametric, and in
// parametric l-systems as implemented here
// - 'f' needs a stride parameter, and
// - '/' needs an angle parameter
// so to make it possible to write "[f///f]" in a string without having to
// provide parameters for every character (which would be tedious)
// - all f (and F) instructions in a string are silently given
//   the default parameter "stride"
// - all +, -, &, ^, /, and \ instructions in a string are silently given
//   the default parameter "angle"
// 
// this, of course, requires that the l-system contain "stride" and "angle" 
// parameters, so this function just... fucking throws if that's not the case
instruction interpret_json_char
(const char c, const std::map<std::string, float>& globals) {
    switch(c) {
        // these don't take any parameter
    case '[':
    case ']':
    case '{':
    case '}':
    case '|':
    case '!':
    case '\'':
        return {c, {}};
        // these take a "stride" parameter
    case 'F':
    case 'f':
        if(auto f = globals.find("stride"); f != globals.end())
            return {c, {f->second}};
        throw std::runtime_error
            ("given instruction with implicit stride parameter but "
             "the \"stride\" variable is not present in the lsystem");
        // these take an "angle" parameter
    case '+':
    case '-':
    case '&':
    case '^':
    case '/':
    case '\\':
        if(auto f = globals.find("angle"); f != globals.end())
            return {c, {f->second}};
        throw std::runtime_error
            ("given instruction with implicit angle parameter but "
             "the \"angle\" variable is not present in the lsystem");
        // strings can ansl contain characters that are not instructions
        // ie: non terminal characters that are associated with rules to
        // expand later
    default:
        return {c, {}};
    }
}

// takes a json transition
// (ie: a deterministic rewrite target, or a "branch" of a stochastic
//  rewrite target)
// 
// and returns a transition function (transition defined in rewrite.hpp)
// that applies the instructions encoded in that json to a
// given parameter vector (in the context of a given globals map)
// 
// (a transition is a function that accepts a parameter vector and returns
//  a rewritten array of instructions)

// first of all we define these two functions whose job is to evaluate
// the different kinds of json transition "code"
// 
// these two functions are defined not quite for convenience or code cleanliness,
// they're defined more because these are the kind of function you REALLY wanna put
// breakpoint at, so inlining them in json_to_transition (as was before)
// is a generally rather bad fucking idea as far as debuggability is concerned
std::vector<instruction> json_eval_string(const std::string& j_str,
                                          const std::map<std::string, float>& globals,
                                          const std::vector<float>& params) {
    std::vector<instruction> res {};
    for(const char c : j_str) 
        res.push_back(interpret_json_char(c, globals));
    return res;
}

std::vector<instruction> json_eval_array(const json& j,
                                         const std::map<std::string, float>& globals,
                                         const std::vector<float>& params) {
    assert(j.is_array()); // scaramanzia
    std::vector<instruction> res {};
    for (const auto &[_, elt] : j.items()) {
        if(elt.is_string())
            for(const char c : elt.get<std::string>()) 
                res.push_back(interpret_json_char(c, globals));

        // [<char>, <params>...]
        else if(elt.is_array()
                && elt[0].is_string()
                && elt[0].size() == 1) {
            const char elt_c = elt[0].get<std::string>()[0];
            std::vector<float> elt_params;
            // this main loop iteration will add the
            // instruction {elt_c, elt_params} to the res vector

            // we have extracted elt_c, now to populate the
            // params array with the values of all param expressions
            // present in <params>...
            // (ie every subelement of the elt array except for the first string)
            for(size_t i = 1; i<elt.size(); ++i) {
                auto elt_param_expr = elt[i];
                if(!is_valid_arithmetic_expression(elt_param_expr))
                    throw std::runtime_error
                        ("cannot evaluate json arithmetic expression array\n"
                         + j.dump()
                         + "\nas the expression\n"
                         + elt_param_expr.dump()
                         + "\nat index ["
                         + std::to_string(i)
                         + "] of the array was was malformed");

                auto elt_param_r = evaluate_arithmetic_expression(elt_param_expr,
                                                                  globals,
                                                                  params);
                if(elt_param_r.is_err())
                    throw std::runtime_error
                        ("could not evaluate the following arithmetic"
                         " expression:\n"
                         + elt_param_expr.dump()
                         + " in the following rewrite expression:\n"
                         + elt.dump()
                         + "\nwithin the following rewrite target:\n"
                         + j.dump()
                         + "\ndue to the following error:\n"
                         + elt_param_r.string_trace());

                auto elt_param = elt_param_r.get();
                elt_params.push_back(elt_param);
            }
            res.push_back({elt_c, elt_params}); 
        }
        else
            throw std::runtime_error
                ("malformed json rewrite instruction:\n"
                 + elt.dump()
                 + "\nexpected either:"
                 + "\n  - a string of instruction characters"
                 + "\n  - an instruction of the form [<char>, <params>...]");
    }
    return res;
}

transition json_to_transition(const json& j,
                              const std::map<std::string, float>& globals) {
    if(j.is_string())
        return [j, globals](const std::vector<float>& params) {
            return json_eval_string(j.get<std::string>(), globals, params);
        };
    
    else if(j.is_array())
        return [j, globals](const std::vector<float>& params) {
            return json_eval_array(j, globals, params);
        };
    else
        throw std::runtime_error
            ("malformed json rewrite target:\n"
             + j.dump());
}

res_fn(parse_rewrite_target,
       (const json& j, const std::map<std::string, float>& globals),
       RewriteTarget, std::string) {

    if(!is_valid_rewrite_target(j))
        return ERR("invalid rewrite target");

    bool target_is_stochastic = j.is_array() && all<json>(j, [](const json& jj) {
        return jj.is_array() && jj.size() > 1 && jj[0].is_number();
    });
    if(target_is_stochastic) {
        std::vector<std::pair<float, transition>> probs;
        for(const auto& [_, v]:j.items())
            probs.push_back
                ({v[0],
                  json_to_transition(json(suffix<json>(1, v)), globals)});
        return OK(RewriteTarget(probs));
    }   

    // otherwise is deterministic
    return OK(RewriteTarget(json_to_transition(j, globals)));
}

// rewrite rules IE:
// object where keys are single letter strings
// and values are rewrite targets
res_fn(parse_rewrite_rules,
       (const json& j, const std::map<std::string, float>& globals),
       std::map<char, RewriteTarget>, std::string) {
    std::map<char, RewriteTarget> res;

    if(!j.is_object())
        return ERR("rewrite rules must be expressed an an object, "
                   "received rewrite rule "
                   + j.dump()
                   + " of invalid type "
                   + json_type_to_str(j.type()));

	for(const auto& [k, v] : j.items())
        // k is a string (or string like idfk)
        if(k.size() != 1)
            return ERR("in rewrite rules json keys must be strings of length exactly "
                       "one, found invalid key \"" + k + "\" of invalid length "
                       + std::to_string(k.size()));

	for(const auto& [k, v] : j.items()) {
        char c = k[0];

		auto r = parse_rewrite_target(v, globals);
		if(r.is_err())
			return ERR_FURTHER("cannot parse rewrite rules json,"
							   " error occured while parsing rewrite target"
							   " json",
							   r.err_trace());
		RewriteTarget rt = r.get(); 

		res.insert({c, rt});
	}
	return OK(res);
}

res_fn(parse_tree, (const json& j), ParsedTree, std::string) {
	if(!j.is_object())
		return ERR("tree json must be object, received tree of invalid "
				   "type " + json_type_to_str(j.type()));

	const std::vector<std::string>
		required_keys = {"texcoords_table",
						 "thickness_table",
						 "axiom",
						 "rewrite_rules",
						 "rewrite_times"};

	auto missing = missing_keys(j, required_keys);
	if(!missing.empty())
		return ERR("malformed tree json:"
				   " missing the following required keys: "
				   + serialize_vec(missing));

	std::map<std::string, float> globals;
	std::vector<std::string> globals_keys = excess_keys(j, required_keys);

    // leggerissimo acrrocchio visto che per poter specificare l'angolo sia in
    // radianti che in gradi si è deciso di rendere queste 3 variabili
    // un attimino speciali
    // visto che "angle", "angle_deg", e "angle_rad" vanno tutte e 3 a scrivere
    // la variable globals["angle"], vogliamo evitare che 2 o più di queste chiavi
    // siamo presenti contemporaneamente nel json
    const std::vector<std::string> angle_keys = {"angle",
                                                 "angle_deg",
                                                 "angle_rad"};
    int contained_angles = 0;
    for(const auto& k : angle_keys)
        if(contains(globals_keys, k))
            contained_angles++;
    if(contained_angles > 1)
        return ERR("tree json global variables\n"
                   + serialize_vec(globals_keys)
                   + "\n contain more than one angle themed global variable "
                   "the variables in question do risk overriding each other "
                   "please rewrite the json to contain only one such global "
                   "variable");

	for(const auto& gk : globals_keys) {
		auto f = j[gk];
		if(!f.is_number())
			return ERR("all global variables in tree must be numbers,"
					   " tree contains invalid global variable \""
					   + gk + "\" of non numeric type "
					   + json_type_to_str(f.type()));
        float val = f.get<float>();

        // questa logica vuol inoltre dire che "angle_rad" e "angle" sono per lo più
        // equivalenti
        if(gk == "angle_deg")
            globals.insert({"angle", deg_to_rad(val)});
        else if(gk == "angle_rad")
            globals.insert({"angle", val});
        else
            globals.insert({gk, val});
	}

	// monadi, monadi, monadi
	// nelle gonadi, gonadi, gonadi
	// ooooooooooooo
	// (da cantare a ritmo di: quei ragazzi della curva b)
	auto text_t_r = parse_texcoords_table(j["texcoords_table"]);
	if(text_t_r.is_err())
		return ERR_FURTHER("cannot parse tree json, "
						   "error while parsing texcoords table",
						   text_t_r.err_trace());

	auto thic_t_r = parse_thickness_table(j["thickness_table"]);
	if(thic_t_r.is_err())
		return ERR_FURTHER("cannot parse tree json, "
						   "error while parsing thickness table",
						   thic_t_r.err_trace());

	auto r_rules_r = parse_rewrite_rules(j["rewrite_rules"], globals);
	if(r_rules_r.is_err())
		return ERR_FURTHER("cannot parse tree json, "
						   "error while parsing rewrite rules json",
						   r_rules_r.err_trace());

	// we don't have a separate function for axiom and rewrite times since
	// they're both just atomic data, easier to inline
	if(auto rt = j["rewrite_times"]; !rt.is_number())
		return ERR("cannot parse tree json, rewrite_times value "
				   + rt.dump()
				   + " is of non numeric type "
				   + json_type_to_str(rt.type()));

	if(auto ax = j["axiom"]; !ax.is_string())
		return ERR("cannot parse tree json, axiom value "
				   + ax.dump()
				   + " required to be a string but is instead of type "
				   + json_type_to_str(ax.type()));

	auto text_t = text_t_r.get();
	auto thic_t = thic_t_r.get();
	auto r_rules = r_rules_r.get();
	size_t r_times = j["rewrite_times"].get<size_t>();

	std::string axiom_s = j["axiom"].get<std::string>();
	std::vector<instruction> axiom = {};
	for(const char c: axiom_s)
		axiom.push_back(interpret_json_char(c, globals));

	return OK(ParsedTree{text_t, thic_t,
						 axiom, r_rules, r_times,
						 globals});
}

res_fn(from_json, (const json& j),
	   std::map<std::string, ParsedTree>, std::string) {
    if(!j.is_object())
		return ERR("cannot parse json file, value in json file was not of "
				   "type object, it was instead of type: "
				   + json_type_to_str(j.type()));

	// controlla che tutte le chiavi siano presenti nel json
	// qui controlla quelle globali a livello di file
	// e quelle locali a livello di albero
	std::stringstream not_found {};
    const std::array required_keys = {
		"version",
    };
    const std::array required_tree_keys = {
		"thickness_table", "texcoords_table",
		"axiom", "rewrite_rules", "rewrite_times",
    };

	std::vector<std::string> tree_names;
    for(const auto&[k, v] : j.items())
	    // ogni chiave globale del file che non sia "version"
		// viene interpretata come albero
		if(k != "version")
			tree_names.push_back(k);

    for(const auto &k: required_keys)
		if(!j.contains(k))
			not_found << "required global key :\"" << k << "\"\n";

    for(const auto &tree_name : tree_names)
	    for(const auto &rtk: required_tree_keys)
			if(auto tree = j[tree_name]; !tree.contains(rtk))
				not_found<< "required tree key :\"" << rtk
						 << "\" in tree: \"" << tree_name<<"\n";

	std::string nf_str = not_found.str();
    if(nf_str.size() != 0)
		return ERR("the following required keys\n"
				   + nf_str
				   + "were not present in the json object");

    const auto version = j["version"];
    if(!version.is_number())
		return ERR("version field in object must be number "
				   "received instead version "
				   + version.dump()
				   + " which is of non numeric type "
				   + json_type_to_str(version));

    if(version != 1)
		return ERR("unsupported version number "
				   + version.dump()
				   + " supported versions are: [1]");

    std::cout<<"using json version "<<version<<std::endl;

    std::map<std::string, Result<ParsedTree, std::string>> tree_results;
	for(const std::string tree_name : tree_names)
		tree_results.insert({tree_name, parse_tree(j[tree_name])});

	std::stringstream not_parsed {};
	for(const auto &[k, v] : tree_results)
		if(!v.is_ok())
			not_parsed << "could not parse tree : \"" << k << "\" "
					   << "because of the following error:\n"
					   << v.string_trace()
					   << "\n";

	std::string np_str = not_parsed.str();
	if(np_str.size() != 0)
		return ERR_FURTHER(
			"at least one tree was malformed, could not parse json",
			{np_str});

	// e dopo aver controllato ogni errore sotto il sole, possiamo vedere
	// di fare sta mappa
    std::map<std::string, ParsedTree> trees;
	for(const auto &[k, v] : tree_results) {
		assert(v.is_ok());
		trees[k] = v.get();
	}

    return OK(trees);
}

res_fn(from_json_file, (const char* json_filename),
	   std::map<std::string, ParsedTree>, std::string) {
    auto jr = read_json_file(json_filename);
    if(!jr.is_ok())
		return ERR_FURTHER("could not parse json file", jr.err_trace());

	const auto j = jr.get();
	return from_json(j);
}

#endif // LSYSTEMS_JSON_HPP_
