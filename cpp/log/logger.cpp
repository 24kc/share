#include "logger.h"
#include <ctime>

namespace akm {

namespace log {

logger::logger(std::ostream& out) : out(out) { }

logger::~logger() { flush(); }

size_t
logger::strftime(char *str, size_t count)
{
	auto t = std::time(NULL);
	return std::strftime(str, count, "%Y-%m-%d %H:%M:%S", std::localtime(&t));
}

void
logger::flush()
{
	pool.join();
	out.flush();
}

} // namespace log

} // namespace akm
