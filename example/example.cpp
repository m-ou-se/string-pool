#include <iostream>
#include <optional>
#include <string_view>

#include "string_tracker.hpp"

int main() {
	string_tracker p;

	std::optional<std::string_view> f1 = p.add_file("example-input-1");
	std::optional<std::string_view> f2 = p.add_file("example-input-2");

	if (!f1 || !f2) {
		std::cerr << "Unable to open files." << std::endl;
		return 1;
	}

	std::string_view s1 = f1->substr(6, 5);
	std::string_view s2 = f1->substr(23, 4);
	std::string_view s3 = f2->substr(28, 3);
	std::cout << p.location(s1) << ": " << s1 << std::endl;
	std::cout << p.location(s2) << ": " << s2 << std::endl;
	std::cout << p.location(s3) << ": " << s3 << std::endl;

	auto b = p.builder();
	b.append(f1->substr(13, 9));
	b.append("n example: ");
	b.append(f2->substr(24, 11));
	std::string_view s4 = b.build();
	std::string_view s5 = s4.substr(11,7);
	std::string_view s6 = s4.substr(20);
	std::cout << p.location(s4) << ": " << s4 << std::endl;
	std::cout << p.location(s5) << ": " << s5 << std::endl;
	std::cout << p.location(s6) << ": " << s6 << std::endl;
}
