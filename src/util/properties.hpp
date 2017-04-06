/*
 * properties.hpp
 *
 *  Created on: 5 de abr de 2017
 *      Author: carlosfaruolo
 */

#ifndef UTIL_PROPERTIES_HPP_
#define UTIL_PROPERTIES_HPP_
#include <ciso646>

#include <string>
#include <map>

namespace util
{
	class Properties
	{
		/// holds the data
		std::map<std::string, std::string> data;

		public:

		/** Returns a reference to the property associated with the given key in this properties object. If the key is not found, it is created with an empty property, and then returned. */
		std::string& operator[](const std::string& key);

		/** Searches for the property with the specified key in this properties object and returns it. If the key is not found, an empty string is returned. */
		std::string get(const std::string& key);

		/** Same as calling operator[key] = value, but returns the previous value of the specified key in this property list, or an empty string if it did not have one. */
		std::string put(const std::string& key, const std::string& value);

		/** Tests if the specified string is a key in this properties object. */
		bool containsKey(const std::string& key);

		/** Clears this properties object so that it contains no keys. */
		void clear();

		/** Reads a property list (key and element pairs) from the given file in a simple line-oriented format */
		void load(const std::string& filename);

		/** Writes this property list (key and element pairs) in this Properties object to the given filename in a format suitable for using the load() method. */
		void store(const std::string& filename);
	};
}

#endif /* UTIL_PROPERTIES_HPP_ */
