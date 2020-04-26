#ifndef _LOGGER_H_
#define _LOGGER_H_

#include <ostream>
#include "thread_pool.h"

namespace akm {

enum class log_level {
	debug, info, warning, error, fatal
};

inline constexpr log_level log_level_debug = log_level::debug;
inline constexpr log_level log_level_info = log_level::info;
inline constexpr log_level log_level_warning = log_level::warning;
inline constexpr log_level log_level_error = log_level::error;
inline constexpr log_level log_level_fatal = log_level::fatal;

namespace log {

template<class T>
std::ostream& operator<<(std::ostream& out, std::reference_wrapper<T>& rw)
{
	return out<<rw.get();
}

class logger {
  public:
	logger(std::ostream& out);
	logger(const logger& other) = delete;
	logger& operator=(const logger& other) = delete;
	~logger();

	template<class... Msgs>
	void
	log(log_level level, Msgs&&... msgs);

	template<class... Msgs>
	void
	debug(Msgs&&... msgs)
	{
		log(log_level_debug, std::forward<Msgs>(msgs)...);
	}

	template<class... Msgs>
	void
	info(Msgs&&... msgs)
	{
		log(log_level_info, std::forward<Msgs>(msgs)...);
	}

	template<class... Msgs>
	void
	warning(Msgs&&... msgs)
	{
		log(log_level_warning, std::forward<Msgs>(msgs)...);
	}

	template<class... Msgs>
	void
	error(Msgs&&... msgs)
	{
		log(log_level_error, std::forward<Msgs>(msgs)...);
	}

	template<class... Msgs>
	void
	fatal(Msgs&&... msgs)
	{
		log(log_level_fatal, std::forward<Msgs>(msgs)...);
	}

	void flush();

  protected:
	static size_t strftime(char *str, size_t count);

	template<class... Msgs>
	void print(Msgs&&... msgs)
	{
		(out<<...<<msgs)<<std::endl;
	}

  private:
	std::ostream& out;
	thread_pool<1> pool;
};

template<class... Msgs>
void
logger::log(log_level level, Msgs&&... msgs)
{
	std::string head;
	switch (level) {
	case log_level_debug: head = "[debug]"; break;
	case log_level_info: head = "[info]"; break;
	case log_level_warning: head = "[warning]"; break;
	case log_level_error: head = "[error]"; break;
	case log_level_fatal: head = "[fatal]"; break;
	}

	char time_buf[32];
	strftime(time_buf, sizeof(time_buf));

	pool.thread(&logger::print<std::string&&, decltype(head), decltype(' '), Msgs&&...>,
		this, std::string(time_buf), head, ' ', std::forward<Msgs>(msgs)...);
}

} // namespace log

using logger = log::logger;

} // namespace akm

#endif
