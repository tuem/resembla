/*
paramset: A Parameter Management Utility for C++
https://github.com/tuem/paramset

Copyright 2017 Takashi Uemura

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#ifndef PARAMSET_HPP
#define PARAMSET_HPP

#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>

#include "cmdline.h"
#include "json.hpp"

namespace paramset{

// internal representation of parameters
struct parameter{
	std::string value;

	parameter(){}
	parameter(const char* value): value(value){}
	parameter(const std::string& value): value(value){}
	template<typename T> parameter(const T& t): value(std::to_string(t)){}

	operator std::string() const{
		return value;
	}

	operator int() const{
		return std::stoi(value);
	}

	operator double() const{
		return std::stod(value);
	}

	operator bool() const{
		if(value.empty() || value == "0")
			return false;
		std::string lower;
		std::transform(std::begin(value), std::end(value), std::back_inserter(lower), ::tolower);
		return lower != "false";
	}

	template<typename T> T as() const{
		return static_cast<T>(*this);
	}

	template<typename T> parameter& operator=(const T& t){
		value = parameter(t).value;
		return *this;
	}
};

// parameter definitions
struct definition{
	std::string name;
	parameter default_value;
	std::vector<std::string> json_path;
	std::string long_option;
	char short_option;
	std::string description;
	bool required;

	definition(const std::string& name, const parameter& default_value): name(name), default_value(default_value){}
	definition(const std::string& name, const parameter& default_value, const std::vector<std::string>& json_path):
			name(name), default_value(default_value), json_path(json_path){}
	definition(const std::string& name, const parameter& default_value, const std::string& long_option, char short_option,
		const std::string& description, bool required = false):
			name(name), default_value(default_value), long_option(long_option), short_option(short_option),
			description(description), required(required){}
	definition(const std::string& name, const parameter& default_value, const std::vector<std::string>& json_path,
		const std::string& long_option, char short_option, const std::string& description, bool required = false):
			name(name), default_value(default_value), json_path(json_path),
			long_option(long_option), short_option(short_option), description(description), required(required){}
};
using definitions = std::vector<definition>;

// main class for managing parameters
class manager{
public:
	// command line arguments without option names
	std::vector<parameter> rest;

	manager(const definitions& defs): defs(defs){
		for(const auto& def: defs)
			params[def.name] = def.default_value;
	}

	// load parameters from command line arguments and config file
	void load(int argc, char* argv[], const std::string conf_option = "", size_t min_unnamed_argc = 0){
		// parse command line arguments
		cmdline::parser parser;
		for(const auto& def: defs)
			if(!def.long_option.empty())
				parser.add(def.long_option, def.short_option, def.description, def.required,
						def.default_value.as<std::string>());
		if(!parser.parse(argc, argv))
			throw std::invalid_argument(parser.error_full() + parser.usage());
		// overwrite parameters with config file
		if(!conf_option.empty() && parser.exist(conf_option)){
			nlohmann::json json;
			std::ifstream(parser.get<std::string>(conf_option)) >> json;
			for(const auto& def: defs)
				if(!def.json_path.empty())
					load_parameter_from_json(&json, def);
		}
		// overwrite parameters with command line arguments
		for(const auto& def: defs)
			if(!def.long_option.empty() && parser.exist(def.long_option))
				params[def.name] = parser.get<std::string>(def.long_option);
		// store rest of command line arguments
		for(const auto& r: parser.rest())
			rest.push_back(r);
		if(rest.size() < min_unnamed_argc)
			throw std::invalid_argument("requires " + std::to_string(min_unnamed_argc) +
					" unnamed option" + (min_unnamed_argc > 1 ? "s" : "") + '\n' + parser.usage());
	}

	// returns parameter value
	parameter& operator[](const std::string& name){
		return params[name];
	}

	// returns parameter value as a specific type
	template<typename T> T get(const std::string& name) const{
		return params.at(name).as<T>();
	}

private:
	const definitions defs;
	std::map<std::string, parameter> params;

	// find def.json_path from json and overwrite params[def.name] if exists
	void load_parameter_from_json(const nlohmann::json* j, const definition& def){
		for(auto i = std::begin(def.json_path); i != std::end(def.json_path); j = &(j->at(*i++)))
			if(j->count(*i) == 0)
				return;
		try{
			params[def.name] = j->get<std::string>();
		}
		catch(const std::exception&){
			// if failed to get as string, convert non-string value to string
			std::stringstream ss;
			ss << *j;
			params[def.name] = ss.str();
		}
	}
};

};
#endif
