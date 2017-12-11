// Thread signaling class
//
// Copyright(C) 2017. Nebojsa Sumrak <nsumrak@yahoo.com>
//
//   This program is free software; you can redistribute it and / or modify
//	 it under the terms of the GNU General Public License as published by
//	 the Free Software Foundation; either version 2 of the License, or
//	 (at your option) any later version.
//
//	 This program is distributed in the hope that it will be useful,
//	 but WITHOUT ANY WARRANTY; without even the implied warranty of
//	 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
//	 GNU General Public License for more details.
//
//	 You should have received a copy of the GNU General Public License along
//	 with this program; if not, write to the Free Software Foundation, Inc.,
//	 51 Franklin Street, Fifth Floor, Boston, MA 02110 - 1301 USA.

#pragma once

#include <pthread.h>

class ThreadSignal {
protected:
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	int count;
	int max_threads;

public:
	ThreadSignal(int num_cores)
	{
		pthread_mutex_init(&mutex, 0);
		pthread_cond_init(&cond, 0);
		count = 0;
		max_threads = num_cores;
	}

	~ThreadSignal()
	{
		pthread_mutex_destroy(&mutex);
		pthread_cond_destroy(&cond);
	}

	// thread uses signal_end to report it is finished
	void signal_end()
	{
		pthread_mutex_lock(&mutex);
		count--;
		pthread_cond_signal(&cond);
		pthread_mutex_unlock(&mutex);
	}

	// used when new thread is created
	void new_thread()
	{
		pthread_mutex_lock(&mutex);
		count++;
		pthread_mutex_unlock(&mutex);
	}

	// main thread uses to wait for thread to finish
	void wait_for_signal()
	{
		pthread_mutex_lock(&mutex);
		while (count >= max_threads)
			pthread_cond_wait(&cond, &mutex);
		pthread_mutex_unlock(&mutex);
	}

	// used by main thread when finishing
	void wait_for_finish()
	{
		pthread_mutex_lock(&mutex);
		while (count)
			pthread_cond_wait(&cond, &mutex);
		pthread_mutex_unlock(&mutex);
	}
};
