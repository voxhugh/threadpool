#include "threadpool.hpp"

ThreadPool::ThreadPool(int min, int max) :m_minThreads(min), m_maxThreads(max), m_Stop(false), m_exitNum(0)
{
	m_idleThread = m_curThread = min;
	cout << "�߳�����" << m_curThread << endl;
	// ʵ�����������߳�
	m_Manager = new thread(&ThreadPool::manager, this);
	// ʵ�����������߳�
	for (int i = 0; i < m_curThread; ++i)
	{
		thread t(&ThreadPool::worker, this);
		m_Workers.emplace(make_pair(t.get_id(), move(t)));
	}
}

ThreadPool::~ThreadPool()
{
	m_Stop = true;
	m_condition.notify_all();
	for (auto& x : m_Workers)
	{
		thread& t = x.second;
		if (t.joinable())
		{
			cout << "******** �߳� " << t.get_id() << " ��Ҫ�˳���..." << endl;
			t.join();
		}
	}
	if (m_Manager->joinable())
	{
		m_Manager->join();
	}
	delete m_Manager;
}

// ������������
void ThreadPool::manager()
{
	// ��ѯִ��
	while (!m_Stop.load())
	{
		this_thread::sleep_for(chrono::seconds(2));

		int cur = m_curThread.load();
		int idle = m_idleThread.load();

		if (cur > m_minThreads && idle > cur / 2)
		{
			m_exitNum.store(2);

			m_condition.notify_all();
			{
				lock_guard<mutex> lck(m_idsMutex);
				for (const auto& id : m_ids)
				{
					auto  it = m_Workers.find(id);
					if (it != m_Workers.end())
					{
						cout << "############## �߳�" << id << "��������...." << endl;
						it->second.join();
						m_Workers.erase(it);
					}
				}
				m_ids.clear();
			}

		}
		else if (idle == 0 && cur < m_maxThreads)
		{
			thread t(&ThreadPool::worker, this);
			cout << "+++++++++++++++ �����һ���߳�, id: " << t.get_id() << endl;
			m_Workers.emplace(make_pair(t.get_id(), move(t)));
			m_ids.emplace_back(t.get_id());
			++m_curThread;
			++m_idleThread;
		}
	}
}

// ������������
void ThreadPool::worker()
{
	while (!m_Stop.load())
	{
		function<void()> task = nullptr;
		{
			unique_lock<mutex> lck(m_taskMutex);
			while (!m_Stop && m_Tasks.empty())
			{
				m_condition.wait(lck);
				if (m_exitNum > 0)
				{
					cout << "--------------- �߳��������, ID: " << this_thread::get_id() << endl;
					--m_curThread;
					--m_exitNum;
					lock_guard<mutex> locker(m_idsMutex);
					m_ids.emplace_back(this_thread::get_id());
					return;
				}
			}

			if (!m_Tasks.empty())
			{
				cout << "ȡ��һ������..." << endl;
				task = move(m_Tasks.front());
				m_Tasks.pop();
			}
		}

		if (task)
		{
			--m_idleThread;
			task();
			++m_idleThread;
		}
	}
}

int calc(int x, int y)
{
	int res = x + y;
	this_thread::sleep_for(chrono::seconds(2));
	return res;
}

int main()
{
	ThreadPool pool(4);
	vector<future<int>> ans;
	for (int i = 0; i < 10; ++i)
	{
		ans.emplace_back(pool.addTask(bind(calc, i, 2 * i)));
	}
	for (auto&& x : ans)
	{
		cout << "�̺߳�������ֵ: " << x.get() << endl;
	}
	return 0;
}