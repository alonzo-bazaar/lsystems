#pragma once

#include <fstream>
#include <exception>
#include <memory>
#include <functional>
#include <string>

#include <cassert>

#include "utils.hpp"
#include "result.hpp"
#include "rewrite.hpp"

#include "json.hpp"
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

// tree json contains these
struct ParsedTree {
	std::vector<std::array<float, 2>> texcoords_table;
	std::vector<float> thickness_table;
	std::map<char, RewriteTarget> rewrite_rules;
	size_t rewrite_times;
};

/*
  accepted texcoord_table formats are
  1. list of couples of numbers
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

// generic function to express both thickness table parsing and texcoord
// table parsing as a function of
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
				auto f = std::find_if(j.begin(), j.end(),
									  complement(validator));
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
				return OK(mapcar(j, transformer));
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

bool is_json_valid_texcoord(const json& j) {
	return j.is_array()
		&& j.size() == 2
		&& j[0].is_number()
		&& j[1].is_number();
}

std::array<float, 2> valid_json_to_texcoord (const json& j) {
	assert(is_json_valid_texcoord(j));
	return {j[0], j[1]};
}

bool is_json_valid_thickness(const json& j) {
	return j.is_number() && static_cast<float>(j) >= 0;
}

float valid_json_to_thickness(const json& j) {
	assert(is_json_valid_thickness(j));
	return static_cast<float>(j);
}

const auto parse_texcoord_table = parse_table<std::array<float, 2>>
	(is_json_valid_texcoord,
	 valid_json_to_texcoord,
	 "texcoord",
	 "texture coordinate (ie: an array of 2 numbers)");

const auto parse_thickness_table = parse_table<float>
	(is_json_valid_thickness,
	 valid_json_to_thickness,
	 "thickness",
	 "thickness value (ie: any non negative number)");

/*
  parse rewrite_rules json
  all keys must be strings of length 1
  all values can be
  - strings (default parameters, if any, inferred from globals map)
  - lists of parameterized instructions interspersed with strings
    parameterized instructions are here refered to as calculator expressions
 */

/*
  valid calculator expressions are
  - all numbers (literals)
  - all strings (variable lookup)

  then we got arithmetic expressions (expressed in json lists)
  the big 4
  - ["+", ...args]
  - ["*", ...args]
  - ["-", arg1, arg2]
  - ["/", arg1, arg2]

  conditionals
  - ["if", cond, then, else]

  strict compairisons
  - [">", arg1, arg2]
  - [">=", arg1, arg2]
  - ["<", arg1, arg2]
  - ["<=", arg1, arg2]
  - ["==", arg1, arg2]

  toleranced compairisons
  - ["<=", arg1, arg2, tolerance]
  - [">=", arg1, arg2, tolerarnce]
  - ["==", arg1, arg2, tolerance]

  (note: strinc compairisons will be implemented in terms of toleranced
   compairisons with a low (but not really) tolearnce of, say, 0.001)
 */

// preliminary syntactic ensurance
res_fn(validate_calculator_expression, (const json& j), bool, std::string) {
	if(j.is_number()) // literal
		return OK(true);
	if(j.is_string()) // variable lookup
		return OK(true);
	else if(j.is_array() && j.size()>0) { // composite expression
		auto op = j["0"];
		if(!op.is_string())
			return ERR("operator is not a string");
		if(!contains({"+", "-", "*", "/",
					  ">", "<", "==", ">=", "<=",
					  "if", "param"}, op))
			return ERR("unrecognized operator " + op.dump());

		for(size_t i = 1; i<j.size(); i++)
			if(auto r = validate_calculator_expression(j[i]);
			   !r.is_ok())
				return ERR_FURTHER("invalid operand at index "
								   + std::to_string(i),
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
					   + std::to_string(rands_size));
		return OK(true);
	}
	return ERR("expected calculator expression to be either "
			   " literal(number), variable (string), or expression(array) "
			   " was given an expression of unrecognized type " +
			   json_type_to_str(j.type()));
}

bool is_valid_calculator_expression(const json& j, bool log_invalid=true) {
	auto r = validate_calculator_expression(j);
	if(r.is_ok())
		return r.get();
	if(log_invalid) {
		std::cerr<<"invalid json!\n";
		r.log_trace();
	}
	return false;
}

