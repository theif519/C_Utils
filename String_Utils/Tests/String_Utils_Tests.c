#include <String_Utils.h>
#include <string.h>
#include <stdlib.h>
#include <Misc_Utils.h>

MU_Logger_t *logger;

#define TEST(condition, test) do { \
    if(!(condition)) FAILED(test); \
    MU_ASSERT_RETURN(condition, logger, 0); \
} while(0)
#define TEST_EQUAL(string_one, string_two, test) do { \
    int cmp_result = 0; \
    if((cmp_result = strcmp(string_one, string_two)) != 0) FAILED(test); \
    MU_ASSERT_RETURN(strcmp(string_one, string_two) == 0, logger, 0); \
} while(0)
#define BOOL(number)(number ? "True" : "False")
#define PASSED(test) MU_LOG_INFO(logger, "Passed Test: \"%s\"\n", test)
#define SKIP(test) MU_LOG_WARNING(logger, "Skipped Test: \"%s\"\n", test)
#define FAILED(test) MU_LOG_ERROR(logger, "Failed Test: \"%s\"\n", test)
#define TEST_ALL(results) do{\
    results += testString_Utils_capitalize();\
    results += testString_Utils_char_at();\
    results += testString_Utils_compare();\
    results += testString_Utils_concat();\
    results += testString_Utils_count();\
    results += testString_Utils_ends_with();\
    results += testString_Utils_equals();\
    results += testString_Utils_from();\
    results += testString_Utils_from_token();\
    results += testString_Utils_index_of();\
    results += testString_Utils_join();\
    results += testString_Utils_replace();\
    results += testString_Utils_reverse();\
    results += testString_Utils_set();\
    results += testString_Utils_split();\
    results += testString_Utils_starts_with();\
    results += testString_Utils_substring();\
    results += testString_Utils_to_lowercase();\
    results += testString_Utils_to_uppercase();\
    results += testString_Utils_trim();\
} while(0)
int testString_Utils_capitalize() {
    char test[] = "Capitalize";
    MU_LOG_VERBOSE(logger, "\nTesting: %s!\n", test);
    char *string TEMP = strdup("hello World");
    MU_LOG_VERBOSE(logger, "Declared string: \"%s\"\n", string);
    char *result_one TEMP = String_Utils_capitalize(&string, SU_NONE);
    MU_LOG_VERBOSE(logger, "Capitalized declared string: \"%s\"\n", result_one);
    String_Utils_capitalize(&string, SU_MODIFY);
    MU_LOG_VERBOSE(logger, "Modified declared string: \"%s\"\n", string);
    TEST_EQUAL(result_one, "Hello World", test);
    TEST_EQUAL(string, result_one, test);
    PASSED(test);
    return 1;
}

int testString_Utils_char_at() {
    char test[] = "Char_At";
    MU_LOG_VERBOSE(logger, "\nTesting: %s!\n", test);
    const char *string = "Hello World";
    unsigned int index_one = 100;
    unsigned int index_two = 5;
    char result_one = String_Utils_char_at(string, index_one);
    MU_LOG_VERBOSE(logger, "Retrieved char in \"%s\" at index: %d is '%c'\n", string, index_one, result_one);
    char result_two = String_Utils_char_at(string, index_two);
    MU_LOG_VERBOSE(logger, "Retrieved char in \"%s\" at index: %d is '%c'\n", string, index_two, result_two);
    TEST(string[10] == result_one, test);
    TEST(string[5] == result_two, test);
    PASSED(test);
    return 1;
}

