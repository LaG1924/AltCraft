#pragma once

struct Thread {
	Thread() = default;
	virtual ~Thread() = default;
	virtual void Execute() = 0;
};