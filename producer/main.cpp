#include <iostream>

#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>

#include <Windows.h>

#include "shared_struct.h"

class Producer
{
public:
	Producer()
		:m_continue(true)
	{
		InitEvent();

		// Create Shared Memory
		CreateSharedMemory();

		// Initialize counter locally and in shared memory
		StartCounter();
	}
	~Producer()
	{
		bi::shared_memory_object::remove("test_condition_variable"); // Remove shared memory from the system
	}

	void InitEvent()
	{
		m_handle = CreateEvent( 
        NULL,               // default security attributes
        TRUE,               // manual-reset event
        FALSE,              // initial state is nonsignaled
        TEXT("WriteEvent")  // object name
        ); 
	}

	void Run()
	{
		// Run thread
		m_thread.reset(new boost::thread(boost::bind(&Producer::EmulateData, this)));

		for (int i = 0; i != 10; ++i)
		{
			//if (WaitForSingleObject(m_handle, INFINITE) == WAIT_OBJECT_0)
			if(true)
			{
				Sleep(1000);
				double counter;
				{
					bi::scoped_lock<mutex_type> lock(m_main_struct->mutex);
					counter = GetPerformanceCounter();
					m_main_struct->time = counter;
					m_main_struct->data_ready = true;
				}
				m_main_struct->cond_data_ready.notify_all();

				std::cout << boost::lexical_cast<std::string>(counter) <<" , i = " << i << std::endl;
			}
			/*else
			{
				std::cout << "Event ERROR !!!" << std::endl;
			}*/
		}
		// Notify the other process we are exiting !
		{
			bi::scoped_lock<mutex_type> lock(m_main_struct->mutex);
			m_main_struct->exited = true;
			m_main_struct->cond_data_ready.notify_all();
		}
		
		m_continue = false;
		m_thread->join();
	}

private:
	void CreateSharedMemory()
	{
		std::string shm_name = "test_condition_variable";
		bi::shared_memory_object::remove(shm_name.c_str());
		m_sho = new bi::managed_shared_memory(bi::create_only, shm_name.c_str(), 1000);
		m_main_struct = m_sho->construct<SharedStruct>("main")();
		m_main_struct->data_ready = false;
		m_main_struct->exited = false;
	}

	void CreateChildProcess()
	{
	}
	
	void EmulateData()
	{
		while (m_continue)
		{
			double current_time = GetPerformanceCounter();
			while (GetPerformanceCounter() < current_time + 990.0)
			{
				Sleep(10);
			}
			PulseEvent(m_handle);
		}
	}

	void StartReading()
	{
	}

	void StartCounter()
	{
		LARGE_INTEGER li;
		QueryPerformanceFrequency(&li);
		m_pc_freq = double(li.QuadPart)/1000.0;
		QueryPerformanceCounter(&li);
		m_counter_start = li.QuadPart;

		m_main_struct->pc_freq = m_pc_freq;
		m_main_struct->counter_start = m_counter_start;
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

	HANDLE						m_handle;
	boost::shared_ptr<boost::thread> m_thread;
	bool						m_continue;
};

int main()
{
	Producer producer;
	Sleep(3000);
	producer.Run();
	system("pause");
	return 0;
}