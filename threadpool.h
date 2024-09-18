#pragma once
#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <unordered_map>
#include <vector>
#include <functional>
#include <atomic>
#include <chrono>
using namespace std;

/*  线程池：1.管理者线程	2.任务队列	3.工作的线程	4.同步机制  */

class ThreadPool
{
public:
	ThreadPool(int min = 4, int max = thread::hardware_concurrency());
	~ThreadPool();
	void addTask(function<void()> f);					// 生产者任务函数
private:
	void manager();										// 管理器任务函数
	void worker();										// 消费者任务函数
private:
	thread* m_Manager;									// 管理者线程
	queue<function<void()>> m_Tasks;					// 任务队列
	unordered_map<thread::id, thread> m_Workers;		// 消费者线程
	vector<thread::id> m_ids;							// 待删列表
	int m_minThreads;
	int m_maxThreads;
	atomic_bool m_Stop;
	atomic_int m_curThread;
	atomic_int m_idleThread;
	atomic_int m_exitNum;
	mutex m_taskMutex;
	mutex m_idsMutex;
	condition_variable m_condition;
};
