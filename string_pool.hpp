#pragma once

#include <map>
#include <string>

#include "optional.hpp"
#include "string_view.hpp"

namespace string_pool_detail {

	template<typename T>
	inline void prevent_sso(std::basic_string<T> &s) {
		s.reserve(sizeof(s));
	}

	inline void prevent_sso(...) {}

}

template<
	typename M,
	typename S = std::string,
	typename V = basic_string_view<typename S::value_type>
>
class string_pool {
public:
	using metadata = M;
	using entry = std::pair<S, M>;
	using string = S;
	using view = V;

	view put(string s, metadata m) {
		string_pool_detail::prevent_sso(s);
		view v(s.data(), s.size());
		pool_.emplace(v.data(), entry{std::move(s), std::move(m)});
		return v;
	}

	entry const *get(view s) const {
		auto i = lookup(s);
		if (i == pool_.end()) return nullptr;
		return &i->second;
	}

	optional<entry> take(view s) {
		auto i = lookup(s);
		if (i == pool_.end()) return {};
		entry e = std::move(i->second);
		pool_.erase(i);
		return std::move(e);
	}

private:
	using pool_type_ = std::map<typename string::const_pointer, entry>;

	pool_type_ pool_;

	typename pool_type_::const_iterator lookup(view s) const {
		auto i = pool_.upper_bound(s.data());
		if (i == pool_.begin()) return pool_.end();
		--i;
		std::size_t offset = s.data() - i->second.first.data();
		if (offset > i->second.first.size()) return pool_.end();
		return i;
	}
};
