#pragma once

#include <fstream>
#include <exception>
#include <cassert>
#include <memory>
#include <functional>
#include <string>

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

res_fn(json_from_file, (const char* filename), json, std::string) {
    std::ifstream f(filename);
    try {
		const json j = json::parse(f);
		f.close();
		return OK(j);
    }
    catch(std::exception &e) {
		f.close();
		return ERR(e.what());
    }
    assert(0 && "unreachable");
}

// tree json contains these
struct ParsedTree {
	std::vector<std::array<float, 2>> texcoords_table;
	std::vector<float> thickness_table;
	std::map<char, RewriteTarget> rewrite_rules;
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

// utility for key checking 
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


bool is_json_valid_texcoord(const json& elt) {
	return elt.is_array()
		&& elt.size() == 2
		&& elt[0].is_number()
		&& elt[1].is_number();
}
std::array<float, 2> valid_json_to_texcoord (const json& elt) {
	return {elt[0], elt[1]};
}

template<typename T>
std::function<Result<std::vector<T>, std::string>(const json& j)> parse_table
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
				return OK(mapcar(transformer, j));
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

auto parse_texcoord_table = parse_table<std::array<float, 2>>
	(is_json_valid_texcoord,
	 valid_json_to_texcoord,
	 "texcoord",
	 "texture coordinate (ie: an array of 2 numbers)");

auto parse_thickness_table = parse_table<float>
	([](const json& j){ return j.is_number() && static_cast<float>(j) >= 0; },
	 [](const json& j){ return static_cast<float>(j); },
	 "thickness",
	 "thickness value (ie: any non negative number)");

res_fn(parse_tree, (const json& j), ParsedTree, std::string) {
	assert(0 && "TODO");
}

res_fn(from_json, (const char* json_filename),
	   std::map<std::string, ParsedTree>, std::string) {
    // https://github.com/nlohmann/json#read-json-from-a-file
    auto jr = json_from_file(json_filename);
    if(!jr.is_ok())
		return ERR_FURTHER("could not parse json file", jr.err_trace());

	const auto j = jr.get();
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
