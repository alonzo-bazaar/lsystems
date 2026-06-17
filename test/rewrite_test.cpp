#include "rewrite.hpp"
#include "gtest/gtest.h"

TEST(RewriteTest, TestEmptyInput) {
	ASSERT_EQ("", rewrite("", {{}}));

	ASSERT_EQ("", rewrite("F-", {
				RWP('F', ""),
				RWP('-', ""),
			}));

	ASSERT_EQ("", rewrite("F-", {
				RWP('F', ""),
				RWP('-', ""),
				RWP('+', ""),
				RWP('*', ""),
			}));
}

TEST(RewriteTest, TestEmptyOutput) {
	ASSERT_EQ("F-", rewrite("F-", {
				RWP('+', ""),
				RWP('*', ""),
			}));
}

TEST(RewriteTest, TestNormalFunctioning) {
	ASSERT_EQ("F-", rewrite("F-", {{}}));

	ASSERT_EQ("G-", rewrite("F-", {
				RWP('F', "G")
			}));

	ASSERT_EQ("F-", rewrite("F-", {RWP('f', "G")}));

	ASSERT_EQ("FF--", rewrite("F-", {
				RWP('F', "FF"),
				RWP('-', "--"),
			}));

	ASSERT_EQ("--FF", rewrite("F-", {
				RWP('F', "--"),
				RWP('-', "FF"),
			}));
}
