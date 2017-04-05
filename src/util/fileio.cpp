/*
 * fileio.cpp
 *
 *  Created on: 5 de abr de 2017
 *      Author: carlosfaruolo
 */

#include "fileio.hpp"

#include "futil/string/actions.hpp"
#include "futil/string/split.hpp"

#include <iostream>
#include <fstream>
#include <map>
#include <stdexcept>

using fileio::Properties;
using std::map;
using std::string;
using std::vector;

std::string& Properties::operator[](const std::string& key)
{
	return data[key];
}

std::string Properties::get(const std::string& key)
{
	map<string, string>::const_iterator lb = data.lower_bound(key);
	if((lb != data.end() and not data.key_comp()(key, lb->first)))
		return lb->second;
	else
		return string();
}

std::string Properties::put(const std::string& key, const std::string& value)
{
	string oldValue = data[key];
	data[key] = value;
	return oldValue;
}

bool Properties::containsKey(const std::string& key)
{
	map<string, string>::const_iterator lb = data.lower_bound(key);
	return (lb != data.end() and not data.key_comp()(key, lb->first));
}

void Properties::clear()
{
	data.clear();
}

void Properties::load(const std::string& filename)
{
	std::ifstream stream(filename.c_str());
	if(not stream.is_open())
		throw std::runtime_error("File could not be opened: " + filename);

	string str;
	while(stream.good())
	{
		getline(stream, str);
		str = trim(str);

		if(str.empty() or starts_with(str, "#") or starts_with(str, "!") or not contains(str, "="))
			continue;

		vector<string> tokens = split(str, '=');
		if(tokens.size() != 2)
			continue;

		string key = trim(tokens[0]);
		string value = trim(tokens[1]);

		if(key.empty())
			continue;

		data[key] = value;
	}
}

void Properties::store(const std::string& filename)
{
	std::ofstream stream(filename.c_str());
	if(not stream.is_open())
		throw std::runtime_error("File could not be opened: " + filename);

	for(map<string, string>::iterator it = data.begin(); it != data.end(); ++it)
		stream << it->first << "=" << it->second;

	stream.close();
}
