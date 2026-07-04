// this is gonna be a bloodbath :D
#include "gtest/gtest.h"

#include "json.hpp"
using json = nlohmann::json;
#include "lsystem_json.hpp"

/*
struct ParsedTree {
	std::vector<std::array<float, 2>> texcoords_table;
	std::vector<float> thickness_table;
	std::string axiom;
	std::map<char, RewriteTarget> rewrite_rules;
	size_t rewrite_times;
	std::map<std::string, float> tree_globals;
};
 */

template<typename T>
void hcf_if_err(T result,
				ssize_t linenum = -1,
				std::string filename = "") {
	if(result.is_err()) {
		std::cerr << "some fatal shit just went down" << std::endl;
		if(linenum != -1)
			std::cerr << "at line: " << linenum << std::endl;
		if(filename.size() != 0)
			std::cerr << "in file: " << filename << std::endl;
		result.log_trace();
		FAIL();
	}
}

#define HCF_IF_ERR(res) (hcf_if_err(res, __LINE__, __FILE__))

#define ASSEQ(a, ...) ASSERT_EQ(a, (__VA_ARGS__))
#define ASSNE(a, ...) ASSERT_NE(a, (__VA_ARGS__))

TEST(JsonTest, DoesntBlowUp) {
	typedef std::vector<instruction> iv;
	json j = {
		{"version" , 1},
		{"fuck" , {
				{"texcoords_table", {{1, 1}}},
				{"thickness_table", {1}},
				{"axiom", "A"},
				{"rewrite_rules", {{"A", "AAAAAA"}}},
				{"rewrite_times", 1},
			}
		}
	};

	auto trees_r = from_json(j);

	// HCF_IF_ERR(trees_r);

	std::map<std::string, ParsedTree> trees = trees_r.get();
	auto fuck_i = trees.find("fuck");
	ASSNE(fuck_i, trees.end());

	auto fuck = fuck_i->second;
	ASSEQ(fuck.texcoords_table[0], std::array<float, 2>{1, 1});
	ASSEQ(fuck.thickness_table[0], 1);
	ASSEQ(fuck.rewrite_times, 1);
	ASSEQ(fuck.axiom, iv{{'A', {}}});
}


TEST(JsonTest, BasicRewrite) {
	typedef std::vector<instruction> iv;
	json j = 
		{{"texcoords_table", {{1, 1}}},
		 {"thickness_table", {1}},
		 {"axiom", "F"},
		 {"stride", 0.3},
		 {"rewrite_rules", {{"F", "ff"},
							{"f", "FF"}}},
		 {"rewrite_times", 1}};

	auto tree_r = parse_tree(j);
	// HCF_IF_ERR(tree_r);
	auto tree = tree_r.get();
	iv a = {{'F', {0.3}}};
	iv rw1 = {{'f', {0.3}}, {'f', {0.3}}};
	iv rw2 = {{'F', {0.3}}, {'F', {0.3}}, {'F', {0.3}}, {'F', {0.3}}};
	iv rw3 = {{'f', {0.3}}, {'f', {0.3}}, {'f', {0.3}}, {'f', {0.3}},
			  {'f', {0.3}}, {'f', {0.3}}, {'f', {0.3}}, {'f', {0.3}}};

	ASSEQ(a, tree.axiom);
	ASSEQ(rw1, rewrite_times(1, tree.axiom, tree.rewrite_rules));
	ASSEQ(rw2, rewrite_times(2, tree.axiom, tree.rewrite_rules));
	ASSEQ(rw3, rewrite_times(3, tree.axiom, tree.rewrite_rules));
}

TEST(JsonTest, ArithRewrite) {
	typedef std::vector<instruction> iv;
	json j = json::parse(R"({
		     "texcoords_table" : [[1, 1]],
		     "thickness_table" : [1],
		     "axiom" : "F",
		     "stride" : 1,
		     "rewrite_rules" : {
                      "F" : [["f" ,["+", ["param", 0], 1]],
                             ["f" ,["+", ["param", 0], 2]]],
                      "f" : [["F" ,["+", ["param", 0], 3]],
                             ["F" ,["+", ["param", 0], 10]]]
		     },
		     "rewrite_times" : 1
         })");

	auto tree_r = parse_tree(j);
	// // HCF_IF_ERR(tree_r);
	auto tree = tree_r.get();
	iv a = {{'F', {1}}};
	iv rw1 = {{'f', {2}}, {'f', {3}}};
	iv rw2 = {{'F', {5}}, {'F', {12}}, {'F', {6}}, {'F', {13}}};

	ASSEQ(a, tree.axiom);
	ASSEQ(rw1, rewrite_times(1, tree.axiom, tree.rewrite_rules));
	ASSEQ(rw2, rewrite_times(2, tree.axiom, tree.rewrite_rules));
}

TEST(JsonTest, EverythingTest) {
	typedef std::vector<instruction> iv;
    json j = json::parse(R"(
        {
            "version" : 1,
        
            "basic_tree" : {
                "stride" : 0.3,
                "angle_deg" : 22.5,
        
                "thickness_table" : {
                    "start" : 0.06,
                    "end"   : 0.015,
                    "steps" : 7
                },
        
                "texcoords_table" : {
                    "start" : [0.05, 0.05],
                    "end"   : [0.95, 0.95],
                    "steps" : 7
                },
        
                "axiom" : "A",
                "rewrite_times" : 7,
        
                "rewrite_rules" : {
                    "A" : ["[&FL!A]",
                           ["/", ["*", 5, "angle"]], "'",
                           "[&FL!A]",
                           ["/", ["*", 7, "angle"]], "'",
                           "[&FL!A]"],
                    "F" : [[0.5, "S", ["/", ["*", 4, "angle"]],
                            ["F", ["*", 2, "stride"]]],
                           [0.5, "S", ["/", ["*", 5, "angle"]],
                            ["F", ["*", 1, "stride"]]]],
                    "S" : "FL",
                    "L" : ["[ '''^^",
                           "    {",
                           "        -f+f+f-|-f+f+f",
                           "    }",
                           "]"]
                }
            }
        }
    )");

	auto trees_r = from_json(j);
	HCF_IF_ERR(trees_r);

    try {
        std::map<std::string, ParsedTree> trees = trees_r.get();
        auto tree = trees["basic_tree"];

        auto r1 = rewrite_times(1, tree.axiom, tree.rewrite_rules);
        auto r2 = rewrite_times(2, tree.axiom, tree.rewrite_rules);
        auto r3 = rewrite_times(3, tree.axiom, tree.rewrite_rules);

        float a = tree.globals["angle"];
        float s = tree.globals["stride"];

        ASSEQ (r1,
               iv({{'[', {}},
                   {'&', {a}}, {'F', {s}}, {'L', {}}, {'!', {}}, {'A', {}},
                   {']', {}},
                   {'/', {a * 5}}, {'\'', {}},
                   {'[', {}},
                   {'&', {a}}, {'F', {s}}, {'L', {}}, {'!', {}}, {'A', {}},
                   {']', {}},
                   {'/', {a * 7}}, {'\'', {}},
                   {'[', {}},
                   {'&', {a}}, {'F', {s}}, {'L', {}}, {'!', {}}, {'A', {}},
                   {']', {}}}));
    }
    catch(std::exception& e) {
        std::cerr<< "while rewriting\n"
                 << "caught exception:\n" << e.what()
                 << std::endl;
        FAIL();
    }
}
