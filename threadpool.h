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

/*  �̳߳أ�1.�������߳�	2.�������	3.�������߳�	4.ͬ������  */

class ThreadPool
{
public:
	ThreadPool(int min = 4, int max = thread::hardware_concurrency());
	~ThreadPool();
	void addTask(function<void()> f);					// ������������
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
