#include "Poco/Path.h"
#include "Poco/Timestamp.h"

#include <iostream>
#include <fstream>
#include <set>

using namespace std;
using Poco::Path;
using Poco::Int64;

inline const size_t BKDRHash(const char *str) {
	size_t hash = 0;
	while (const auto ch = (size_t)*str++) {
		hash = hash * 131 + ch;
	}

	return hash;
}

const Int64 currentTime() {
	Poco::Timestamp now;
	return now.epochMicroseconds();
}

const string paths[] = {"百度买房语料", "网易新闻语料", "网易新闻语料20171122", "一般词全集语料", "一般词全集语料1", "一般词全集语料2", "一般词全集语料3", "一般词全集语料4", "一般词全集语料5"};

void merge(set<size_t> *sentences, const string &in_path, ofstream &out) {
  cout << "输入文件：" << in_path << endl;

  ifstream in;
  in.open(in_path, ios_base::in);

  string buffer;
  while (getline(in, buffer)) {
    const auto str = buffer.c_str();
    const auto hash = BKDRHash(str);
    if (sentences->insert(hash).second) {
      out << str << endl;
    }
  }

  in.close();
}

int main(int argc, char** argv) {
  const Int64 begin = currentTime();

	auto *sentences = new set<size_t>();
  const string out_url = "所有去重.unique.txt";

	Path path(false);
	path.pushDirectory(".");
	path.pushDirectory("assets");
  path.setFileName(out_url);

  const string out_path(path.makeAbsolute().toString());
  ofstream out;
  out.open(out_path, ios_base::out);

  for (int i=0, m=sizeof(paths)/sizeof(string); i<m; i++) {
    const string p = paths[i];
    path.setFileName(p + ".grouped.txt");
    const string in_path(path.makeAbsolute().toString());

    merge(sentences, in_path, out);
  }

  out.close();

  delete sentences;

  const Int64 end = currentTime();

  cout << "合并所有文件到：" << out_path << "，耗时：" << end - begin << " 毫秒。" << endl;

  return EXIT_SUCCESS;
}
