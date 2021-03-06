#include "timer.h"
#include "alloc_check.h"

struct c_utils_timer {
    /// Keeps track of the start time.
	time_t *start;
    /// Keeps track of the end time.
	time_t *end;
	/// Whether or not the timer is started, to prevent undefined behavior if it isn't.
	volatile unsigned char is_running;
	/// Whether or not the timer has been finished, to prevent undefined behavior if it isn't.
	volatile unsigned char is_finished;
};

struct c_utils_timer *c_utils_timer_init(int start) {
	struct c_utils_timer *timer;
	C_UTILS_ON_BAD_CALLOC(timer, NULL, sizeof(*timer))
		goto err;

	C_UTILS_ON_BAD_CALLOC(timer->start, NULL, sizeof(*timer->start))
		goto err_start;
	
	C_UTILS_ON_BAD_CALLOC(timer->end, NULL, sizeof(*timer->end))
		goto err_end;

	if (start)
		time(timer->start);

	timer->is_running = !!start;
	timer->is_finished = 0;

	return timer;

	err_end:
		free(timer->start);
	err_start:
		free(timer);
	err:
		return NULL;
}

int c_utils_timer_start(struct c_utils_timer *timer) {
	if (timer->is_running) 
		return 0;
	
	time(timer->start);
	
	timer->is_running = 1;
	timer->is_finished = 0;
	
	return 1;
}

int c_utils_timer_stop(struct c_utils_timer *timer) {
	if (!timer->is_running || timer->is_finished) 
		return 0;
	
	time(timer->end);
	
	timer->is_running = 0;
	timer->is_finished = 1;
	
	return 1;
}

char *c_utils_timer_string(struct c_utils_timer *timer) {
	if (!timer->is_finished) 
		return NULL;
	
	double total_time = difftime(*timer->end, *timer->start);
	int hours = 0, minutes = 0, seconds = 0;
	
	while ((int)(total_time/3600)) {
		hours++;
		total_time -= 3600;
	}
	
	while ((int)(total_time/60)) {
		minutes++;
		total_time -= 60;
	}
	
	while ((int)total_time) {
		seconds++;
		total_time--;
	}
	
	char *formatted_time;
	asprintf(&formatted_time, "%02d:%02d:%02d", hours, minutes, seconds);
	
	return formatted_time;
}

void c_utils_timer_destroy(struct c_utils_timer *timer) {
	if (!timer)
		return;
	
	free(timer->start);
	free(timer->end);
	free(timer);
}