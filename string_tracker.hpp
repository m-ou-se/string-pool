#pragma once

#include <iostream>
#include <utility>
#include <vector>

#include "string_pool.hpp"
#include "string_view.hpp"

struct source_location {
	string_view file_name;
	unsigned int line = 0;
	unsigned int column = 0;
};

inline bool operator==(source_location const &a, source_location const &b) {
	return a.file_name == b.file_name && a.line == b.line && a.column == b.column;
}

inline bool operator!=(source_location const &a, source_location const &b) {
	return !(a == b);
}

std::ostream &operator<<(std::ostream &, source_location const &);

namespace string_tracker_detail {
	struct source_map {
		std::vector<std::pair<size_t, string_view>> sources;
	};
	struct source_origin {
		source_location location;
		source_map sources;
		source_origin(source_location location = {}) : location(location) {}
		source_origin(source_map sources) : sources(std::move(sources)) {}
	};
}

class string_tracker {
public:
	string_view add(std::string, source_location = {});
	string_view add(std::string, string_view derived_from);

	optional<string_view> add_file(string_view file_name);

	source_location location(string_view s) const;

	struct string_builder {
		void append(string_view);
		void append(string_view, string_view derived_from);

		string_view build();

		void reserve(size_t s) { buffer.reserve(s); }

	private:
		string_tracker &tracker;
		std::string buffer;
		string_tracker_detail::source_map origin;

		explicit string_builder(string_tracker &tracker) : tracker(tracker) {}

		friend class string_tracker;
	};

	string_builder builder() { return string_builder{*this}; }

private:
	string_pool<string_tracker_detail::source_origin, std::string> pool;

	string_tracker_detail::source_origin origin(string_view s) const;
};
