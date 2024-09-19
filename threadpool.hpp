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

/*  �̳߳أ�1.�������߳�	2.�������	3.�������߳�	4.ͬ������  */

class ThreadPool
{
public:
	ThreadPool(int min = 4, int max = thread::hardware_concurrency());
	~ThreadPool();
	template<typename F, typename... Args>
	// �������ͺ��ã�auto׷��ͨ��������ȡ���Ƶ��õ��Ľ����������ȷ���߱�����һ������
	auto addTask(F&& f, Args&&... args) -> future<typename result_of<F(Args...)>::type>
	{
		// F&& f δ���������ͣ���תΪ��Ӧ���͵�����
		using returnType = typename result_of<F(Args...)>::type;
		// �����װ������󶨺��κ��������������һ������ָ�룬���ڼ�ʹ������ת�����⴫�ݹ������͸ı�
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
	void manager();										// ������������
	void worker();										// ������������
private:
	thread* m_Manager;									// �������߳�
	queue<function<void()>> m_Tasks;					// �������
	unordered_map<thread::id, thread> m_Workers;		// �������߳�
	vector<thread::id> m_ids;							// ��ɾ�б�
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
