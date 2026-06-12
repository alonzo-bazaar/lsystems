#pragma once

#include<map>
#include<string>
#include<sstream>

std::string rewrite (const std::string& axiom,
					 const std::map<char, std::string>& transformations);

std::string rewrite_times (unsigned int times,
						   const std::string& axiom,
						   const std::map<char, std::string>& transformations);
