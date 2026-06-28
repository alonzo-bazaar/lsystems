#pragma once

#include<vector>
#include<initializer_list>
#include<array>
#include<exception>
#include<sstream>
#include<iostream>
#include<variant>
#include<functional>
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
template<typename Ret, typename... Args>
std::function<Ret(Args... args)> complement
(std::function<Ret(Args... args)> orig) {
	return [orig](Args... args){ return !orig(args...); };
}

// support for std::functions but also for like... functions
template<typename Ret, typename... Args>
std::function<Ret(Args... args)> complement
(Ret(*orig)(Args... args)) {
	return [orig](Args... args){ return !orig(args...); };
}

// no clue wether std::transform works
// https://en.cppreference.com/cpp/algorithm/transform
// since res does initially contain 0 elements so... can't really
// transform into it
template<typename In, typename Out, typename Collection>
std::vector<Out>
mapcar(std::function<Out(In)> fn, Collection in) {
	std::vector<Out>res;
	res.reserve(in.size());
	for(const In& in_elt : in)
		res.push_back(fn(in_elt));
	return res;
}

// same as above, also support plain ol' function pointers
// should probalby just have a template with callable shit
template<typename In, typename Out, typename Collection>
std::vector<Out>
mapcar(Out(*fn)(In), Collection in) {
	std::vector<Out>res;
	res.reserve(in.size());
	for(const In& in_elt : in)
		res.push_back(fn(in_elt));
	return res;
}

template<typename T, typename Elt>
bool contains(const T& t, const Elt& elt) {
	return std::find(t.cbegin(), t.cend(), elt) != t.cend();
}

template<typename T, typename Fn>
bool contains_if(const T& t, const Fn& fn) {
	return std::find_if(t.cbegin(), t.cend(), fn) != t.cend();
}

/*
template<typename T>
std::vector<T>vec_cat2(std::vector<T> v1, std::vector<T> v2) {
	std::vector v3(v1);
	v3.insert(v3.end(), v2.begin(), v2.end());
	return v3;
}

// hic est reimplementatio de
// https://doc.rust-lang.org/std/result/
// in c++ (fatta male)
// S = success type, E = error type
// 
// come filosofia generale di questa classe Result
// (raggiunta dopo lunghe lotte col compiler, non era affatto il mio piano
//  iniziale)
// 
// c++ la prende bene se gli lasciamo decidere implicitamente che
// dato un return type Result<A, B>
// allora return A vuol dire ok
// e return B vuol dire che qualcosa non va bene
// 
// ma se proviamo a specificare
// return Result(A), oppure
// return Result::ok(a), oppure
// return Ok(a)
// per tutte queste tre alternative, s'incazza
// quindi il modo più facile per usare questa classe result è lasciare che
// c++ faccia implicitamente la conversione di qualsiasi cosa e manco provare
// a essere espliciti
// 
// inoltre
// spesso si usano stringhe come valori di ritorno
// spesso si usano stringhe come messaggi di errore
// e spesso si usa, come stringa o valore di ritorno, un valore che può essere
// convertito implicitamente da stringa
// 
// se ci si affida quindi alla conversione implicita, e non abbiamo altra
// scelta, il compiler non riesce a distinguere cosa è cosa
// 
// per ovviare ciò sono forniti i seguenti struct wrapper il cui unico scopo
// è rendere esplicito al compiler "ue fra, da questo lato del result"
// (resi generici visto che non costa manco troppo da fare)
// 
// sia avvisato l'utente che l'uso di questa classe result porta a tempi
// di compilazione che non invidio al rust, e a messaggi di errure di
// lunghezze della madonna 
// la uso nel mio codice solo per sunk cost fallacy a sto punto
template <typename T>
struct Ok {
	const T t;
	Ok(const T t):t(t){}
	const T get() { return t; }
	operator T() const noexcept { return t; }

	std::ostream& operator<< (std::ostream& os) const {
		return os<<"Ok{"<<t<<"}";
	}
	std::stringstream& operator<< (std::stringstream& ss) const {
		return ss<<t;
	}
};

template <typename T>
struct Err {
	const T t;
	Err(const T t):t(t){}
	const T get() { return t; }
	operator T() const noexcept { return t; }

	std::ostream& operator<< (std::ostream& os) const {
		return os<<"Err{"<<t<<"}";
	}
	std::stringstream& operator<< (std::stringstream& ss) const {
		return ss<<t;
	}
};

template <typename S, typename E>
struct Result {
	const bool is_ok;
	std::variant<S, std::vector<E>> data;
	// errors represented internally as a cascade of events
	// ergo, collection

	Result(Ok<S> s) :data(s), is_ok(true){}
	Result(S s) :data(s), is_ok(true){}

	Result(Err<E> s) :data(s), is_ok(true){}
	Result(E e) :data(std::vector<E>{e}), is_ok(false){}

	Result(const std::vector<E>& ev) :data(ev), is_ok(false){}

	Result(const std::initializer_list<E>& ev) :data(ev), is_ok(false){}

	bool ok() const noexcept { return is_ok; }
	operator bool() const noexcept { return is_ok; }

	// together with vector initializer serves to implicitly convert
	// error
	std::vector<E> furthermore(E e) {
		std::vector<E> cpy;
		cpy.push_back(e);
		for(const auto& c: err_data())
			cpy.push_back(c);
		return cpy;
	}

	S get() const { return std::get<0>(data); }
	E err() const { return std::get<1>(data)[0]; }
	std::vector<E> err_data() const { return std::get<1>(data); }

	operator S() const noexcept { return get(); }
	operator E() const noexcept { return err(); }

	S get_or(const S def) const noexcept {
		try {
			if(ok())
				return get();
			return def;
		}
		catch(std::exception &e) {
			return def;
		}
	}

	std::ostream& operator<< (std::ostream& os) const {
		if(is_ok)
			return os<<"Result::Ok{" << get() << "}";
		else
			return os<<"Result::Err{" << err() << "}";
	}
	std::string err_trace() const {
		std::stringstream ss; 
		ss << std::string{err()};
		for(auto c = err_data().begin()+1; c != err_data().end(); c++)
			ss << "  Caused by: " << std::string{*c};

		return ss.str();
	}
	~Result() = default;
};


template<typename S1, typename S2, typename E>
Result<S1, E> further_error(const Result<S2, E>& r, const E e) {
	auto errcpy = r.err_data();
	errcpy.push_back(e);
	return errcpy;
}
*/
