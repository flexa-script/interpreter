#include "watch.hpp"

using namespace utils;

// Watch

Watch::Watch(watch_t elapsed) : elapsed(elapsed) {}

Watch::~Watch() = default;

watch_t Watch::get_elapsed() {
	return elapsed;
}

void Watch::restart() {
	reset();
	start();
}

// Stopwatch

Stopwatch::Stopwatch() : start_time(0), Watch(0) { }

Stopwatch::~Stopwatch() = default;

void Stopwatch::start() {
	start_time = clock();
}

void Stopwatch::stop() {
	elapsed = clock() - start_time;
}

void Stopwatch::reset() {
	elapsed = 0;
	start_time = 0;
}

std::string Stopwatch::get_elapsed_formatted() {
	watch_t d = 0, h = 0, m = 0, s = 0, mil = 0, aux_elapsed = elapsed;
	d = aux_elapsed / DAY;
	aux_elapsed %= DAY;
	h = aux_elapsed / HOUR;
	aux_elapsed %= HOUR;
	m = aux_elapsed / MINUTE;
	aux_elapsed %= MINUTE;
	s = aux_elapsed / SECOND;
	aux_elapsed %= SECOND;
	mil = aux_elapsed;
	std::stringstream r;
	if (d) {
		r << d << "d ";
	}
	if (h) {
		r << h << "h ";
	}
	if (m) {
		r << m << "m ";
	}
	if (s) {
		r << s << "s ";
	}
	if (mil) {
		r << mil << "mil";
	}
	return r.str();
}

// ChronoStopwatch

ChronoStopwatch::ChronoStopwatch() : start_time(std::chrono::high_resolution_clock::now()), Watch(0) { }

ChronoStopwatch::~ChronoStopwatch() = default;

void ChronoStopwatch::start() {
	start_time = std::chrono::high_resolution_clock::now();
}

void ChronoStopwatch::stop() {
	elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start_time).count();
}

void ChronoStopwatch::reset() {
	elapsed = 0;
	start_time = std::chrono::high_resolution_clock::now();
}

std::string ChronoStopwatch::get_elapsed_formatted() {
	watch_t min = 0, sec = 0, mil = 0, mic = 0, nan = 0, aux_elapsed = elapsed;
	sec = aux_elapsed / CHR_SECOND;
	aux_elapsed %= CHR_SECOND;
	mil = aux_elapsed / CHR_MILLISECOND;
	aux_elapsed %= CHR_MILLISECOND;
	mic = aux_elapsed / CHR_MICROSECOND;
	aux_elapsed %= CHR_MICROSECOND;
	nan = aux_elapsed;
	while (sec >= 60) {
		sec -= 60;
		min++;
	}
	std::stringstream r;
	if (min) {
		r << min << "min ";
	}
	if (sec) {
		r << sec << "sec ";
	}
	if (mil) {
		r << mil << "mil ";
	}
	if (mic) {
		r << mic << "mic ";
	}
	if (nan) {
		r << nan << "nan ";
	}
	return r.str();
}

// Timer

Timer::Timer() : start_time(0), interval(0) { }

Timer::Timer(clock_t interval) : start_time(0), interval(interval) { }

Timer::~Timer() = default;

void Timer::start() {
	start_time = clock();
}

bool Timer::timeout() {
	if (clock() - start_time > interval) {
		start_time = clock();
		return true;
	}
	return false;
}

void Timer::set_interval(clock_t interval) {
	this->interval = interval;
}

clock_t Timer::get_interval() {
	return interval;
}