int testString_Utils_compare() {
    char test[] = "Compare";
    MU_LOG_VERBOSE(logger, "\nTesting: %s!\n", test);
    const char *string_one = "Hello World";
    const char *string_two = "Hello_World";
    const char *string_three = "Hello WorlD";
    int parameter_one = SU_IGNORE_CASE;
    int parameter_two = SU_NONE;
    int result_one = String_Utils_compare(string_one, string_two, SU_NONE);
    int result_two = String_Utils_compare(string_one, string_three, SU_IGNORE_CASE);
    int result_three = String_Utils_compare(string_one, string_three, SU_NONE);
    MU_LOG_VERBOSE(logger, "(Sensitive) The comparison of \"%s\" and \"%s\": %s\n", string_one, string_two, BOOL(result_one == 0));
    TEST(result_one != 0, test);
    MU_LOG_VERBOSE(logger, "(Insensitive) The comparison of \"%s\" and \"%s\": %s\n", string_one, string_three, BOOL(result_two == 0));
    TEST(result_two == 0, test);
    MU_LOG_VERBOSE(logger, "(Sensitive) The comparison of \"%s\" and \"%s\": %s\n", string_one, string_three, BOOL(result_three == 0));
    TEST(result_three != 0, test);
    PASSED(test); 
    return 1;
}

int  testString_Utils_concat() {
    char test[] = "Concat";
    MU_LOG_VERBOSE(logger, "\nTesting: %s!\n", test);
    char *string_one = "Hello ";
    const char *string_two = "World";
    char *string_three TEMP = strdup("Modify this string: ");
    MU_LOG_VERBOSE(logger, "Declared string: \"%s\"\n", string_three);
    char *result_one TEMP = String_Utils_concat(&string_one, string_two, SU_NONE);
    String_Utils_concat(&string_three, result_one, SU_MODIFY); // Modifies string_three
    MU_LOG_VERBOSE(logger, "Concatenation of \"%s\" and \"%s\": \"%s\"\n", string_one, string_two, result_one);
    TEST_EQUAL(result_one, "Hello World", test);
    MU_LOG_VERBOSE(logger, "Modified declared string: \"%s\"\n", string_three);
    TEST_EQUAL("Modify this string: Hello World", string_three, test);
    PASSED(test);
    return 1;
}

int  testString_Utils_contains() {
    char test[] = "Contains";
    MU_LOG_VERBOSE(logger, "\nTesting: %s!\n", test);
    const char *string = "Hello World, the weather is nice today, isnt it?";
    const char *search = "The";
    int result_one = String_Utils_contains(string, search, SU_IGNORE_CASE);
    int result_two = String_Utils_contains(string, search, SU_NONE);
    MU_LOG_VERBOSE(logger, "Insensitive search in string \"%s\" for \"%s\": %s\n", string, search, BOOL(result_one));
    TEST(result_one == 1, test);
    MU_LOG_VERBOSE(logger, "Sensitive search in string \"%s\" for \"%s\": %s\n", string, search, BOOL(result_one));
    TEST(result_two == 0, test);
    PASSED(test);
    return 1;
}


int  testString_Utils_count() {
    char test[] = "Count";
    MU_LOG_VERBOSE(logger, "\nTesting: %s!\n", test);
    const char *string = "What is the meaning of the word the, when there is the person in the mirror staring back at the recipient? The answer is unclear.";
    const char *delimiter = "The";
    int result_one = String_Utils_count(string, delimiter, SU_IGNORE_CASE);
    int result_two = String_Utils_count(string, delimiter, SU_NONE);
    MU_LOG_VERBOSE(logger, "(Insensitive) Amount of times the string \"%s\" contains the substring \"%s\": %d\n", string, delimiter, result_one);
    TEST(result_one == 8, test);
    MU_LOG_VERBOSE(logger, "(Sensitive) Amount of times the string \"%s\" contains the substring \"%s\": %d\n", string, delimiter, result_two);
    TEST(result_two == 1, test);
    PASSED(test);
    return 1;
}

