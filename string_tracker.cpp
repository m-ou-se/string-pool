#include <algorithm>
#include <fstream>
#include <iostream>
#include <vector>

#include "string_tracker.hpp"

std::ostream &operator<<(std::ostream &out, source_location const &l) {
	out << (l.file_name.empty() ? "<unknown>" : l.file_name);
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

string_view string_tracker::add(std::vector<char> buffer, source_location origin) {
	return pool.put(std::move(buffer), std::move(origin));
}

string_view string_tracker::add(std::vector<char> buffer, string_view origin) {
	return pool.put(std::move(buffer), source_map{{{0, origin}}});
}

string_view string_tracker::add(std::vector<char> buffer, source_origin origin) {
	return pool.put(std::move(buffer), std::move(origin));
}

string_view string_tracker::add_copy(string_view s, source_location origin) {
	return add(std::vector<char>(s.begin(), s.end()), std::move(origin));
}

string_view string_tracker::add_copy(string_view s, string_view origin) {
	return add(std::vector<char>(s.begin(), s.end()), source_map{{{0, origin}}});
}

source_origin string_tracker::origin(string_view s) const {
	auto x = pool.get(s);
	if (!x) return {};
	string_view before_s(x->first.data(), s.data() - x->first.data());
	return advance(x->second, before_s);
}

source_location string_tracker::location(string_view s) const {
	auto o = origin(s);
	if (o.sources.sources.empty()) {
		return o.location;
	} else {
		return location(o.sources.sources.front().second);
	}
}

optional<string_view> string_tracker::add_file(string_view file_name) {
	std::ifstream file{std::string{file_name.begin(), file_name.end()}};
	file.seekg(0, std::ios_base::end);
	std::ifstream::pos_type file_size = file.tellg();
	std::vector<char> contents;
	if (file_size != std::ifstream::pos_type(-1)) contents.reserve(file_size);
	file.seekg(0, std::ios_base::beg);
	contents.assign(std::istreambuf_iterator<char>(file), {});
	if (file.fail()) return {};
	if (!pool.get(file_name)) {
		std::vector<char> buffer(file_name.begin(), file_name.end());
		file_name = pool.put(std::move(buffer), {});
	}
	return pool.put(std::move(contents), source_location{file_name, 1, 1});
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
	return tracker.add(std::move(buffer), std::move(origin));
}
