#include "MyApp.h"
#include "Logging.h"


int main() {
	try {
		MyApp app;
		app.StartApp();
	}
	catch (std::exception e) {
		GlenderLog(LogTypeLog, std::format("Failed to start app with message: {}", e.what()));
	}
	return 0;
}
