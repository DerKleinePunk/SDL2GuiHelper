#include "SDL2Helper.h"

namespace utils
{
    utils::LOGCALLBACK logCallback;

	void LogOutputFunction(void *userdata, int category, SDL_LogPriority priority, const char *message) {
		logCallback(userdata, category, priority, message);
	}
}