int  testString_Utils_ends_with() {
    char test[] = "Ends_With";
    MU_LOG_VERBOSE(logger, "\nTesting: %s!\n", test);
    const char *string = "Catch the end of this string";
    const char *find_one = "string";
    const char *find_two = "END OF THIS STRING";
    int result_one = String_Utils_ends_with(string, find_one, SU_IGNORE_CASE);
    int result_two = String_Utils_ends_with(string, find_two, SU_IGNORE_CASE);
    int result_three = String_Utils_ends_with(string, find_two, SU_NONE);
    MU_LOG_VERBOSE(logger, "(Insensitive) String \"%s\" ends with \"%s\": %s\n", string, find_one, BOOL(result_one));
    TEST(result_one == 1, test);
    MU_LOG_VERBOSE(logger, "(Insensitive) String \"%s\" ends with \"%s\": %s\n", string, find_two, BOOL(result_two));
    TEST(result_two == 1, test);
    MU_LOG_VERBOSE(logger, "(Sensitive) String \"%s\" ends with \"%s\": %s\n", string, find_two, BOOL(result_three));
    TEST(result_three == 0, test);
    PASSED(test);
    return 1;
}

int  testString_Utils_equals() {
    char test[] = "Equals";
    MU_LOG_VERBOSE(logger, "\nTesting: %s!\n", test);
    const char *string_one = "Check to see if this equals another string!";
    const char *string_two = "CHECK TO SEE IF THIS EQUALS ANOTHER STRING!";
    int result_one = String_Utils_equals(string_one, string_two, SU_IGNORE_CASE);
    int result_two = String_Utils_equals(string_one, string_two, SU_NONE);
    MU_LOG_VERBOSE(logger, "(Insensitive) String \"%s\" is equal to \"%s\": %s\n", string_one, string_two, BOOL(result_one));
    TEST(result_one == 1, test);
    MU_LOG_VERBOSE(logger, "(Sensitive) String \"%s\" is equal to \"%s\": %s\n", string_one, string_two, BOOL(result_two));
    TEST(result_two == 0, test);
    PASSED(test);
    return 1;
}

int  testString_Utils_from() {
    char test[] = "From";
    MU_LOG_VERBOSE(logger, "\nTesting: %s!\n", test);
    char *string = "Please get everything past here: I am an idiot!";
    char *mutable_string TEMP = strdup("Modify this!");
    MU_LOG_VERBOSE(logger, "Declared string: \"%s\"\n", mutable_string);
    unsigned int index_one = 33;
    unsigned int index_two = 9001;
    char *result_one TEMP = String_Utils_from(&string, index_one, SU_NONE);
    char *result_two TEMP = String_Utils_from(&string, index_two, SU_NONE);
    MU_LOG_VERBOSE(logger, "The substring of \"%s\" from index %d is \"%s\"\n", string, index_one, result_one);
    TEST_EQUAL(result_one, "I am an idiot!", test);
    MU_LOG_VERBOSE(logger, "The substring of \"%s\" from index %d is \"%s\"\n", string, index_two, result_two);
    TEST_EQUAL(result_two, "!", test);
    String_Utils_from(&mutable_string, 7, SU_MODIFY);
    MU_LOG_VERBOSE(logger, "Modified declared string from index 7: \"%s\"\n", mutable_string);
    TEST_EQUAL(mutable_string, "this!", test);
    PASSED(test);
    return 1;
}

int  testString_Utils_from_token() {
    char test[] = "From_Token";
    MU_LOG_VERBOSE(logger, "\nTesting: %s!\n", test);
    char *string TEMP = strdup("Please token above: BLAH BLAH BLAH USELESS INFO <Parse_Me>int:32;char*:Hello World;void*:NULL;<Parse_Me> BLAH BLAH BLAH USELESS INFO!");
    MU_LOG_VERBOSE(logger, "Declared string: \"%s\"\n", string);
    const char *delimiter = "<Parse_Me>";
    char *result_one TEMP = String_Utils_from_token(&string, delimiter, SU_NONE);
    //char *result_two TEMP = String_Utils_from_token(&string, delimiter, SU_LAST);
    MU_LOG_VERBOSE(logger, "The string \"%s\" from token \"%s\" is \"%s\"\n", string, delimiter, result_one);
    TEST_EQUAL(result_one, "<Parse_Me>int:32;char*:Hello World;void*:NULL;<Parse_Me> BLAH BLAH BLAH USELESS INFO!", test);
    //MU_LOG_VERBOSE(logger, "The string \"%s\" from last token \"%s\" is \"%s\"\n", string, delimiter, result_two);
    //TEST_EQUAL(result_two, "<Parse_Me> BLAH BLAH BLAH USELESS INFO!", test);
    String_Utils_from_token(&string, delimiter, SU_MODIFY);
    MU_LOG_VERBOSE(logger, "Modified declared string: \"%s\"\n", string);
    TEST_EQUAL(string, result_one, test);
    PASSED(test);
    return 1;
}

