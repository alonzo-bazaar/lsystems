#pragma once
#ifndef LSYSTEMS_RESULT_HPP_
#define LSYSTEMS_RESULT_HPP_

#include<iostream>
#include<variant>
#include<list>
#include<functional>
#include<sstream>
#include<cmath>
#include<vector>

template<typename S, typename E>
struct Result {
	typedef S ok_t;
	typedef E err_t;
	std::variant<S, std::list<E>> inner;
	Result(S s):inner{std::in_place_index<0>, s}{}
	Result(E e):inner{std::in_place_index<1>, std::list<E>{e}}{}
	Result(std::list<E> el):inner{std::in_place_index<1>, el}{}

	static Result OK(S s) { return Result(s); }
	static Result ERR(E e) { return Result(e); }
	static Result ERR_FURTHER(E new_err, std::list<E> causing_trace) {
		causing_trace.push_front(new_err);
		return Result<S, E>{causing_trace};
	}

	bool is_ok() const { return inner.index() == 0; }
	bool is_err() const { return inner.index() == 1; }
	operator bool() const { return inner.index() == 0; }
	S get() const { return std::get<0>(inner); }
	E err() const { return std::get<1>(inner).front(); }
	std::list<E> err_trace() const { return std::get<1>(inner); }

	void log(std::ostream& outs=std::cout,
			 std::ostream& errs=std::cerr,
			 bool trail_nl = true) const {
		if(is_ok()) {
			outs<<"OK: " << get();
			if(trail_nl) outs<<std::endl;
		}
		else {
			errs<<"ERR: " << err();
			if(trail_nl) errs<<std::endl;
		}
	}
	void log_trace(std::ostream&os = std::cerr, bool trail_nl = true) const {
		bool first = true;
		for(const auto& elt : err_trace()) {
			if(first) {
				os<<elt;
				first = false;
			}
			else
				os<<"\n  Caused by : " << elt;
		}
		if(trail_nl)
			os<<std::endl;
	}

	std::string string_trace() const {
		std::stringstream ss;
		log_trace(ss, false);
		return ss.str();
	}
};

// define a res_fn
#define res_fn(name, params, ...)                           \
	struct name ## __hack {                                 \
		typedef Result<__VA_ARGS__> ret_t;                  \
		static ret_t name params ;                          \
	};                                                      \
	const std::function<name##__hack::ret_t params> name =  \
		name##__hack::name;                                 \
	name##__hack::ret_t name##__hack::name params

// can also be separated in two different macros to
// forward declare a res_fn
#define res_fn_decl(name, params, ...)                              \
	struct name ## __hack {                                         \
		typedef Result<__VA_ARGS__> ret_t;                          \
		static ret_t name params ;                                  \
	};                                                              \
    const extern std::function<name##__hack::ret_t params> name;

// define a res_fn
#define res_fn_defn(name, params, ...)                      \
	const std::function<name##__hack::ret_t params> name =  \
		name##__hack::name;                                 \
	name##__hack::ret_t name##__hack::name params


#define OK ret_t::OK
#define ERR ret_t::ERR
#define ERR_FURTHER ret_t::ERR_FURTHER

#endif // LSYSTEMS_RESULT_HPP_
