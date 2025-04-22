#include "Logging.h"

#include <memory>

using namespace glender;

static std::unique_ptr<Logger> logger_;

Logger* glender::GetLogger() {
	if (!logger_) {
#ifndef NDEBUG
		logger_ = std::make_unique<Logger>("log/glender.log", true, true, true);
#else
		logger_ = std::make_unique<Logger>("log/glender.log", false, true, false);
#endif
	}
	return logger_.get();
}
