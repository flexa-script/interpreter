#ifndef WATCH_HPP
#define WATCH_HPP

#include <time.h>
#include <chrono>
#include <string>
#include <sstream>

namespace utils {

#define MILLISECOND 1
#define SECOND (1000 * MILLISECOND)
#define MINUTE (60 * SECOND)
#define HOUR (60 * MINUTE)
#define DAY (24 * HOUR)

#define CHR_NANOSECOND 1
#define CHR_MICROSECOND (1000 * CHR_NANOSECOND)
#define CHR_MILLISECOND (1000 * CHR_MICROSECOND)
#define CHR_SECOND (1000 * CHR_MILLISECOND)

	typedef int64_t watch_t;

	class Watch {
	protected:
		watch_t elapsed;

	public:
		// constructors
		Watch(watch_t elapsed);

		virtual ~Watch() = 0;

		watch_t get_elapsed();

		// control flow
		virtual void start() = 0;
		virtual void stop() = 0;
		virtual void reset() = 0;

		void restart();

		// getters and setters
		virtual std::string get_elapsed_formatted() = 0;
	};

	class Stopwatch : public Watch {
	private:
		watch_t start_time;

	public:
		// constructors
		Stopwatch();

		~Stopwatch();

		// control flow
		void start() override;
		void stop() override;
		void reset() override;

		// getters and setters
		std::string get_elapsed_formatted() override;
	};

	class ChronoStopwatch : public Watch {
	private:
		std::chrono::steady_clock::time_point start_time;

	public:
		// constructors
		ChronoStopwatch();

		~ChronoStopwatch();

		// control flow
		void start() override;
		void stop() override;
		void reset() override;

		// getters and setters
		std::string get_elapsed_formatted() override;
	};

	class Timer {
	private:
		clock_t start_time;
		clock_t interval;

	public:
		// constructors
		Timer();

		Timer(clock_t interval);

		~Timer();

		// control flow
		void start();

		bool timeout();

		// getters and setters
		void set_interval(clock_t interval);
		clock_t get_interval();
	};

}

#endif // !WATCH_HPP
