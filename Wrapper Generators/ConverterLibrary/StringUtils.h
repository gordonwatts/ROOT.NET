#pragma once

#include <vector>
#include <string>
#include <algorithm>

// Split a string by a deliminter
inline std::vector<std::string> split(const std::string &source, const std::string &delimiter)
{
	std::vector<std::string> result;

	auto idx = source.find(delimiter);
	decltype(idx) lastDelim = 0;
	while (idx != std::string::npos) {
		result.push_back(source.substr(lastDelim, idx - lastDelim));
		lastDelim = idx + delimiter.size();
		idx = source.find(delimiter, lastDelim);
	}
	
	// And anything left at the end of the string.
	if (lastDelim <= source.size())
		result.push_back(source.substr(lastDelim));

	return result;
}

// Make a string suitable for source code.
inline std::string sanitized_for_cpp(const std::string &source)
{
	std::string result(source);
	std::replace(result.begin(), result.end(), ':', '_');
	return result;
}

