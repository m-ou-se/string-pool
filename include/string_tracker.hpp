#pragma once

#include <iostream>
#include <optional>
#include <string_view>
#include <utility>
#include <vector>

#include "string_pool.hpp"

namespace string_pool {

// The location of a character in a text file.
struct source_location {

	// The file name, or empty if unknown.
	std::string_view file_name;

	// The line number, or 0 if unknown.
	unsigned int line = 0;

	// The column number, or 0 if unknown.
	unsigned int column = 0;

	explicit operator bool() const {
		return line || column || !file_name.empty();
	}
};

inline bool operator==(source_location const &a, source_location const &b) {
	return a.file_name == b.file_name && a.line == b.line && a.column == b.column;
}

inline bool operator!=(source_location const &a, source_location const &b) {
	return !(a == b);
}

std::ostream &operator<<(std::ostream &, source_location const &);

namespace impl_detail {
	struct source_map {
		std::vector<std::pair<size_t, std::string_view>> sources;
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
	std::string_view add(std::string, source_location = {});
	std::string_view add(std::string, std::string_view derived_from);

	std::optional<std::string_view> add_file(std::string_view file_name);

	struct get_result {
		std::string_view original_source;
		char const *original_char;
		source_location location;
	};

	get_result get(char const *) const;
	get_result get(std::string_view s) const { return get(s.data()); }

	source_location location(std::string_view s) const { return get(s).location; }
	source_location location(char const * s) const { return get(s).location; }

	struct string_builder {
		void append(std::string_view);
		void append(std::string_view, std::string_view derived_from);

		bool empty() const;

		std::string_view build();

		void reserve(size_t s) { buffer.reserve(s); }

	private:
		string_tracker &tracker;
		std::string buffer;
		impl_detail::source_map origin;

		explicit string_builder(string_tracker &tracker) : tracker(tracker) {}

		friend class string_tracker;
	};

	string_builder builder() { return string_builder{*this}; }

private:
	string_pool<impl_detail::source_origin> pool;

	std::pair<std::string_view, impl_detail::source_origin> origin(std::string_view) const;
};

}
