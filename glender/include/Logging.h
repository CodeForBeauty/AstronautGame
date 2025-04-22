#pragma once

#include "Logger.hpp"


namespace glender {
	Logger* GetLogger();
#define GlenderLog(type, message) GetLogger()->AddLog(type, message, __FILE__, __func__, __LINE__)
}