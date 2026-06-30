#pragma once

#include<vector>
#include<initializer_list>
#include<array>
#include<exception>
#include<sstream>
#include<iostream>
#include<variant>
#include<functional>
#include<concepts>
#include<algorithm>
#include"raylib.h" // per Color

constexpr float deg_to_rad(const float deg) {
    return (deg * PI) / 180.0f;
}

template<typename T>
std::vector<T> map_range(T start, T end, size_t n) {
    std::vector<T> res(n);
    double start_weight = 1.0;
    double end_weight = 0.0;
    double step = 1.0/(n-1);

    for(size_t i = 0; i<n; ++i) {
		res[i] = static_cast<T>((start * start_weight) + (end * end_weight));
		start_weight -= step;
		end_weight += step;
    }
    return res;
}

// workaround per permettere partial specializaton di template di funzioni
// in quanto questa non è presente nello standard c++ 17
#define def_range_mat(T, N)									\
    template<>												\
    std::vector<std::array<T, N>>							\
    map_range<std::array<T, N>> (std::array<T, N> start,	\
								 std::array<T, N> end,		\
								 size_t n) {				\
		std::array<std::vector<T>, N> arrs;					\
															\
		for(size_t i = 0; i<N; ++i)							\
			arrs[i] = map_range<T>(start[i], end[i], n);	\
															\
		std::vector<std::array<T, N>> res(n);				\
		for(size_t i = 0; i<n; ++i)							\
			for(size_t j = 0; j<N; ++j)						\
				res[i][j] = arrs[j][i];						\
															\
		return res;											\
    }

def_range_mat(float, 2)
// def_range_mat(float, 3)         // per Vector3 
def_range_mat(unsigned char, 4) // per Color

template<>
std::vector<Color> map_range<Color>(Color start, Color end, size_t n) {
    auto floats = map_range(std::array{start.r, start.g, start.b, start.a},
							std::array{end.r, end.g, end.b, end.a},
							n);

    std::vector<Color> res(n);
    for(size_t i = 0; i<n; ++i) {
		res[i].r = floats[i][0];
		res[i].g = floats[i][1];
		res[i].b = floats[i][2];
		res[i].a = floats[i][3];
    }
    return res;
}

template<typename T>
std::string serialize_vec(const std::vector<T> v,
						  const std::string open="[",
						  const std::string close="]",
						  const std::string separator=", ") {
	std::stringstream ss;
	ss << open;
	for(size_t i = 0; i<v.size(); ++i) {
		ss << v[i];
		if(i != v.size()-1)
			ss<<separator;
	}
	ss << close;
	return ss.str();
}

// https://cppreference.com/cpp/language/pack
// https://en.cppreference.com/cpp/experimental/constraints
template<typename Cont, typename Elt>
concept goddamn_container =
	requires(Cont c)
	{
		{ c.size() } -> std::convertible_to<std::size_t>;
		{ *(c.begin()) } -> std::convertible_to<Elt>;
		{ *(c.cbegin()) } -> std::convertible_to<const Elt>;
	};

// reinterpretatio che me tornava meglio di
// https://en.cppreference.com/cpp/concepts/invocable
template<typename Fn, typename Res, typename... Args>
concept invocable_with_res =
	requires(Fn fn, Args... args) {
		{ fn(args...) } -> std::same_as<Res>;
	};

template<typename Fn, typename Res, typename... Args>
concept vaguely_invocable_with_res =
	requires(Fn fn, Args... args) {
		{ fn(args...) } -> std::convertible_to<Res>;
	};

template<typename Arg, typename Fn>
Fn complement (Fn orig)
	requires vaguely_invocable_with_res<Fn, bool, Arg> {
	return static_cast<Fn>
		([orig](Arg arg){ return !orig(arg); });
}

template<typename InElt, typename OutElt, typename Fn, typename Cont>
std::vector<OutElt> mapcar(Cont in_c, Fn fn)
	requires
	invocable_with_res<Fn, OutElt, InElt> &&
	goddamn_container<Cont, InElt>
{
	std::vector<OutElt>res;
	res.reserve(in_c.size());
	for(const InElt& in_elt : in_c)
		res.push_back(fn(in_elt));
	return res;
}

template<typename T>
bool contains(const std::vector<T> t, const T elt) {
	return std::find(t.cbegin(), t.cend(), elt) != t.cend();
}

template<typename T, typename Fn>
bool contains_if(const T t, const Fn fn) {
	return std::find_if(t.cbegin(), t.cend(), fn) != t.cend();
}

template<typename T, typename Fn>
bool all(const T t, const Fn fn) {
	for(const auto& elt : t)
		if(!fn(t))
			return false;
	return true;
}

// indeed I am landfilling in the iota from std::views because for whatever
// goddamn reason I decided to do this project in the completely arbitrary
// standard of c++ 17, why do you ask?
std::vector<size_t>iota(size_t to, size_t from=0) {
	assert(from <= to);
	std::vector<size_t> res;
	res.reserve(to-from);
	for(size_t i = from; i<to; ++i)
		res.push_back(i);
	return res;
}

template<typename T, typename Container>
std::vector<T> suffix(size_t by, Container c) {
	std::vector<T> res;
	for(size_t i = by; i < c.size(); ++i)
		res.push_back(c[i]);
	return res;
}

// fixme: y no work :(
#define func(params, ...)										\
	(std::function<decltype(__VA_ARGS__)params> ([=] params {	\
		return __VA_ARGS__;										\
	}))

#define func_body(ret, params, ...)				\
	(std::function<ret params> ([=] params		\
								__VA_ARGS__		\
							   ))

/*
// frfr
#define func_nocap(params, ...)									\
	(std::function<decltype(__VA_ARGS__)params>([] params  {	\
		return __VA_ARGS__;										\
	}))

#define func_cap(params, ret ...)								\
	(std::function<decltype(ret)params>([__VA_ARGS__] params  {	\
		return ret;												\
	}))
*/