constexpr float default_tolerance = 0.0001;
// https://stackoverflow.com/a/4598865
bool float_eq(float a, float b, float tolerance=default_tolerance) {
    return (std::abs(a - b)
			<= tolerance * std::max(std::abs(a), std::abs(b)));
}
bool rands_eq(std::vector<float> rands) {
	if(rands.size() == 2) return float_eq(rands[0], rands[1]);
	else return float_eq(rands[0], rands[1], rands[2]);
}

// (a json generated rewrite target will contain one or more lambdas
//  invoking this function)
res_fn(evaluate_calculator_expression,
	   (const json& expr,
		const std::map<std::string, float>& globals,
		const std::vector<float>& params),
	   float, std::string) {
	// numbers are self evaluating in calculator expressions
	if(expr.is_number())
		return OK(expr);

	// strings are variable lookups
	if(expr.is_string()) {
		if(auto f = globals.find(expr); f != globals.end())
			return OK(f->second);
		else
			return ERR("variable " +
					   expr.dump()
					   + " is not defined within this lsystem specification");
	}

	// arrays are composite expressions
	if(expr.is_array()) {
		// operator
		const json &op_j = expr[0];
		if(!op_j.is_string()) return ERR
								  ("calculator expression operator "
								   + expr.dump()
								   + " is not a string, cannot interpret it"
								   + " as an operator");
		std::string op = op_j;

		// operands
		auto rand_results = mapcar
			(iota(expr.size(), 1),
			 std::function<ret_t(size_t)>([expr, globals, params](size_t i) {
				 return evaluate_calculator_expression
					 (expr[i], globals, params);
			 }));
		auto first_rand_err = std::find_if(rand_results.begin(),
										   rand_results.end(),
										   [](ret_t r){return r.is_err();});
		if(first_rand_err != rand_results.end())
			return ERR_FURTHER
				("cannot parse calculator expression: "
				 "encountered while evaluating expression operand",
				 first_rand_err->err_trace());

		// if we get here all results are ok
		std::vector<float> rands = mapcar
			(rand_results,
			 std::function<ret_t::ok_t(const ret_t&)>([](const ret_t& t){
				 return t.get();
			 }));
#ifdef expect_size
#error "lol change this macro's name then"
#endif
#define expect_size(pred, desc)									\
		if(!(rands.size() pred))								\
			return ERR("\"" +  op +  "\" expression "			\
					   "expected " +  desc + " arguments"		\
					   " but " + std::to_string(rands.size())	\
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
						   + " of parameter array of length "
						   + std::to_string(rands.size()));
			return OK(params[i]);
		}
		return ERR("unrecognized operator [" + op + "]");
#undef expect_size
	}
	return ERR("this bit was supposed to be unreachable,"
			   " ask gdb how you got here, I have no clue");
}

bool is_valid_rewrite_target(const json& j,
							 const std::map<std::string, float>& globals) {
	if(j.is_string())
		return true;

	if(j.is_array())
		return
			// deterministic rewrite rule
			// (array of rewrite rule elements)
			all(j, std::function<bool(const json&)>([](const json& jj) {
				return
					jj.is_string() ||
					(jj.is_array() &&
					 jj[0].is_string() &&
					 jj[0].size() == 1 &&
					 all(iota(jj.size(), 1),
						 std::function<bool(size_t)>([jj](size_t i) {
							 return is_valid_calculator_expression(jj[i]);
						 })));}))
			// stochastic rewrite rule
			// (array of arrays where each array
			//  startings with a probability number and is then followed by
			//  0 or more rewrite rule elements)
			|| all(j, std::function<bool(const json&)>([](const json& jj) {
				return jj.is_array()
					&& jj.size() > 0
					// number has to be literal number
					&& jj[0].is_number()
					&& all(iota(jj.size(), 1),
						   std::function<bool(size_t)>([jj] (size_t i) {
							   return is_valid_calculator_expression(jj[i]);
						   }));}));

	return false;
}

