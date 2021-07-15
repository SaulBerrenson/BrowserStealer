#pragma once
#include "Forwards.h"

template <class T>
class ICollector
{
public:
	ICollector() = default;
	virtual ~ICollector() = default;
	virtual List<T> collect_data() = 0;	
};
