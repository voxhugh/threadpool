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
#include <future>
using namespace std;

/*  线程池：1.管理者线程	2.任务队列	3.工作的线程	4.同步机制  */

class ThreadPool
{
public:
	ThreadPool(int min = 4, int max = thread::hardware_concurrency());
	~ThreadPool();
	template<typename F, typename... Args>
	// 返回类型后置，auto追踪通过类型萃取器推导得到的结果，并且明确告诉编译是一个类型
	auto addTask(F&& f, Args&&... args) -> future<typename result_of<F(Args...)>::type>
	{
		// F&& f 未定引用类型，会转为对应类型的引用
		using returnType = typename result_of<F(Args...)>::type;
		// 任务包装器打包绑定函参后的任务函数，并套一层智能指针，绑定期间使用完美转发避免传递过程类型改变
		auto task = make_shared<packaged_task<returnType()>>(bind(forward<F>(f), forward<Args>(args)...));
		future<returnType> res = task->get_future();
		{
			unique_lock<mutex> lk(m_taskMutex);
			m_Tasks.emplace([task]() {(*task)(); });
		}
		m_condition.notify_one();
		return res;
	}

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
