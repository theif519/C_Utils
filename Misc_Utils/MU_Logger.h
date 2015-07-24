#ifndef MISC_UTILS_H
#define MISC_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <assert.h>

/*
 * Miscallaneous Utilities for the C Programming Language, or Misc_Utils, is a collection,
 * albeit a small one, of utilities that aren't big enough to deserve it's own package.
 * 
 * Inside, it contains basic logging macros to log information to a file, a debugging
 * macro to output information to stderr in a prettified format. It should also be
 * noted that the logging functions also show the name of the file and the line where 
 * the macro is invoked, thanks to GCC magic.
 * 
 * Inside also is a timer, which is extremely bare bones for now, and allows you
 * to start, stop and obtain the total time in string format.
 */

/**
 * @brief The logger level associated with each logging macro.
 * 
 * The logger level determines precisely what gets printed, as any log macros that's minimum-requirement is below
 * the logger's level, will not log a message. For instance, if you select MU_ALL, then all messages are displayed, however
 * if you choose MU_ERROR, then only ERROR and ASSERT messages are displayed. MU_CUSTOM allows the user to define the prefix
 * to appear inside of the brackets, and also does not contain the file and line number.
 */
typedef enum {
	/// Display all types of warnings.
	MU_ALL = 0,
	/// Display Verbose, Info, Custom, Warnings, Errors and Assertions only.
	MU_VERBOSE,
	/// Display Info, Custom, Warnings, Errors and Assertions only.
	MU_INFO,
	/// Display Custom, Warnings, Errors and Assertions only.
	MU_CUSTOM,
	/// Display Warnings, Errors and Assertions only.
	MU_WARNING,
	/// Display Errors and Assertions only.
	MU_ERROR,
	/// Display Assertions only.
	MU_ASSERTION,
	/// Does not display any Warnings, Errors, or even Assertions.
	MU_NONE
} MU_Logger_Level_e;

typedef struct {
	char *all_f;
	char *verbose_f;
	char *info_f;
	char *custom_f;
	char *warning_f;
	char *error_f;
	char *assertion_f;
	char *default_f;
} MU_Logger_Format_t;

/**
 * @brief Logger utility to log various types of messages according to different MU_Logger_Level_t levels.
 * 
 * The logger will log any messages that are above the logger level, and if any are above, is macro dependent.
 * For exmaple, MU_LOG_WARNING will do nothing if the logger level is above MU_WARNING level, but something like
 * MU_ASSERT or MU_ASSERT_RETURN will call assert or return respectively while not logging the condition at all.
 * 
 * It should also be noted that, if the logger has not been created or if errors occur during initialization barring
 * ones that cause a segmentation fault, as in the cases where *alloc return NULL or if the logfile failed to be created,
 * then it would count as if the logger level was above the macro's minimum-required level.
 */
typedef struct {
	/// The log file to write to.
	FILE *file;
	/// The formatter for log strings.
	MU_Logger_Format_t format;
	/// The level determining what messages will be printed.
	MU_Logger_Level_e level;
} MU_Logger_t;

#ifdef NDEBUG
/// If NDEBUG is defined, then MU_DEBUG becomes a NOP.
#define MU_DEBUG(message, ...)
#else
/// Prints a debug message, along with it's timestamp, file and line of code it's on.
#define MU_DEBUG(message, ...) do { \
	char *timestamp = MU_get_timestamp(); \
	fprintf(stderr, "%s: [DEBUG]: %s: " message "\n", timestamp, __FUNCTION__, ##__VA_ARGS__); \
	free(timestamp); \
} while(0)
#endif
/// An assertion which prints to stderr, the logfile and also shows the file and line that triggered it as well as timestamp.
#define MU_ASSERT(condition, logger, message, ...) !condition ? MU_Logger_Log(logger, MU_ASSERTION, NULL, message, MU_LOGGER_STRINGIFY(condition), \
	__FILE__, MU_LOGGER_STRINGIFY(__LINE__), __FUNCTION__, ##__VA_ARGS__), exit(EXIT_FAILURE) : false;