// character is provided per se and may be returned with or without
// parameters
// this is the function that handles how F or + or - instructions
// may be initialized with implicit parameters from `globals` 
instruction interpret_json_char
(const char c, const std::map<std::string, float>& globals) {
	switch(c) {
		// these instructions don't take any parameter
	case '[':
	case ']':
	case '{':
	case '}':
	case '|':
	case '!':
	case '\'':
		return {c, {}};
		// f and F when given no parameters are initialized with
		// a predefined stride, which is present in the lsystem spec under
		// the key "stride"
	case 'F':
	case 'f':
		if(auto f = globals.find("stride"); f != globals.end())
			return {c, {f->second}};
		throw std::runtime_error
			("given instruction with implicit stride parameter but "
			 "the \"stride\" variable is not present in the lsystem");
		// + - & ^ / and \ when given no params take instead the default
		// parameter "angle"
	case '+':
	case '-':
	case '&':
	case '^':
	case '/':
	case '\\':
		if(auto f = globals.find("stride"); f != globals.end())
			return {c, {f->second}};
		throw std::runtime_error
			("given instruction with implicit angle parameter but "
			 "the \"angle\" variable is not present in the lsystem");
	default:
		return {c, {}};
	}
}

transition json_to_transition(const json& j,
							  const std::map<std::string, float> globals) {
	if(j.is_string())
		return [j, globals](std::vector<float> params) {
			std::vector<instruction> res {};
			for(const char c : j.get<std::string>()) 
				res.push_back(interpret_json_char(c, globals));
			return res;
		};
	
	else if(j.is_array())
		return [j, globals](std::vector<float> params) {
			std::vector<instruction> res {};
			for (const auto &[_, elt] : j.items()) {
				if(elt.is_string())
					for(const char c : j.get<std::string>()) 
						res.push_back(interpret_json_char(c, globals));

				else if(elt.is_array()
						&& elt[0].is_string()
						&& elt[0].size() == 1) {
					const char c = elt[0].get<std::string>()[0];
					std::vector<float> vals;
					for(size_t i = 1; i<elt.size(); ++i) {
						auto r = evaluate_calculator_expression
							(elt[i], globals, params);
						if(r.is_ok())
							vals.push_back(r.get());
						else throw std::runtime_error
								 ("could not evaluate arithmetic expression"
								  " within the following lsystem json bit:\n"
								  + elt.dump()
								  + "\nwithin the following lsystem json bit:\n"
								  + j.dump()
								  + "\ndue to the following error:\n"
								  + r.string_trace());
					}
					res.push_back({c, vals}); 
				}
				else
					throw std::runtime_error
						("malformed json rewrite instruction:\n"
						 + elt.dump());
			}
			return res;
		};
	else
		throw std::runtime_error
			("malformed json rewrite target:\n"
			 + j.dump());
}

res_fn(parse_rewrite_target,
	   (const json& j, const std::map<std::string, float>& globals),
	   RewriteTarget, std::string) {
	// rewrite target can be either a single rewrite target or a list
	if(!is_valid_rewrite_target(j, globals))
		return ERR("invalid rewrite target");

	// is stochastic?
	// easier to isolate than checking for deterministic rewrite targets 
	if(j.is_array() &&
	   all(j, std::function<bool(const json&)>([](const json& jj) {
		   return jj[0].is_number();
	   }))) {
		std::vector<std::pair<float, transition>> probs;
		for(const auto& [_, v]:j.items())
			probs.push_back({v[0],
							 json_to_transition(json(suffix<json>(1, v)),
												globals)});
		return RewriteTarget(probs);
	}	

	// otherwise is deterministic
	return RewriteTarget(json_to_transition(j, globals));
}

res_fn(parse_rewrite_rules,
	   (const json& j, const std::map<std::string, float>& globals),
	   std::map<char, RewriteTarget>, std::string) {
	assert(0 && "TODO");
}

res_fn(parse_tree, (const json& j), ParsedTree, std::string) {
	assert(0 && "TODO");
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
		"color_table", "thickness_table",
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
				   + "were not present in the json file");

    const auto version = j["version"];
    if(!version.is_number())
		return ERR("version field in object must be number "
				   "received instead version "
				   + version.dump()
				   + " which is of type "
				   + json_type_to_str(version));

    if(version != 1)
		return ERR("unsupported version number "
				   + version.dump()
				   + " supported versions are: [1]");

    std::cout<<"using json version "<<version<<std::endl;

	return std::map<std::string, ParsedTree>{};

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