int  testString_Utils_index_of() {
    char test[] = "Index_Of";
    MU_LOG_VERBOSE(logger, "\nTesting: %s!\n", test);
    const char *string = "AMANDA, the Panda exclaimed that, her name is of high enough importnace to be spelled in all caps: amanda, the panda!";
    const char *token = "AMANDA, the Panda";
    MU_LOG_VERBOSE(logger, "The string to be searched is \"%s\", the token to be searched for is \"%s\"\n", string, token);
    int result_one = String_Utils_index_of(string, token, SU_IGNORE_CASE | SU_LAST);
    int result_two = String_Utils_index_of(token, string, SU_NONE);
    int result_three = String_Utils_index_of(string, token, SU_LAST);
    char result_one_char = string[result_one];
    char result_two_char = token[result_two];
    char result_three_char = string[result_three];
    MU_LOG_VERBOSE(logger, "(Insensitive) The last token in the string begins with the char '%c'\n", result_one_char);
    TEST(result_one_char == 'a', test);
    MU_LOG_VERBOSE(logger, "Searching for impossible results should return a null terminator: %s\n", BOOL(result_two_char == '\0'));
    TEST(result_two_char == '\0', test);
    MU_LOG_VERBOSE(logger, "(Sensitive) The last token in the string begins with the char '%c'\n", result_three_char);
    TEST(result_three_char == 'A', test);
    PASSED(test);
    return 1;
}

int  testString_Utils_join() {
    char test[] = "Join";
    MU_LOG_VERBOSE(logger, "\nTesting: %s!\n", test);
    const char** array_of_strings = malloc(sizeof(char *) * 4);
    array_of_strings[0] = "One prison"; array_of_strings[1] = "One person";
    array_of_strings[2] = "One bond"; array_of_strings[3] = "One Power!";
    char *delimiter = ", ";
    size_t size = 4;
    MU_LOG_VERBOSE(logger, "The array of strings to be joined contain the following: \"%s\", \"%s\", \"%s\", \"%s\"\n", 
        array_of_strings[0], array_of_strings[1], array_of_strings[2], array_of_strings[3]);
    char *result_one TEMP = String_Utils_join(array_of_strings, delimiter, size);
    MU_LOG_VERBOSE(logger, "The joined string with the delimiter \"%s\" became \"%s\"\n", delimiter, result_one);
    TEST_EQUAL(result_one, "One prison, One person, One bond, One Power!", test);
    free(array_of_strings);
    PASSED(test);
    return 1;
}

int  testString_Utils_replace() {
    char test[] = "Replace";
    MU_LOG_VERBOSE(logger, "\nTesting: %s!\n", test);
    char *string TEMP = strdup("LOlOlOl I love my sOul enough to bOwl with a fruit cannOli dipped in raviOli");
    MU_LOG_VERBOSE(logger, "Declared string to be modified and operated on: \"%s\"\n", string);
    char old_char = 'O';
    char new_char = 'e';
    char *result_one TEMP = String_Utils_replace(&string, old_char, new_char, SU_NONE);
    char *result_two TEMP = String_Utils_replace(&string, old_char, new_char, SU_IGNORE_CASE);
    MU_LOG_VERBOSE(logger, "(Sensitive) Replaced all chars '%c' with char '%c', resulting in \"%s\"\n", old_char, new_char, result_one);
    TEST_EQUAL(result_one, "Lelelel I love my seul enough to bewl with a fruit canneli dipped in ravieli", test);
    MU_LOG_VERBOSE(logger, "(Insensitive) Replaced all chars '%c' with char '%c', resulting in \"%s\"\n", old_char, new_char, result_two);
    TEST_EQUAL(result_two, "Lelelel I leve my seul eneugh te bewl with a fruit canneli dipped in ravieli", test);
    String_Utils_replace(&string, old_char, new_char, SU_MODIFY | SU_IGNORE_CASE);
    MU_LOG_VERBOSE(logger, "Replaced declared string with previous result: \"%s\"\n", string);
    TEST_EQUAL(string, result_two, test);
    PASSED(test);
    return 1;
}

