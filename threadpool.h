/*
Pixcen - A windows platform low level pixel editor for C64
Copyright (C) 2013  John Hammarberg (crt@nospam.censordesign.com)

This file is part of Pixcen.

Pixcen is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Pixcen is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Pixcen.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <atomic>
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <functional>
#include <memory>

struct threadpool
{
private:
	struct ev
	{
		friend threadpool;

		void reset()
		{
			std::unique_lock<std::mutex> lck(mtx);
			flag = false;
		}

		void set()
		{
			std::unique_lock<std::mutex> lck(mtx);
			flag = true;
			cv.notify_one();
		}

	public:

		void wait()
		{
			std::unique_lock<std::mutex> lck(mtx);
			while (!flag)
			{
				cv.wait(lck);
			}
		}

	private:
		std::mutex mtx;
		std::condition_variable cv;
		volatile int flag;
	};

public:

	threadpool()
		: count(0)
	{
		unsigned int max = std::thread::hardware_concurrency();
		while (max)
		{
			threadlist.push_back(std::thread(&threadpool::run, this));
			--max;
		}
	}

	~threadpool()
	{
		{
			std::unique_lock<std::mutex> lck(mtx);
			for (size_t n = 0; n < threadlist.size(); n++)
			{
				++count;
				queue.push(std::pair<std::function<void()>, std::shared_ptr<ev>>(nullptr, nullptr));
				cv.notify_one();
			}
		}

		for (size_t n = 0; n < threadlist.size(); n++)
		{
			threadlist[n].join();
		}
	}

	typedef std::shared_ptr<ev> waitable;

	template<typename Function>
	void fire_and_forget(Function && fn)
	{
		{
			std::unique_lock<std::mutex> lck(mtx);
			++count;
			queue.push(std::pair<std::function<void()>, std::shared_ptr<ev>>(fn, nullptr));
		}
		cv.notify_one();
	}

	template<typename Function>
	waitable schedule(Function && fn)
	{
		std::shared_ptr<ev> w;
		{
			std::unique_lock<std::mutex> lck(mtx);
			++count;

			ev *e;
			if (evcache.size())
			{
				e = evcache.back();
				evcache.pop_back();
			}
			else
			{
				e = new ev;
			}

			e->reset();

			w = std::shared_ptr<ev>(e, [=](ev *p)
			{
				std::unique_lock<std::mutex> lck(mtx);
				evcache.push_back(p);
			});

			queue.push(std::pair<std::function<void()>, std::shared_ptr<ev>>(fn, w));
		}
		cv.notify_one();

		return w;
	}

private:

	void run()
	{
		for (;;)
		{
			std::pair<std::function<void()>, std::shared_ptr<ev>> p;

			{
				std::unique_lock<std::mutex> lck(mtx);
				while (!count)
				{
					cv.wait(lck);
				}
				--count;

				p = queue.front();
				queue.pop();
			}

			if (!p.first)
				break;

			p.first();

			if (p.second)
			{
				p.second->set();
			}
		}
	}

	std::queue<std::pair<std::function<void()>, std::shared_ptr<ev>>> queue;
	std::mutex mtx;
	std::condition_variable cv;
	int count;

	std::vector<std::thread> threadlist;
	std::vector<ev *> evcache;
};
