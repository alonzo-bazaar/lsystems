#include "rewrite.hpp"
#include "gtest/gtest.h"

#define RETURN_EMPTY_INSTRUCTION_VECTOR			\
	[](const std::vector<float>&ignored){		\
		(void)ignored;							\
		return std::vector<instruction>{};		\
	}

#define RETURN_FIXED_INSTRUCTION_VECTOR(...)			\
	[](const std::vector<float>&ignored){				\
		(void)ignored;									\
		return std::vector<instruction>{__VA_ARGS__};	\
	}

TEST(RewriteTest, TestEmptyOutput) {
	auto a = rewrite({}, {});
	ASSERT_EQ(0, a.size());

	auto b = rewrite({{'F',{}}, {'-',{}}}, {
			RWP('F', RETURN_EMPTY_INSTRUCTION_VECTOR),
			RWP('-', RETURN_EMPTY_INSTRUCTION_VECTOR),
		});
	ASSERT_EQ(0, b.size());

	auto c = rewrite({{'F',{}}, {'-',{}}}, {
			RWP('F', RETURN_EMPTY_INSTRUCTION_VECTOR),
			RWP('-', RETURN_EMPTY_INSTRUCTION_VECTOR),
			RWP('+', RETURN_EMPTY_INSTRUCTION_VECTOR),
			RWP('*', RETURN_EMPTY_INSTRUCTION_VECTOR),
		});
	ASSERT_EQ(0, c.size());
}

TEST(RewriteTest, TestNoMatchingRule) {
	std::vector<instruction>e = {{'F',{}}, {'-',{}}};
	auto a1 = rewrite(e, {
			RWP('+', RETURN_EMPTY_INSTRUCTION_VECTOR),
			RWP('*', RETURN_EMPTY_INSTRUCTION_VECTOR),
		});
	ASSERT_EQ(e, a1);

	auto a2 = rewrite(e, {});
	ASSERT_EQ(e, a2);
}

TEST(RewriteTest, TestNormalFunctioning) {
	std::vector<instruction>e1 = {{'G',{}}, {'-',{}}};
	std::vector<instruction>a1 = rewrite({{'F',{}}, {'-',{}}}, {
			RWP('F', RETURN_FIXED_INSTRUCTION_VECTOR({'G', {}})),
		});
	ASSERT_EQ(e1, a1);

	std::vector<instruction>e2 = {{'F',{}}, {'-',{}}};
	std::vector<instruction>a2 = rewrite({{'F',{}}, {'-',{}}}, {
			RWP('f', RETURN_FIXED_INSTRUCTION_VECTOR({'G', {}})),
		});
	ASSERT_EQ(e2, a2);

	std::vector<instruction>e3 = {{'F',{}}, {'F',{}}, {'-',{}}, {'-',{}}};
	std::vector<instruction>a3 = rewrite({{'F', {}}, {'-', {}}}, {
			RWP('F', RETURN_FIXED_INSTRUCTION_VECTOR({'F', {}}, {'F', {}})),
			RWP('-', RETURN_FIXED_INSTRUCTION_VECTOR({'-', {}}, {'-', {}}))
		});
	ASSERT_EQ(e3, a3);

	std::vector<instruction>e4 = {{'-',{}}, {'-',{}}, {'F',{}}, {'F',{}}};
	std::vector<instruction>a4 = rewrite({{'F', {}}, {'-', {}}}, {
			RWP('F', RETURN_FIXED_INSTRUCTION_VECTOR({'-', {}}, {'-', {}})),
			RWP('-', RETURN_FIXED_INSTRUCTION_VECTOR({'F', {}}, {'F', {}}))
		});
	ASSERT_EQ(e4, a4);
}

TEST(RewriteTest, TestWaitHowDoValuesWork) {
	std::vector<instruction>e1 = {{'G',{10}}, {'-',{}}};
	std::vector<instruction>a1 = rewrite({{'F',{5}}, {'-',{}}}, {
			RWP('F', [](const std::vector<float>& v){
				return std::vector<instruction>{{'G', {v[0]*2}}};
			})});
	ASSERT_EQ(e1, a1);
}
