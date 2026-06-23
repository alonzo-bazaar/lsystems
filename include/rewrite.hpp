#pragma once

#include<map>
#include<string>
#include<sstream>

#include<memory>
#include<cassert>    // per assert
#include<functional> // per std::function

// per evitare di avere type declaration troppo geroglifiche
typedef std::pair<char, std::vector<float>> instruction;
typedef std::function<std::vector<instruction>(std::vector<float>)> transition;

// RewriteTarget è definita tutta nel file .hpp in quanto la struttura di
// questa in sottoclassi è particolarmente legata alla semantica dei suoi varii
// methodi quindi separarla in file .hpp e .cpp porterebbe a una perdita di
// chiarezza che non vale molto la pena per una compilazione un po' più veloce
class RewriteTarget {
public:
	RewriteTarget(const transition& t)
		:inner(std::make_shared<Deterministic>(t)) {}
	RewriteTarget(const std::vector<std::pair<float, transition>>& t_probs)
		:inner(std::make_shared<Random>(t_probs)) {}

	std::vector<instruction> expand(std::vector<float> args) const {
		return inner->expand(args);
	}

private:
	class Inner {
	public:
		virtual std::vector<instruction> expand(const std::vector<float>&)
			const = 0;
	};

	class Deterministic : public Inner {
	public:
		Deterministic(const transition& t):t(t){}
		virtual std::vector<instruction>
		expand(const std::vector<float>& args) const override {
			return t(args);
		}
	private:
		const transition t;
	};

	class Random : public Inner {
	public:
		Random(const std::vector<std::pair<float, transition>>& t_probs)
			:t_probs(t_probs) {
			// inizializzare t_probs come vuota è un'errore abbastanza fatale
			// quindi si gestisce con un panic direttamente
			assert(t_probs.size() > 0
				   && "can't have a map with negative number of elements!");
		}
		virtual std::vector<instruction>
		expand(const std::vector<float>& args) const override {
			// random uniform tra 0 e 1
			float r = (double)rand()/RAND_MAX;
			// scegli a caso un elemento p.second dato p in t_probs
			// con probabilità pari a p.first per ogni elemento di t_probs
			// (a patto che la somma di tutti i p.first faccia 1
			//  che altrimenti kolmogorov si arrabbia)
			size_t picked = 0;
			for(auto const& p: t_probs) {
				if(r < p.first) {
					return p.second(args);
				}
				else r -= p.first;
				picked++;
			}
			// come fallback se il loop di sopra non rende niente
			// (non si sa mai quando usi i float)

			// rbegin ritorna un puntatore/iteratore all'ultimo elemento
			return t_probs.rbegin()->second(args);
		}
	private:
		const std::vector<std::pair<float, transition>> t_probs;
	};

	const std::shared_ptr<Inner> inner;
};

// rewrite pair
// shortcut di convenience
#define RWP(a, ...) {a, RewriteTarget(__VA_ARGS__)}

std::vector<instruction>
rewrite(const std::vector<instruction>& axiom,
		const std::map<char, RewriteTarget>& transformations);

std::vector<instruction>
rewrite_times (unsigned int times,
			   const std::vector<instruction>& axiom,
			   const std::map<char, RewriteTarget>& transformations);
