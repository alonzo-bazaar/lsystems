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
std::string rewrite (const std::string& axiom,
					 const std::map<char, std::string>& transformations) {
	std::stringstream ss;
	for(char c : axiom) {
		auto f = transformations.find(c);
		if(f != transformations.end())
			ss << f->second;
		else
			ss << c;
	}
	return ss.str();
}

// idem di sopra ma fa il rewrite tot volte invece che una sola
std::string rewrite_times (unsigned int times,
						   const std::string& axiom,
						   const std::map<char, std::string>& transformations) {
	if(times == 0)
		return axiom;
	return rewrite_times(times-1,
						 rewrite(axiom, transformations),
						 transformations);
}
