#include "Poco/Path.h"
#include "Poco/Timestamp.h"
#include "Poco/Notification.h"
#include "Poco/NotificationQueue.h"
#include "Poco/ThreadPool.h"
#include "Poco/RWLock.h"
#include "Poco/Runnable.h"

#include "Poco/Logger.h"
#include "Poco/AsyncChannel.h"
#include "Poco/ConsoleChannel.h"
#include "Poco/AutoPtr.h"

#include <iostream>
#include <fstream>
#include <set>

using namespace std;
using Poco::Path;
using Poco::Int64;
using Poco::Notification;
using Poco::NotificationQueue;
using Poco::ThreadPool;
using Poco::RWLock;
using Poco::Runnable;

using Poco::Logger;
using Poco::AsyncChannel;
using Poco::ConsoleChannel;
using Poco::AutoPtr;

static Logger* log;

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
    LineNotification(const string &line) {
			_line = new const string(line);
		}

    const string* getLine() const {
      return _line;
    }

  protected:
    ~LineNotification() {
      delete _line;
    }

  private:
    const string *_line;
};

class MergeWorker : public Runnable {
  public:
    MergeWorker(
			NotificationQueue *in_queue,
			NotificationQueue *out_queue,
			set<size_t> *sentences,
			RWLock &rw_lock
		) : _in_queue(in_queue), _out_queue(out_queue), _sentences(sentences), _rw_lock(rw_lock) {}

    void run() {
      // while((Notification *notification = _queue.waitDequeueNotification()) != NULL) {
      cout << "进入线程：" << Poco::Thread::current()->name() << endl;

      for (Notification *notification = _in_queue->waitDequeueNotification();
          NULL != notification; notification = _in_queue->waitDequeueNotification()) {
        LineNotification *pLN = (LineNotification*)notification;
        if (NULL != pLN) {
          const auto pChar = pLN->getLine()->c_str();
          const auto hash = BKDRHash(pChar);

          // 因为 _sentences 集合只添加不减少，所以可以取消这个只读锁
					// Poco::ScopedReadRWLock read_lock(_rw_lock);

					// 如果要查询的 hash 值不存在，则添加
					auto search = _sentences->find(hash);
					auto end = _sentences->end();
					if (search == end) {
						Poco::ScopedWriteRWLock write_lock(_rw_lock);

						// 使用写入锁重新判断要查询的 hash 值不存在
						if (_sentences->insert(hash).second) {
							_out_queue->enqueueNotification(notification);

              // 如果 notification 被重新放入 _out_queue，
              // 则用 continue 跳转到下一轮 for 循环，
              // 不需要先 notification->release()
              continue;
            }
          }
				}

        // 只有 notification 没有被重新放入 _out_queue 时，
        // 就需要在这里直接释放
        notification->release();
      }
    }

  private:
    NotificationQueue *_in_queue;
		NotificationQueue *_out_queue;
    set<size_t> *_sentences;
		RWLock &_rw_lock;
};

class WriterWorker : public Runnable {
  public:
    WriterWorker(NotificationQueue *queue, ofstream &out) : _queue(queue), _out(out) {}

    void run() {
      cout << "进入线程：" << Poco::Thread::current()->name() << endl;

      for (Notification *notification = _queue->waitDequeueNotification();
        NULL != notification; notification = _queue->waitDequeueNotification()) {
        LineNotification *pLN = (LineNotification*)notification;

        if (NULL != pLN) _out << *(pLN->getLine()) << '\n';

        notification->release();
      }
    }

  private:
    NotificationQueue *_queue;
    ofstream &_out;
};

const string paths[] = { "百度买房语料", "网易新闻语料"/*, "网易新闻语料20171122", "一般词全集语料", "一般词全集语料1", "一般词全集语料2", "一般词全集语料3", "一般词全集语料4", "一般词全集语料5" */};

void merge(NotificationQueue *queue, set<size_t> *sentences, const string &in_path) {
	cout << "输入文件：" << in_path << endl;

	ifstream in;
	in.open(in_path, ios_base::in);

	string buffer;
	while (getline(in, buffer)) {
    queue->enqueueNotification(new LineNotification(buffer));
	}

	in.close();

  while (!queue->empty()) {
    cout << "Wait for merge: " << in_path << endl;
    Poco::Thread::sleep(100);
  }
}

int main(int argc, char** argv) {
  AutoPtr<ConsoleChannel> pCons(new ConsoleChannel);
  AutoPtr<AsyncChannel> pAsync(new AsyncChannel(pCons));
  Logger::root().setChannel(pAsync);

  log = &(Logger::get("SentenceMerge"));

	const Int64 begin = currentTime();
	auto *sentences = new set<size_t>();

	RWLock rw_lock;

	const string out_url = "所有去重.unique.txt";

	Path path(false);
	path.pushDirectory(".");
	path.pushDirectory("assets");
	path.setFileName(out_url);

	const string out_path(path.makeAbsolute().toString());
	ofstream out;
	out.open(out_path, ios_base::out);

  NotificationQueue *in_queue = new NotificationQueue;
	NotificationQueue *out_queue = new NotificationQueue;

  MergeWorker worker1(in_queue, out_queue, sentences, rw_lock);
  MergeWorker worker2(in_queue, out_queue, sentences, rw_lock);
	/*MergeWorker worker3(in_queue, out_queue, sentences);
	MergeWorker worker4(in_queue, out_queue, sentences);*/

	WriterWorker writer(out_queue, out);

  ThreadPool::defaultPool().start(worker1);
  ThreadPool::defaultPool().start(worker2);
	/*ThreadPool::defaultPool().start(worker3);
	ThreadPool::defaultPool().start(worker4);*/

	ThreadPool::defaultPool().start(writer);

	for (int i = 0, m = sizeof(paths) / sizeof(string); i < m; i++) {
		const string p = paths[i];
		path.setFileName(p + ".grouped.txt");
		const string in_path(path.makeAbsolute().toString());

		merge(in_queue, sentences, in_path);
	}

	out.close();

	delete sentences;

  while (!in_queue->empty()) {
    cout << "等待合并队列结束。" << endl;
    Poco::Thread::sleep(500);
  }
	in_queue->wakeUpAll();

	while (!out_queue->empty()) {
		cout << "等待写入队列结束。" << endl;
		Poco::Thread::sleep(500);
	}
	out_queue->wakeUpAll();

	ThreadPool::defaultPool().joinAll();

	delete in_queue;
	delete out_queue;

	const Int64 end = currentTime();

	cout << "合并所有文件到：" << out_path << "，耗时：" << end - begin << " 微秒。" << endl;

	return EXIT_SUCCESS;
}
