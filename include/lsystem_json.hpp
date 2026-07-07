#pragma once
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

std::string json_type_to_str(const json::value_t t);

res_fn_decl(read_json_file, (const char* filename), json, std::string);

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

// utilities for key checking 
// (to be utilized in conjunction with serialize_vec for error reporting)
std::vector<std::string> missing_keys
(const json& j, const std::vector<std::string>& required_keys);
std::vector<std::string> excess_keys
(const json& j, const std::vector<std::string>& required_keys);

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

bool is_json_valid_texcoords(const json& j);
std::array<float, 2> valid_json_to_texcoords (const json& j);
bool is_json_valid_thickness(const json& j);
float valid_json_to_thickness(const json& j);

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

res_fn_decl(validate_arithmetic_expression, (const json& j), bool, std::string);
bool is_valid_arithmetic_expression(const json& j);

// https://stackoverflow.com/a/4598865
constexpr float default_tolerance = 0.0001;
bool float_eq(float a, float b, float tolerance=default_tolerance);
bool rands_eq(std::vector<float> rands);

res_fn_decl(evaluate_arithmetic_expression,
       (const json& expr,
        const std::map<std::string, float>& globals,
        const std::vector<float>& params),
       float, std::string);
res_fn_decl(validate_rewrite_target, (const json& j), bool, std::string);

bool is_valid_rewrite_target(const json& j);
instruction interpret_json_char
(const char c, const std::map<std::string, float>& globals);
std::vector<instruction> json_eval_string(const std::string& j_str,
                                          const std::map<std::string, float>& globals,
                                          const std::vector<float>& params);
std::vector<instruction> json_eval_array(const json& j,
                                         const std::map<std::string, float>& globals,
                                         const std::vector<float>& params);
transition json_to_transition(const json& j,
                              const std::map<std::string, float>& globals);
res_fn_decl(parse_rewrite_target,
       (const json& j, const std::map<std::string, float>& globals),
       RewriteTarget, std::string);
res_fn_decl(parse_rewrite_rules,
       (const json& j, const std::map<std::string, float>& globals),
       std::map<char, RewriteTarget>, std::string);
res_fn_decl(parse_tree, (const json& j), ParsedTree, std::string);
res_fn_decl(from_json, (const json& j),
	   std::map<std::string, ParsedTree>, std::string);
res_fn_decl(from_json_file, (const char* json_filename),
	   std::map<std::string, ParsedTree>, std::string);

#endif // LSYSTEMS_JSON_HPP_
