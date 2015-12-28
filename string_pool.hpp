#pragma once

#include <iterator>
#include <map>
#include <vector>

#include "optional.hpp"
#include "string_view.hpp"

template<typename T, typename C = char>
class string_pool {
public:
	using entry = std::pair<std::vector<C>, T>;
	using string_view = basic_string_view<C>;

	string_view put(std::vector<C> s, T metadata) {
		return put({std::move(s), std::move(metadata)});
	}

	string_view put(entry e) {
		string_view s(e.first.data(), e.first.size());
		pool_.emplace(s.data(), std::move(e));
		return s;
	}

	entry const *get(string_view s) const {
		auto i = lookup(s);
		if (i == pool_.end()) return nullptr;
		return &i->second;
	}

	optional<entry> take(string_view s) {
		auto i = lookup(s);
		if (i == pool_.end()) return {};
		entry e = std::move(i->second);
		pool_.erase(i);
		return std::move(e);
	}

private:
	std::map<C const *, entry> pool_;

	typename std::map<C const *, entry>::const_iterator
	lookup(string_view s) const {
		auto i = pool_.upper_bound(s.data());
		if (i == pool_.begin()) return pool_.end();
		--i;
		std::size_t offset = s.data() - i->second.first.data();
		if (offset > i->second.first.size()) return pool_.end();
		return i;
	}
};
