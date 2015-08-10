#define _GNU_SOURCE
#include "SU_String.h"

static MU_Logger_t *logger = NULL;

__attribute__((constructor)) static void init_logger(void){
    logger = MU_Logger_create("./String_Utils/Logs/SU_String.log", "w", MU_ALL);
}

__attribute__((destructor)) static void destroy_logger(void){
    MU_Logger_destroy(logger);
}

/*
 * String_Utils has had it's garbage_collector garbage removed from it. For now, even as an experiment, it wasn't
 * worth claiming it as stable when it was unstable in and of itself. The garbage collector may have worked, but it
 * literally did nothing of value except complicate code and even increase the program size by 3x. So, instead, I
 * opted to have another experiment, one which is already heavily tested. The macro TEMP uses the
 * cleanup attribute specific for GCC and Clang compilers, making it 100% dependent on them to compile, but
 * it's easy enough to remove either way. Besides, it accomplished what I want to do. 
 *
 * Of course in the future I most likely (99.999% sure) I'm going to be replacing it entirely from the library functions,
 * as it currently is impossible to use without it being dependent on GCC and POSIX as is. Most likely, it will be apart of the whole C_Utils
 * package. However, I'll go into what TEMP does. What TEMP does is it will call the assigned callback for cleanup when it
 * leaves the variable's scope, most likely a function, but could be inside of a loop if it was declared there. TEMP will basically just free 
 * the string without further ado, preventing any memory leaks and pretty much eliminated needs to call free yourself.
 */

char SU_String_char_at(const String str, unsigned int index){
    MU_ARG_CHECK(logger, '\0', str, str && index <= strlen(str));
    return str[index];
}

bool SU_String_contains(const String str, const String substr, size_t len, bool ignore_case){ 
    MU_ARG_CHECK(logger, false, str, substr);
    size_t str_len = len ? len : strlen(str);
    size_t substr_len = strlen(substr);
    if(substr_len > str_len) return false;
    if(substr_len == str_len) return SU_String_equal(str, substr, len, ignore_case);
    size_t i = 0;
    for(; i < str_len - substr_len; i++){
        char c = str[i];
        if(c == '\0') break;
        if(c == substr[0]){
            if(SU_String_equal(str + i, substr, substr_len, ignore_case)){
                return true;
            }
        }
    }
    return false;
}

String SU_String_to_lowercase(String str, size_t len){
    MU_ARG_CHECK(logger, NULL, str);
    size_t str_len = len ? len : strlen(str), i = 0;
    for(; i < str_len; i++){
        char c = str[i];
        if(c == '\0') break;
        // Since tolower takes an int, and returns an int, I ensure (un)signedness by casting.
        str[i] = (char)tolower((unsigned char)c);
    }
    return str;
}

String SU_String_to_uppercase(String str, size_t len){
    MU_ARG_CHECK(logger, NULL, str);
    size_t str_len = len ? len : strlen(str), i = 0;
    for(; i < str_len; i++){
        char c = str[i];
        if(c == '\0') break;
        // Since tolower takes an int, and returns an int, I ensure (un)signedness by casting.
        str[i] = (char)toupper((unsigned char)c);
    }
    return str;
}

bool SU_String_equal(const String string_one, const String string_two, size_t len, bool ignore_case){
    MU_ARG_CHECK(logger, false, string_one, string_two);
    size_t str_len = len ? len : strlen(string_one);
    bool is_equal = false;
    if(ignore_case){
        is_equal = strncasecmp(string_one, string_two, str_len) == 0;
    } else {
        is_equal = strncmp(string_one, string_two, str_len) == 0;
    }
    return is_equal;
}