int  testString_Utils_reverse() {
    char test[] = "Reverse";
    MU_LOG_VERBOSE(logger, "\nTesting: %s!\n", test);
    char *string TEMP = strdup("stressed");
    MU_LOG_VERBOSE(logger, "Declared string: \"%s\"\n", string);
    int parameter_one = SU_NONE;
    int parameter_two = SU_MODIFY;
    char *result_one TEMP = String_Utils_reverse(&string, parameter_one);
    MU_LOG_VERBOSE(logger, "Reversed the declared string to \"%s\"\n", result_one);
    TEST_EQUAL(result_one, "desserts", test);
    MU_LOG_VERBOSE(logger, "Modified declared string: \"%s\"\n", string);
    String_Utils_reverse(&string, parameter_two);
    TEST_EQUAL(string, result_one, test);
    PASSED(test);
    return 1;
}

int  testString_Utils_set() {
    char test[] = "Set";
    MU_LOG_VERBOSE(logger, "\nTesting: %s!\n", test);
    char *string_one TEMP = strdup("SU_MODIFY THIS!");
    const char *string_two = "Hello world, bloody beautiful day isnt it? Have fun while Im stuck inside testing for hours, and hours! Bastard.";
    MU_LOG_VERBOSE(logger, "Declared string as \"%s\"\n", string_one);
    String_Utils_set(&string_one, string_two);
    MU_LOG_VERBOSE(logger, "Modified declared string: \"%s\"\n", string_one);
    TEST_EQUAL(string_one, string_two, test);
    PASSED(test);
    return 1;
}

int  testString_Utils_split() {
    char test[] = "Split";
    MU_LOG_VERBOSE(logger, "\nTesting: %s!\n", test);
    const char *string = "How else, says the Guard, does one continue along the path of righteousness, except by destroying all that is not righteous?";
    const char *delimiter = ",";
    MU_LOG_VERBOSE(logger, "The string to split with delimiter \"%s\": \"%s\"\n", delimiter, string);
    size_t* size = malloc(sizeof(size_t));
    char** result = String_Utils_split(string, delimiter, size);
    int i = 0;
    MU_LOG_VERBOSE(logger, "The split strings are \"%s\", \"%s\", \"%s\", \"%s\"\n", result[0], result[1], result[2], result[3]);
    TEST_EQUAL(result[0], "How else", test);
    TEST_EQUAL(result[1], " says the Guard", test);
    TEST_EQUAL(result[2], " does one continue along the path of righteousness", test);
    TEST_EQUAL(result[3], " except by destroying all that is not righteous?", test);
    for(i; i < *size; i++) free(result[i]);
    free(result);
    free(size);
    PASSED(test);
    return 1;
}

int  testString_Utils_starts_with() {
    char test[] = "Starts_With";
    MU_LOG_VERBOSE(logger, "\nTesting: %s!\n", test);
    const char *string = "Is it me? Who else would it be, fool!";
    const char *find = "is it me?";
    int result_one = String_Utils_starts_with(string, find, SU_IGNORE_CASE);
    int result_two = String_Utils_starts_with(string, find, SU_NONE);
    MU_LOG_VERBOSE(logger, "(Insensitive) The string \"%s\" starts with \"%s\": %s\n", string, find, BOOL(result_one));
    TEST(result_one == 1, test);
    MU_LOG_VERBOSE(logger, "(Sensitive) The string \"%s\" starts with \"%s\": %s\n", string, find, BOOL(result_two));
    TEST(result_two == 0, test);
    PASSED(test);
    return 1;
}

