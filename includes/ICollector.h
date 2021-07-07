#pragma once
#include "Forwards.h"

class ICollector
{
public:
	ICollector() = default;
	virtual ~ICollector() = default;
	virtual List<AccountData> collect_data() = 0;	
};