String *SU_String_split(const String str, size_t len, const String delimiter, size_t *size){
    MU_ARG_CHECK(logger, NULL, str, delimiter);
    size_t str_len = len ? len : strlen(str), num_strings = 0;
    char str_copy[str_len + 1];
    snprintf(str_copy, str_len, "%s", str);
    char **split_strings = malloc(sizeof(char *));
    if(!split_strings){
        MU_LOG_ASSERT(logger, "malloc: '%s'", strerror(errno));
        goto error;
    }
    char *saveptr;
    char *curr_string = strtok_r(str_copy, delimiter, &saveptr);
    if(!curr_string){ 
        free(split_strings);
        return NULL; 
    } 
    do {
        char **tmp = realloc(split_strings, (sizeof(char *) * (num_strings + 1)));
        if(!tmp){
            MU_LOG_ASSERT(logger, "realloc: '%s'", strerror(errno));
            goto error;
        }
        split_strings = tmp;
        if(!(split_strings[num_strings] = malloc(strlen(curr_string) + 1))){
            MU_LOG_ASSERT(logger, "malloc: '%s'", strerror(errno));
            goto error;
        }
        sprintf(split_strings[num_strings++], "%s", curr_string);
    } while((curr_string = strtok_r(NULL, delimiter, &saveptr)));
    *size = num_strings;
    return split_strings;

    error:
        if(split_strings){
            int i = 0;
            for(; i < num_strings; i++){
                free(split_strings[i]);
            }
        }
        return NULL;
}

String SU_String_reverse(String str, size_t len){
    MU_ARG_CHECK(logger, NULL, str);
    size_t str_len = len ? len : strlen(str);
    int i = 0, j = str_len - 1;
    for(; i < j; i++, j--){
        char c = str[i];
        str[i] = str[j];
        str[j] = c;
    }
    return str;
}

String SU_String_replace(String str, char old_char, char new_char, size_t len, bool ignore_case){
    MU_ARG_CHECK(logger, NULL, str);
    size_t str_len = len ? len : strlen(str);
    int i = 0;
    char search_char = ignore_case ? tolower((unsigned char) old_char) : old_char;
    for(; i < str_len; i++){
        char curr_char = ignore_case ? tolower((unsigned char) str[i]) : str[i];
        if(curr_char == '\0') break;
        if(search_char == curr_char){
            str[i] = new_char;
        }
    }
    return str;
}

String SU_String_join(const String arr[], const String delimiter, size_t size){
    MU_ARG_CHECK(logger, NULL, arr, delimiter);
    size_t buf_size = BUFSIZ;
    char *buf = calloc(1, buf_size + 1);
    if(!buf){
        MU_LOG_ASSERT(logger, "malloc: '%s'", strerror(errno));
        goto error;
    }
    size_t allocated = 0, size_left = buf_size, i = 0;
    for(; i < size; i++){
        char *curr_string = arr[i];
        size_t str_len = strlen(curr_string);
        // If this is not the last iteration, we need to make room for the delimiter.
        if(i != (size - 1)){
            str_len += strlen(delimiter);
        }
        while(size_left < str_len){
            buf_size *= 2;
            char *tmp = realloc(buf, buf_size + 1);
            if(!tmp){
                MU_LOG_ASSERT(logger, "realloc: '%s'", strerror(errno));
                goto error;
            }
            buf = tmp;
            size_left = buf_size - allocated;
        }
        // If this is the last iteration, do not append the delimiter.
        if(i == (size - 1)){
            sprintf(buf, "%s%s", buf, curr_string);
        } else {
            sprintf(buf, "%s%s%s", buf, curr_string, delimiter);
        }
        allocated += str_len;
        size_left -= str_len;
    }
    // Shrink the buffer size to the actual string size.
    char *tmp = realloc(buf, strlen(buf) + 1);
    if(!tmp){
        MU_LOG_ASSERT(logger, "realloc: '%s'", strerror(errno));
        goto error;
    }
    return buf;

    error:
        if(buf){
            free(buf);
        }
        return NULL;
}

bool SU_String_starts_with(const String str, const String find, bool ignore_case){
    MU_ARG_CHECK(logger, false, str, find);
    size_t str_len = strlen(str), find_len = strlen(find);
    if(find_len > str_len) return false;
    else return SU_String_equal(str, find, find_len, ignore_case);
}

bool SU_String_ends_with(const String str, const String find, bool ignore_case){
    MU_ARG_CHECK(logger, false, str, find);
    size_t str_len = strlen(str), find_len = strlen(find), offset = str_len - find_len;;
    if(find_len > str_len) return false;
    else return SU_String_equal(str + offset, find, find_len, ignore_case);
}

