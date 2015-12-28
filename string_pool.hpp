#pragma once

#include <map>
#include <string>

#include "optional.hpp"
#include "string_view.hpp"

namespace string_pool_detail {

	template<typename T>
	inline void prevent_sso(std::basic_string<T> &s) {
		if (s.capacity() < sizeof(s)) s.reserve(sizeof(s));
	}

	inline void prevent_sso(...) {}

}

/* Owns strings, so you can use string_views everywhere.
 *
 * Every string is together with some metadata, that can be looked up later using only the
 * string_view. That way, you don't have to copy/move the metadata around, but still access it when
 * you need it. A good example of metadata you could store is the file name that the string was read
 * from, so you can report it in errors and warnings.
 *
 * By default, this class uses std::string, but there's a small issue with std::string:
 * std::string isn't guaranteed to keep its .data() constant over moves, which this class relies on.
 * To work around that, s.resize(sizeof(s)) is applied to all strings, to prevent the only
 * acceptable reason for std::string to not keep its .data() constant: small string optimization.
 * If you want to be absolutely sure, use std::vector<char> instead.
 *
 * M: The type of the metadata.
 * S: The string type to use.
 * V: The string_view type to use.
 */
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

	/* Move a string into the pool, together with metadata.
	 *
	 * Takes the ownership of the string, and returns you a string_view to it.
	 */
	view put(string s, metadata m) {
		string_pool_detail::prevent_sso(s);
		view v(s.data(), s.size());
		pool_.emplace(v.data(), entry{std::move(s), std::move(m)});
		return v;
	}

	/* Lookup a string in the pool.
	 *
	 * Returns a pointer to the std::pair of the original string and the metadata that the
	 * view refers to. (The view may be a substring.)
	 * Returns nullptr if the view doesn't refer to any string inside this pool.
	 */
	entry const *get(view s) const {
		auto i = lookup(s);
		if (i == pool_.end()) return nullptr;
		return &i->second;
	}

	/* Take a string out of the pool.
	 *
	 * Behaves the same as get, but takes the string (and metadata) out of the pool
	 * and returns it by value, giving you the ownership back.
	 */
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