/// Simply log an assertion to the logfile rather than doing anything.
#define MU_LOG_ASSERT(logger, message, ...) MU_Logger_Log(logger, MU_ASSERTION, NULL, message, NULL, \
	__FILE__, MU_LOGGER_STRINGIFY(__LINE__), __FUNCTION__, ##__VA_ARGS__)

/// Log an error message along with timestamp, file and line of code.
#define MU_LOG_ERROR(logger, message, ...) MU_Logger_Log(logger, MU_ERROR, NULL, message, NULL, \
	__FILE__, MU_LOGGER_STRINGIFY(__LINE__), __FUNCTION__, ##__VA_ARGS__)

/// Log a warning message along with timestamp, file and line of code.
#define MU_LOG_WARNING(logger, message, ...) MU_Logger_Log(logger, MU_WARNING, NULL, message, NULL, \
	__FILE__, MU_LOGGER_STRINGIFY(__LINE__), __FUNCTION__, ##__VA_ARGS__)

/// Log a customer message along with timestamp, without file and line of code.
#define MU_LOG_CUSTOM(logger, custom_level, message, ...) MU_Logger_Log(logger, MU_CUSTOM, custom_level, message, NULL, \
	__FILE__, MU_LOGGER_STRINGIFY(__LINE__), __FUNCTION__, ##__VA_ARGS__)

/// Log an info message along with timestamp, file and line of code.
#define MU_LOG_INFO(logger, message, ...) MU_Logger_Log(logger, MU_INFO, NULL, message, NULL, \
	__FILE__, MU_LOGGER_STRINGIFY(__LINE__), __FUNCTION__, ##__VA_ARGS__)

/// Log a verbose message along with timestamp, file and line of code. This differs from INFO in that it is used for
/// logging almost trivial information, good for if you want to log every single thing without having to remove it later.
#define MU_LOG_VERBOSE(logger, message, ...) MU_Logger_Log(logger, MU_VERBOSE, NULL, message, NULL, \
	__FILE__, MU_LOGGER_STRINGIFY(__LINE__), __FUNCTION__, ##__VA_ARGS__)

#define MU_LOGGER_STRINGIFY(x) MU_LOGGER_STRINGIFY_THIS(x)
#define MU_LOGGER_STRINGIFY_THIS(x) #x

/**
 * @brief Initialize a logger passed to it.
 * 
 * Will initialize a logger passed to it, as long as logger, filename and the mode passed
 * are not NULL. Hence, logger must be allocated before being passed to this function.
 * If the filename or mode are invalid, and file isn't able to be created, then it returns 0
 * for failure. The level passed determines whether or not the log macros actually log the
 * message to file; if the level is not greater than the macro's level, it will log it to file.
 * 
 * @param logger Logger to Initialize.
 * @param filename Name of the log file to be created.
 * @param mode Mode to open the file in, I.E 'w', 'r', 'rw'.
 * @param level The minimum level of logging to be processed. Anything below it will be ignored.
 * @return 1 if successful, 0 if logger, filename or mode are NULL or if unable to open the file.
 */
MU_Logger_t *MU_Logger_create(const char *filename, const char *mode, MU_Logger_Level_e level);

/// Returns the format for the log level, and if not initialized, returns all, then falls back to default if not specified either.
const char *MU_Logger_Format_get(MU_Logger_t *logger, MU_Logger_Level_e level);

const char *MU_Logger_Level_to_string(MU_Logger_Level_e level);

/// Used by the logger to directly log a message. Use macro for logging!
bool MU_Logger_Log(MU_Logger_t *logger, MU_Logger_Level_e level, const char *custom_level, const char *msg, const char *cond, const char *file_name, const char *line_number, const char *function_name, ...);

/**
 * @brief Destroys the logger passed to it, freeing it if flagged.
 * @param logger Logger to be destroyed.
 * @return 1 on success, 0 if logger is NULL.
 */
int MU_Logger_destroy(MU_Logger_t *logger);

/**
 * Obtain the current timestamp in Hours:Minutes:Seconds AM/PM. (I.E 11:45:30 AM)
 * @return Formatted timestamp.
 */
char *MU_get_timestamp(void);

#endif