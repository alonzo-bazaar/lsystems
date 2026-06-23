#include "rewrite.hpp"

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

