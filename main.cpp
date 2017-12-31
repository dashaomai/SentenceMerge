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

const string paths[] = {"百度买房语料", "网易新闻语料", "网易新闻语料20171122"};

void merge(set<size_t> *sentences, const string in_path, const string out_path) {
  cout << "输入文件：" << in_path << "，输出文件：" << out_path << endl;

  ifstream in;
  in.open(in_path, ios_base::in);

  ofstream out;
  out.open(out_path, ios_base::out);

  string buffer;
  while (getline(in, buffer)) {
    const auto str = buffer.c_str();
    const auto hash = BKDRHash(str);
    if (sentences->insert(hash).second) {
      out << str << endl;
    }
  }

  out.close();
  in.close();
}

int main(int argc, char** argv) {
	auto *sentences = new set<size_t>();
  const string out_path = "所有去重.unique.txt";

	Path path(false);
	path.pushDirectory(".");
	path.pushDirectory("assets");

  for (int i=0, m=sizeof(paths)/sizeof(string); i<m; i++) {
    const string p = paths[i];
    path.setFileName(p + ".grouped.txt");
    const string in_path(path.makeAbsolute().toString());

    merge(sentences, in_path, out_path);
  }

  return EXIT_SUCCESS;
}
