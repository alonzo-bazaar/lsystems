#pragma once

#include<map>
#include<string>
#include<sstream>

#include<memory>
#include<cassert>

class RewriteTarget {
public:
	RewriteTarget(const std::string& s) {
		inner = std::make_shared<Deterministic>(s);
	}
	RewriteTarget(const std::map<float, std::string>& probs) {
		inner = std::make_shared<Random>(probs);
	}
	std::string expand() const {
		return inner->get();
	}

private:
	class Inner {
	public:
		virtual std::string get() const = 0;
	};

	class Deterministic : public Inner {
	public:
		Deterministic(const std::string& s):s(s){}
		virtual std::string get() const override {
			return s;
		}
	private:
		std::string s;
	};

	class Random : public Inner {
	public:
		Random(const std::map<float, std::string>& probs) :probs(probs) {
			// inizializzare vuota sta roba è un'errore fatale quindi
			// manco sto a fa "graceful failure handling"
			assert(probs.size() > 0
				   && "can't have a map with negative number of elements!");
		}
		virtual std::string get() const override {
			// random uniform tra 0 e 1
			float r = (double)rand()/RAND_MAX;
			for(auto const p: probs) {
				if(r < p.first) return p.second;
				else r -= p.first;
			}
			// rbegin ritorna un puntatore/iteratore all'ultimo elemento
			return probs.rbegin()->second;
		}
	private:
		std::map<float, std::string> probs;
	};

	std::shared_ptr<Inner> inner;
};

// rewrite pair
#define RWP(a, ...) {a, RewriteTarget(__VA_ARGS__)}

std::string rewrite (const std::string& axiom,
					 const std::map<char, RewriteTarget>& transformations);

std::string rewrite_times (unsigned int times,
						   const std::string& axiom,
						   const std::map<char, RewriteTarget>& transformations);
