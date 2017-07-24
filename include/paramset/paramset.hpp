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

#pragma once

#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>

#include "cmdline.h"
#include "json.hpp"

namespace paramset{

// common exception
struct exception: public std::exception{
	const std::string message;

	explicit exception(const std::string& message): message(message){}

	const char* what() const noexcept override{
		return message.c_str();
	}
};

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
		try{
			return std::stoi(value);
		}
		catch(const std::invalid_argument& e){
			throw exception("invalid value: " + value);
		}
	}

	operator double() const{
		try{
			return std::stod(value);
		}
		catch(const std::invalid_argument& e){
			throw exception("invalid value: " + value);
		}
	}

	operator bool() const{
		if(value == "" || value == "0")
			return false;
		std::string lower;
		std::transform(std::begin(value), std::end(value), std::back_inserter(lower), ::tolower);
		return !(lower == "false");
	}

	template<typename T> T as() const{
		return static_cast<T>(*this);
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
	const definitions defs;
	std::map<std::string, parameter> params;
public:
	// command line arguments without option names
	std::vector<parameter> rest;

	manager(const definitions& defs): defs(defs){
		for(const auto& def: defs)
			params[def.name] = def.default_value;
	}

	// load parameters from command line arguments and config file
	void load(int argc, char* argv[], const std::string& conf = "", size_t min_unnamed_argc = 0){
		// parse command line arguments
		cmdline::parser parser;
		for(const auto& def: defs)
			if(def.long_option != "")
				parser.add(def.long_option, def.short_option, def.description, def.required, def.default_value.as<std::string>());
		if(!parser.parse(argc, argv))
			throw exception(parser.error_full() + parser.usage());
		// overwrite parameters with config file
		if(conf != "" && parser.exist(conf)){
			std::ifstream ifs(parser.get<std::string>(conf));
			nlohmann::json json;
			try{
				ifs >> json;
			}
			catch(const std::exception& e){
				throw exception(e.what());
			}
			for(const auto& def: defs)
				json_set(json, def); 
		}
		// overwrite parameters with command line arguments
		for(const auto& def: defs){
			if(def.long_option != "" && parser.exist(def.long_option))
				params[def.name] = {parser.get<std::string>(def.long_option)};
		}
		// store rest of command line arguments
		for(const auto& r: parser.rest()){
			rest.push_back(r);
		}
		if(rest.size() < min_unnamed_argc){
			std::stringstream ss;
			ss << "requires " << min_unnamed_argc << " unnamed options" << std::endl;
			throw exception(ss.str() + parser.usage());
		}
	}

	// returns parameter value
	const parameter operator[](const std::string& name){
		try{
			return params.at(name);
		}
		catch(const std::out_of_range& e){
			throw exception("invalid parameter name: " + name);
		}
	}

	// returns parameter value as a specific type
	template<typename T> T get(const std::string& name){
		return (*this)[name].as<T>();
	}
private:
	void json_set(const nlohmann::json& root, const definition& def){
		if(def.json_path.size() > 0){
			// find property
			const auto* j = &root;
			for(const auto& key: def.json_path){
				if(j->count(key) == 0)
					return;
				j = &(*j)[key];
			}
			try{
				// try to read value as string
				params[def.name] = j->get<std::string>();
			}
			catch(const std::exception&){
				// set non-string value
				std::stringstream ss;
				ss << *j;
				params[def.name] = ss.str();
			}
		}
	}
};

};
