#include "rewrite.hpp"
#include "gtest/gtest.h"

TEST(RewriteTest, TestEmptyInput) {
	ASSERT_EQ("", rewrite("", {{}}));

	ASSERT_EQ("", rewrite("F-", {
				{'F', ""},
				{'-', ""},
			}));

	ASSERT_EQ("", rewrite("F-", {
				{'F', ""},
				{'-', ""},
				{'+', ""},
				{'*', ""},
			}));
}

TEST(RewriteTest, TestEmptyOutput) {
	ASSERT_EQ("F-", rewrite("F-", {
				{'+', ""},
				{'*', ""},
			}));
}

TEST(RewriteTest, TestNormalFunctioning) {
	ASSERT_EQ("F-", rewrite("F-", {{}}));

	ASSERT_EQ("G-", rewrite("F-", {{'F', "G"}}));

	ASSERT_EQ("F-", rewrite("F-", {{'f', "G"}}));

	ASSERT_EQ("FF--", rewrite("F-", {
				{'F', "FF"},
				{'-', "--"},
			}));

	ASSERT_EQ("--FF", rewrite("F-", {
				{'F', "--"},
				{'-', "FF"},
			}));
}
