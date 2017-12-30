#include "Poco/Path.h"

#include <iostream>
#include <fstream>
#include <set>

using namespace std;
using Poco::Path;

const size_t BKDRHash(const char *str) {
	size_t hash = 0;
	while (const auto ch = (size_t)*str++) {
		hash = hash * 131 + ch;
	}

	return hash;
}

int main(int argc, char** argv) {
	auto *sentences = new set<const size_t >();

	Path path(false);
	path.pushDirectory(".");
	path.pushDirectory("assets");
	path.setFileName("百度买房语料.sentence.txt");

  string in_path(path.makeAbsolute().toString());

	ifstream in;
	in.open(in_path, ios_base::in);

	path.setFileName("百度买房语料.grouped.txt");
	in_path = path.makeAbsolute().toString();

	ofstream out;
	out.open(in_path, ios_base::out);

	string buffer;

	while (getline(in, buffer)) {
		// 计算字符串的 hash 码，以检查是不是已经存在过的字符串
		const auto str = buffer.c_str();
		const auto hash = BKDRHash(str);
		if (sentences->insert(hash).second) {
			// out.write(str, strlen(str));
      out << str << endl;
		}
	}

	out.close();
	in.close();

	delete sentences;

  return EXIT_SUCCESS;
}
