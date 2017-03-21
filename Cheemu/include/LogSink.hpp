#pragma once

#include <spdlog/sinks/sink.h>

#include "ExampleAppLog.hpp"

class LogSink : public spdlog::sinks::sink
{
	void log(const spdlog::details::log_msg& msg) override
	{
		appLog.AddLog( "%s\n", msg.raw.str().c_str() );
	}

	void flush() 
	{
	}

	ExampleAppLog& appLog;

public:
	LogSink( ExampleAppLog& _appLog ): appLog(_appLog) {}
};

