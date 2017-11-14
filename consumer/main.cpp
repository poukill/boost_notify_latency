#include <iostream>

#include <boost/lexical_cast.hpp>

#include <Windows.h>

#include "shared_struct.h"

class Consumer
{
public:
	Consumer()
	{
		OpenSharedMemory();

		StartCounter();
	}
	void Run()
	{
		bi::scoped_lock<mutex_type> lock(m_main_struct->mutex);
		while (!m_main_struct->exited)
		{
			m_main_struct->cond_data_ready.wait(lock);
			if(m_main_struct->data_ready)
			{
				m_main_struct->data_ready = false;
				double counter = GetPerformanceCounter();
				double diff = counter - m_main_struct->time;
				std::cout << boost::lexical_cast<std::string>(counter) << " and the diff is : " << diff << std::endl;
			}
		}
		std::cout << "Exiting..." << std::endl;
	}

private:
	void OpenSharedMemory()
	{
		std::string shm_name = "test_condition_variable";
		m_sho = new bi::managed_shared_memory(bi::open_only, shm_name.c_str());
		m_main_struct = m_sho->find<SharedStruct>("main").first;
	}

	void StartCounter()
	{
		bi::scoped_lock<mutex_type> lock(m_main_struct->mutex);
		m_pc_freq = m_main_struct->pc_freq;
		m_counter_start = m_main_struct->counter_start;
	}

	double GetPerformanceCounter()
	{
		LARGE_INTEGER li;
		QueryPerformanceCounter(&li);
		double result = double(li.QuadPart-m_counter_start)/m_pc_freq;
		return result;
	}
private:
	bi::managed_shared_memory*	m_sho;
	SharedStruct*				m_main_struct;
	
	double						m_pc_freq;
	long long					m_counter_start;
};


int main()
{
	Consumer consumer;
	consumer.Run();
	system("pause");
	return 0;
}