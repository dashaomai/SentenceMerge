#include "Poco/Path.h"
#include "Poco/Timestamp.h"
#include "Poco/Notification.h"
#include "Poco/NotificationQueue.h"
#include "Poco/ThreadPool.h"
#include "Poco/Runnable.h"

#include <iostream>
#include <fstream>
#include <set>

using namespace std;
using Poco::Path;
using Poco::Int64;
using Poco::Notification;
using Poco::NotificationQueue;
using Poco::ThreadPool;
using Poco::Runnable;

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

class LineNotification : public Notification {
  public:
    LineNotification(const string *line) : _line(line) {}

    const string* getLine() const {
      return _line;
    }

  private:
    const string *_line;
};

class MergeWorker : public Runnable {
  public:
    MergeWorker(NotificationQueue *queue, set<size_t> *sentences) : _queue(queue), _sentences(sentences) {}

    void run() {
      // while((Notification *notification = _queue.waitDequeueNotification()) != NULL) {
      cout << "进入线程：" << Poco::Thread::current()->name() << endl;

      for (Notification *notification = _queue->waitDequeueNotification();
          NULL != notification; notification = _queue->waitDequeueNotification()) {
        LineNotification *pLN = (LineNotification*)notification;
        if (NULL != pLN) {
          const auto pChar = pLN->getLine()->c_str();
          const auto hash = BKDRHash(pChar);
          if (_sentences->insert(hash).second) {
            cout << pChar << endl;
          }
        }

        notification->release();
      }
    }

  private:
    NotificationQueue *_queue;
    set<size_t> *_sentences;
};

const string paths[] = { "百度买房语料", "网易新闻语料", "网易新闻语料20171122", "一般词全集语料", "一般词全集语料1", "一般词全集语料2", "一般词全集语料3", "一般词全集语料4", "一般词全集语料5" };

void merge(NotificationQueue *queue, set<size_t> *sentences, const string &in_path, ofstream &out) {
	cout << "输入文件：" << in_path << endl;

	ifstream in;
	in.open(in_path, ios_base::in);

	string *buffer = new string;
	while (getline(in, *buffer)) {
		/*const auto str = buffer.c_str();
		const auto hash = BKDRHash(str);
		if (sentences->insert(hash).second) {
			out << str << endl;
		}*/

    queue->enqueueNotification(new LineNotification(buffer));
	}

	in.close();

  while (!queue->empty()) {
    cout << "Wait for merge: " << in_path << endl;
    Poco::Thread::sleep(100);
  }
}

int main(int argc, char** argv) {
	const Int64 begin = currentTime();
	auto *sentences = new set<size_t>();

  NotificationQueue *queue = new NotificationQueue;

  MergeWorker worker1(queue, sentences);
  MergeWorker worker2(queue, sentences);

  ThreadPool::defaultPool().start(worker1);
  ThreadPool::defaultPool().start(worker2);

	const string out_url = "所有去重.unique.txt";

	Path path(false);
	path.pushDirectory(".");
	path.pushDirectory("assets");
	path.setFileName(out_url);

	const string out_path(path.makeAbsolute().toString());
	ofstream out;
	out.open(out_path, ios_base::out);

	for (int i = 0, m = sizeof(paths) / sizeof(string); i < m; i++) {
		const string p = paths[i];
		path.setFileName(p + ".grouped.txt");
		const string in_path(path.makeAbsolute().toString());

		merge(queue, sentences, in_path, out);
	}

	out.close();

	delete sentences;

  while (!queue->empty()) {
    cout << "Wait for all merged." << endl;
    Poco::Thread::sleep(100);
  }

  delete queue;

	const Int64 end = currentTime();

	cout << "合并所有文件到：" << out_path << "，耗时：" << end - begin << " 微秒。" << endl;

	return EXIT_SUCCESS;
}