int  testString_Utils_substring() {
    char test[] = "Substring";
    MU_LOG_VERBOSE(logger, "\nTesting: %s!\n", test);
    char *string TEMP = strdup("Below me lies the dragon... not just any dragon, the dragon called *gasp* *chokes* *dies*");
    MU_LOG_VERBOSE(logger, "Declared string: \"%s\"\n", string);
    unsigned int begin = 28;
    unsigned int end = 9001;
    char *result TEMP = String_Utils_substring(&string, begin, end, SU_NONE);
    MU_LOG_VERBOSE(logger, "Parsing out substring from index %d to %d: \"%s\"\n", begin, end, result);
    TEST_EQUAL(result, "not just any dragon, the dragon called *gasp* *chokes* *dies*", test);
    String_Utils_substring(&string, begin, end, SU_MODIFY);
    MU_LOG_VERBOSE(logger, "Modified declared string: \"%s\"\n", string);
    TEST_EQUAL(string, result, test);
    PASSED(test);
    return 1;
}

int  testString_Utils_to_lowercase() {
    char test[] = "To_Lowercase";
    MU_LOG_VERBOSE(logger, "\nTesting: %s!\n", test);
    char *string TEMP = strdup("HELLO WORLD");
    MU_LOG_VERBOSE(logger, "Declared string: \"%s\"\n", string);
    char *result TEMP = String_Utils_to_lowercase(&string, SU_NONE);
    MU_LOG_VERBOSE(logger, "Lowercase of declared string: \"%s\"\n", result);
    TEST_EQUAL(result, "hello world", test);
    String_Utils_to_lowercase(&string, SU_MODIFY);
    MU_LOG_VERBOSE(logger, "Modified declared string: \"%s\"\n", string);
    TEST_EQUAL(string, result, test);
    PASSED(test);
    return 1;
}

int  testString_Utils_to_uppercase() {
    char test[] = "To_Uppercase";
    MU_LOG_VERBOSE(logger, "\nTesting: %s!\n", test);
    char *string TEMP = strdup("hello world");
    MU_LOG_VERBOSE(logger, "Declared string: \"%s\"\n", string);
    char *result TEMP = String_Utils_to_uppercase(&string, SU_NONE);
    MU_LOG_VERBOSE(logger, "Lowercase of declared string: \"%s\"\n", result);
    TEST_EQUAL(result, "HELLO WORLD", test);
    String_Utils_to_uppercase(&string, SU_MODIFY);
    MU_LOG_VERBOSE(logger, "Modified declared string: \"%s\"\n", string);
    TEST_EQUAL(string, result, test);
    PASSED(test);
    return 1;
}

int testString_Utils_trim() {
    char test[] = "Trim";
    MU_LOG_VERBOSE(logger, "\nTesting: %s!\n", test);
    char *string TEMP = strdup("        asdadadasd      ");
    MU_LOG_VERBOSE(logger, "Declared string: \"%s\"\n", string);
    char *result TEMP = String_Utils_trim(&string, SU_NONE);
    MU_LOG_VERBOSE(logger, "Trimmed declared string: \"%s\"\n", result);
    TEST_EQUAL(result, "asdadadasd", test);
    String_Utils_trim(&string, SU_MODIFY);
    MU_LOG_VERBOSE(logger, "Modified declared string: \"%s\"\n", string);
    TEST_EQUAL(string, result, test);
    PASSED(test);
    return 1;
}

int main(void){
    logger = malloc(sizeof(MU_Logger_t));
    MU_Logger_Init(logger, "String_Utils_Test_Log.txt", "w", MU_ALL);
    Timer_t *timer = Timer_Init(1);
    int result = 0;
    const int total_tests = 20;
    TEST_ALL(result);
    MU_LOG_INFO(logger, "All tests finished! Passed %d/%d\n", result, total_tests);
    Timer_Stop(timer);
    char *total_time = Timer_To_String(timer);
    MU_LOG_INFO(logger, "Total Time: \"%s\"\n", total_time);
    Timer_Destroy(timer);
    free(total_time);
    MU_Logger_Destroy(logger, 1);
    return 0;
}