// this is gonna be a bloodbath :D
#include "gtest/gtest.h"

#include "json.hpp"
using json = nlohmann::json;
#include "lsystem_json.hpp"

TEST(JsonTest, DoesntBlowUp) {
	json j = {
		{"version" , 1},
		{"fuck" , {
				{"color_table", {{1, 1}}},
				{"thickness_table", {1}},
				{"axiom", "A"},
				{"rewrite_rules", {{"A", "AAAAAA"}}},
				{"rewrite_times", {1}},
			}
		}
	};

	auto trees_r = from_json(j);

	// bah, esageriamo
	EXPECT_TRUE(trees_r.is_ok());
	EXPECT_FALSE(trees_r.is_err());

	std::map<std::string, ParsedTree> trees = trees_r.get();
}
