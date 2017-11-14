#pragma once

#include <boost/interprocess/sync/interprocess_condition.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>

#pragma pack(push, 1)

namespace bi = boost::interprocess;

typedef bi::interprocess_mutex									mutex_type;
typedef bi::interprocess_condition								condition_type;

struct SharedStruct
{
	// Data to share
	double			time;
	double			random;

	// Sync objects
	mutex_type		mutex;
	condition_type	cond_data_ready;
	volatile bool	data_ready;
	volatile bool	exited;

	// timing information
	long long		counter_start;
	double			pc_freq;
};
