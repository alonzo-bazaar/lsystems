#include "rewrite.hpp"

// visto che ne io ne te si fa c++ da quarant'anni ti lascio come documentazione
// https://en.cppreference.com/cpp/container/map
// (per map in generale)
// https://en.cppreference.com/cpp/container/map/find
// (per come funziona find)
// in pratica map.find(coso) ritorna 
// - se coso non è presente in map ritorna map.end() per dire "un ce sta"
// - se coso è presente in map ritorna un "puntatore" a una std::pair dove
//   puntatore->first è coso (la chiave) e puntatore->second è il valore
//   associato a quella chiave all'interno di map
//   (c++ è strano in culo)

// per creare la stringa uso una stringstream
// https://en.cppreference.com/cpp/io/basic_stringstream
// è tipo cout ma scrive a una stringa interna invece che a terminale
// e puoi fare ss.str() alla fine per ottenere la stringa a cui scrive

// data una stringa axiom e una hash map transformations che dice
// "dopo sta trasformazione il carattere tot diventa la stringa tot"
// applica la trasformazione a tutti i caratteri della stringa, mettili insieme
// e ritorna la stringa che ne risulta

std::vector<instruction>
rewrite(const std::vector<instruction>& axiom,
		const std::map<char, RewriteTarget>& transformations) {
	std::vector<instruction> acc;

    for(const auto& i : axiom) {
		auto f = transformations.find(i.first);
		if(f != transformations.end()) {
			auto ex = f->second.expand(i.second);
			// https://stackoverflow.com/questions/2551775/appending-a-vector-to-a-vector
			acc.insert(acc.end(), ex.begin(), ex.end());
		}
		else
			acc.push_back(i);
    }
    return acc;
}

// idem di sopra ma fa il rewrite tot volte invece che una sola
std::vector<instruction>
rewrite_times (unsigned int times,
			   const std::vector<instruction>& axiom,
			   const std::map<char, RewriteTarget>& transformations) {
    if(times == 0)
		return axiom;
    return rewrite_times(times-1,
						 rewrite(axiom, transformations),
						 transformations);
}

