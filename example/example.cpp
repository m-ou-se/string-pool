#include <iostream>

#include "string_tracker.hpp"

int main() {
	string_tracker p;

	auto f1 = p.add_file("../string_tracker.hpp");
	auto f2 = p.add_file("../string_pool.hpp");

	if (!f1 || !f2) {
		std::cerr << "Unable to open files." << std::endl;
		return 1;
	}

	auto s1 = f1->substr(19, 10);
	auto s2 = f1->substr(54, 7);
	auto s3 = f2->substr(98, 20);
	std::cout << p.location(s1) << ": " << s1 << std::endl;
	std::cout << p.location(s2) << ": " << s2 << std::endl;
	std::cout << p.location(s3) << ": " << s3 << std::endl;

	auto b = p.builder();
	b.append(f1->substr(8, 4));
	b.append("foobar");
	b.append(f2->substr(15, 7));
	auto s4 = b.build();
	auto s5 = s4.substr(7,3);
	auto s6 = s4.substr(10);
	std::cout << p.location(s4) << ": " << s4 << std::endl;
	std::cout << p.location(s5) << ": " << s5 << std::endl;
	std::cout << p.location(s6) << ": " << s6 << std::endl;
}
