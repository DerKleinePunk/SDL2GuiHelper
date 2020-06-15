//based on https://github.com/vichargrave/wqueue/blob/master/wqueue.h
#pragma once

#include <mutex>
#include <deque>

template <typename T> class waitingqueue
{
	std::deque<T> _queue;
	std::mutex   _mutex;
	std::condition_variable _condivari;
	bool ready;

public:
	waitingqueue(): 
		ready(false) {
	} 

	~waitingqueue() {
		_queue.clear();
	}
    
    void clear() {
        _mutex.lock();
        _queue.clear();
        _mutex.unlock();
    }
	void add(T item) {
		_mutex.lock();
		_queue.push_back(item);
		_mutex.unlock();

		std::lock_guard<std::mutex> lk(_mutex);
		ready = true;
		_condivari.notify_one();
	}

	T remove() {
		_mutex.lock();
		if (_queue.size() == 0) {
			_mutex.unlock();
			std::unique_lock<std::mutex> lk(_mutex);
			_condivari.wait(lk, [this] { return ready; });
			lk.unlock();
			_mutex.lock();
		}
		ready = false;
		T item = _queue.front();
		_queue.pop_front();
		_mutex.unlock();
		return item;
	}

	int size() {
		_mutex.lock();
		int size = _queue.size();
		_mutex.unlock();
		return size;
	}
};