String SU_String_trim(String *str_ptr, size_t len){
    MU_ARG_CHECK(logger, NULL, str_ptr, str_ptr && *str_ptr);
    char *str = *str_ptr;
    size_t str_len = len ? len : strlen(str);
    char *buf = malloc(str_len + 1); 
    if(!buf){
        MU_LOG_ASSERT(logger, "malloc: '%s'", strerror(errno));
        goto error;
    }
    size_t i = 0, j = str_len - 1;
    for(; i < str_len; i++){
        if(!isspace((unsigned char)str[i])) break;
    }
    for(; j > i; j--){
        if(!isspace((unsigned char)str[j])) break;
    }
    /* 
        We must now copy the string from the offset past any empty spaces at the beginning and before the end.
        +2 because +1 for null terminator, and +1 because we start at index 0 for j, so we offset for it.
    */
    snprintf(buf, j - i + 2, "%s", str + i);
    // Now we shrink it to the new size.
    char *tmp = realloc(buf, strlen(buf) + 1);
    if(!tmp){
        MU_LOG_ASSERT(logger, "realloc: '%s'", strerror(errno));
        goto error;
    }
    *str_ptr = (buf = tmp);
    return buf;

    error:
        if(buf){
            free(buf);
        }
        return NULL;
}

String SU_String_substring(String str, unsigned int offset, unsigned int end){
    MU_ARG_CHECK(logger, NULL, str);
    if(end && offset > end){
        return NULL;
    }
    size_t new_end = end ? end : strlen(str);
    size_t buf_size = (new_end - offset);
    char *buf = malloc(buf_size + 1);
    if(!buf){
        MU_LOG_ASSERT(logger, "malloc: '%s'", strerror(errno));
        goto error;
    }
    snprintf(buf, buf_size, "%s", str + offset);
    // In case a null terminator was found, it will shrink the buffer to actual size.
    char *tmp = realloc(buf, strlen(buf) + 1);
    if(!tmp){
        MU_LOG_ASSERT(logger, "realloc: '%s'", strerror(errno));
        goto error;
    }
    buf = tmp;
    return buf;

    error:
        if(buf){
            free(buf);
        }
        return NULL;
}

int SU_String_index_of(const String str, const String substr, size_t len, bool ignore_case){
    MU_ARG_CHECK(logger, -1, str, substr);
    size_t substr_len = strlen(substr), str_len = len ? len : strlen(str);
    if(substr_len > str_len) return -1;
    if(substr_len == str_len){
        if(SU_String_equal(str, substr, len, ignore_case)) return 0;
        return -1;
    }
    size_t i = 0;
    for(; i < str_len - substr_len; i++){
        char c = str[i];
        if(c == '\0') break;
        if(c == substr[0]){
            if(SU_String_equal(str + i, substr, substr_len, ignore_case)){
                return i;
            }
        }
    }
    return -1;
}

unsigned int SU_String_count(const String str, const String substr, size_t len, bool ignore_case){
    MU_ARG_CHECK(logger, 0, str, substr);
    size_t str_len = len ? len : strlen(str), substr_len = strlen(substr), i = 0, count = 0;
    if(substr_len > str_len) return 0;
    if(substr_len == str_len){
        if(SU_String_equal(str, substr, str_len, ignore_case)) return 1;
        return 0;
    }
    for(; i < str_len - substr_len; i++){
        char c = str[i];
        if(c == '\0') break;
        if(c == substr[0]){
            if(SU_String_equal(str + i, substr, substr_len, ignore_case)){
                count++;
                i += (substr_len - 1);
            }
        }
    }
    return count;
}

// Reimplement function to use strcasestr().
String SU_String_between(const String str, const String start, const String end, size_t len, bool ignore_case){
    MU_ARG_CHECK(logger, NULL, str, start, end);
    size_t str_len = len ? len : strlen(str), start_len = strlen(start), end_len = strlen(end), i = 0, str_left = str_len;
    size_t start_offset = 0, end_offset = 0;
    // Easy stack allocated buffer for non-null terminated strings.
    char buf[str_len + 1];
    snprintf(buf, str_len, "%s", str);
    // Attempt to find the offset of start.
    for(; i < str_len; i++){
        bool char_equal;
        if(ignore_case){
            char_equal = (tolower((unsigned char)str[i]) == tolower((unsigned char)start[0]));
        } else {
            char_equal = str[i] == start[0];
        }
        if(char_equal){
            if(SU_String_equal(str, start, start_len, ignore_case)){
                start_offset = i;
                i += start_len;
                break;
            }
        }
        str_left--;
    }
    // Check if we even have enough left in the string to search for the end.
    if(end_len >= str_left) return NULL;
    // TODO: Continue
}

void String_Utils_destroy(char **str){
    free(*str);
}
