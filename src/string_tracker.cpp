#include <algorithm>
#include <fstream>
#include <iostream>
#include <vector>

#include "string_tracker.hpp"

using mstd::optional;
using mstd::string_view;

namespace string_pool {

using namespace impl_detail;

std::ostream &operator<<(std::ostream &out, source_location const &l) {
	out << (l.file_name.empty() ? "?" : l.file_name);
	if (l.line) {
		out << ':' << l.line;
		if (l.column) out << ':' << l.column;
	} else {
		if (l.column) out << ":?:" << l.column;
	}
	return out;
}

source_location advance(source_location location, string_view s) {
	unsigned int n_newlines = 0;
	char const *last_newline = nullptr;
	for (char const &c : s) {
		if (c == '\n') {
			++n_newlines;
			last_newline = &c;
		}
	}
	if (n_newlines > 0) {
		if (location.line) location.line += n_newlines;
		location.column = s.data() + s.size() - last_newline;
	} else {
		if (location.column) location.column += s.size();
	}
	return location;
}

source_map advance(source_map const &map, size_t s) {
	auto i = std::prev(std::upper_bound(
		map.sources.begin(),
		map.sources.end(),
		std::make_pair(s, string_view{}),
		[] (auto const &a, auto const &b) { return a.first < b.first; }
	));
	source_map result;
	result.sources.assign(i, map.sources.end());
	result.sources.front().first = s;
	return result;
}

source_origin advance(source_origin const &origin, string_view s) {
	if (origin.sources.sources.empty()) {
		return advance(origin.location, s);
	} else {
		return advance(origin.sources, s.size());
	}
}

string_view string_tracker::add(std::string buffer, source_location origin) {
	return pool.put(std::move(buffer), std::move(origin));
}

string_view string_tracker::add(std::string buffer, string_view origin) {
	return pool.put(std::move(buffer), source_map{{{0, origin}}});
}

std::pair<string_view, source_origin> string_tracker::origin(string_view s) const {
	auto x = pool.get(s);
	if (!x) return {};
	string_view before_s(x->first.data(), s.data() - x->first.data());
	return {x->first, advance(x->second, before_s)};
}

string_tracker::get_result string_tracker::get(char const * s) const {
	auto o = origin(s);
	if (o.second.sources.sources.empty()) {
		get_result result;
		result.location = o.second.location;
		result.original_source = o.first;
		result.original_char = s;
		return result;
	} else {
		return get(&o.second.sources.sources.front().second[0]);
	}
}

optional<string_view> string_tracker::add_file(string_view file_name) {
	std::ifstream file{file_name.to_string()};
	file.seekg(0, std::ios_base::end);
	std::ifstream::pos_type file_size = file.tellg();
	std::string contents;
	if (file_size != std::ifstream::pos_type(-1)) contents.reserve(file_size);
	file.seekg(0, std::ios_base::beg);
	contents.assign(std::istreambuf_iterator<char>(file), {});
	if (file.fail()) return {};
	if (!pool.get(file_name)) file_name = pool.put(file_name.to_string(), {});
	return pool.put(std::move(contents), source_location{file_name, 1, 1});
}

bool string_tracker::string_builder::empty() const {
	return origin.sources.empty();
}

void string_tracker::string_builder::append(string_view s) {
	origin.sources.emplace_back(buffer.size(), tracker.pool.get(s) ? s : string_view{});
	buffer.insert(buffer.end(), s.begin(), s.end());
}

void string_tracker::string_builder::append(string_view s, string_view o) {
	origin.sources.emplace_back(buffer.size(), o);
	buffer.insert(buffer.end(), s.begin(), s.end());
}

string_view string_tracker::string_builder::build() {
	return tracker.pool.put(std::move(buffer), std::move(origin));
}

}
