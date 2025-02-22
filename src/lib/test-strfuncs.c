/* Copyright (c) 2009-2018 Dovecot authors, see the included COPYING file */

#include "test-lib.h"
#include "array.h"

static void test_p_strdup(void)
{
	test_begin("p_strdup()");
	test_assert(p_strdup(default_pool, NULL) == NULL);

	const char *src = "foo";
	char *str = p_strdup(default_pool, src);
	test_assert(str != src && str != NULL && strcmp(src, str) == 0);
	p_free(default_pool, str);

	test_end();
}

static void test_p_strndup(void)
{
	struct {
		const char *input;
		const char *output;
		size_t len;
	} tests[] = {
		{ "foo", "fo", 2 },
		{ "foo", "foo", 3 },
		{ "foo", "foo", 4 },
		{ "foo\0more", "foo", 8 },
	};
	test_begin("p_strndup()");

	for (unsigned int i = 0; i < N_ELEMENTS(tests); i++) {
		char *str = p_strndup(default_pool, tests[i].input,
				      tests[i].len);
		test_assert_strcmp_idx(str, tests[i].output, i);
		p_free(default_pool, str);
	}
	test_end();
}

static void test_p_strdup_empty(void)
{
	test_begin("p_strdup_empty()");
	test_assert(p_strdup_empty(default_pool, NULL) == NULL);
	test_assert(p_strdup_empty(default_pool, "") == NULL);

	const char *src = "foo";
	char *str = p_strdup_empty(default_pool, src);
	test_assert(str != src && str != NULL && strcmp(src, str) == 0);
	p_free(default_pool, str);

	test_end();
}

static void test_p_strdup_until(void)
{
	const char src[] = "foo\0bar";
	char *str;

	test_begin("p_strdup_until()");
	str = p_strdup_until(default_pool, src, src+2);
	test_assert(strcmp(str, "fo") == 0);
	p_free(default_pool, str);

	str = p_strdup_until(default_pool, src, src+3);
	test_assert(strcmp(str, "foo") == 0);
	p_free(default_pool, str);

	/* \0 is ignored */
	str = p_strdup_until(default_pool, src, src+7);
	test_assert(memcmp(str, src, sizeof(src)) == 0);
	p_free(default_pool, str);

	str = p_strdup_until(default_pool, src, src+8);
	test_assert(memcmp(str, src, sizeof(src)) == 0);
	p_free(default_pool, str);

	test_end();
}

static void test_p_strarray_dup(void)
{
	const char *input[][3] = {
		{ NULL },
		{ "a", NULL },
		{ "foobar", NULL },
		{ "a", "foo", NULL }
	};
	const char **ret;
	unsigned int i, j;

	test_begin("p_strarray_dup");

	for (i = 0; i < N_ELEMENTS(input); i++) {
		ret = p_strarray_dup(default_pool, input[i]);
		for (j = 0; input[i][j] != NULL; j++) {
			test_assert(strcmp(input[i][j], ret[j]) == 0);
			test_assert(input[i][j] != ret[j]);
		}
		test_assert(ret[j] == NULL);
		i_free(ret);
	}
	test_end();
}

static void test_t_strsplit(void)
{
	struct {
		const char *input;
		const char *const *output;
	} tests[] = {
		/* empty string -> empty array. was this perhaps a mistake for
		   the API to do this originally?.. can't really change now
		   anyway. */
		{ "", (const char *const []) { NULL } },
		{ "\n", (const char *const []) { "", "", NULL } },
		{ "\n\n", (const char *const []) { "", "", "", NULL } },
		{ "foo", (const char *const []) { "foo", NULL } },
		{ "foo\n", (const char *const []) { "foo", "", NULL } },
		{ "foo\nbar", (const char *const []) { "foo", "bar", NULL } },
		{ "foo\nbar\n", (const char *const []) { "foo", "bar", "", NULL } },
		{ "\nfoo\n\nbar\n\n", (const char *const []) { "", "foo", "", "bar", "", "", NULL } },
	};
	const char *const *args, *const *args2, *const *args3;

	test_begin("t_strsplit");

	for (unsigned int i = 0; i < N_ELEMENTS(tests); i++) {
		/* split_str_fast() with single separator */
		args = t_strsplit(tests[i].input, "\n");
		/* split_str_slow() with a secondary separator */
		args2 = t_strsplit(tests[i].input, "\r\n");
		/* also as suffix */
		args3 = t_strsplit(tests[i].input, "\n\r");
		for (unsigned int j = 0; tests[i].output[j] != NULL; j++) {
			test_assert_idx(null_strcmp(tests[i].output[j], args[j]) == 0, i);
			test_assert_idx(null_strcmp(args[j], args2[j]) == 0, i);
			test_assert_idx(null_strcmp(args[j], args3[j]) == 0, i);
		}
	}
	test_end();
}

static void test_t_strsplit_spaces(void)
{
	struct {
		const char *input;
		const char *const *output;
	} tests[] = {
		/* empty strings */
		{ "", (const char *const []) { NULL } },
		{ "\n", (const char *const []) { NULL } },
		{ "\n\n", (const char *const []) { NULL } },
		/* normal */
		{ "foo", (const char *const []) { "foo", NULL } },
		{ "foo\n", (const char *const []) { "foo", NULL } },
		{ "foo\nbar", (const char *const []) { "foo", "bar", NULL } },
		{ "foo\nbar\n", (const char *const []) { "foo", "bar", NULL } },
		{ "\nfoo\n\nbar\n\n", (const char *const []) { "foo", "bar", NULL } },
	};
	const char *const *args, *const *args2, *const *args3;

	test_begin("t_strsplit_spaces");

	for (unsigned int i = 0; i < N_ELEMENTS(tests); i++) {
		args = t_strsplit_spaces(tests[i].input, "\n");
		/* test also with a secondary nonexistent separator */
		args2 = t_strsplit_spaces(tests[i].input, "\r\n");
		/* also as suffix */
		args3 = t_strsplit_spaces(tests[i].input, "\n\r");
		for (unsigned int j = 0; tests[i].output[j] != NULL; j++) {
			test_assert_idx(null_strcmp(tests[i].output[j], args[j]) == 0, i);
			test_assert_idx(null_strcmp(args[j], args2[j]) == 0, i);
			test_assert_idx(null_strcmp(args[j], args3[j]) == 0, i);
		}
	}

	/* multiple separators */
	args = t_strsplit_spaces(" , ,   ,str1  ,  ,,, , str2   , ", " ,");
	test_assert(strcmp(args[0], "str1") == 0);
	test_assert(strcmp(args[1], "str2") == 0);
	test_assert(args[2] == NULL);
	test_end();
}

static void test_t_str_replace(void)
{
	test_begin("t_str_replace");
	test_assert(strcmp(t_str_replace("foo", 'a', 'b'), "foo") == 0);
	test_assert(strcmp(t_str_replace("fooa", 'a', 'b'), "foob") == 0);
	test_assert(strcmp(t_str_replace("afooa", 'a', 'b'), "bfoob") == 0);
	test_assert(strcmp(t_str_replace("", 'a', 'b'), "") == 0);
	test_assert(strcmp(t_str_replace("a", 'a', 'b'), "b") == 0);
	test_assert(strcmp(t_str_replace("aaa", 'a', 'b'), "bbb") == 0);
	test_assert(strcmp(t_str_replace("bbb", 'a', 'b'), "bbb") == 0);
	test_assert(strcmp(t_str_replace("aba", 'a', 'b'), "bbb") == 0);
	test_end();
}

static void test_t_str_oneline(void)
{
	test_begin("t_str_oneline");
	test_assert(strcmp(t_str_oneline("\n"), "") == 0);
	test_assert(strcmp(t_str_oneline("\r"), "") == 0);
	test_assert(strcmp(t_str_oneline("\n\n"), "") == 0);
	test_assert(strcmp(t_str_oneline("\r\r"), "") == 0);
	test_assert(strcmp(t_str_oneline("\r\n"), "") == 0);
	test_assert(strcmp(t_str_oneline("\r\n\r\n"), "") == 0);
	test_assert(strcmp(t_str_oneline("\n\r"), "") == 0);
	test_assert(strcmp(t_str_oneline("\n\r\n\r"), "") == 0);
	test_assert(strcmp(t_str_oneline("foo"), "foo") == 0);
	test_assert(strcmp(t_str_oneline("\nfoo"), "foo") == 0);
	test_assert(strcmp(t_str_oneline("foo\n"), "foo") == 0);
	test_assert(strcmp(t_str_oneline("\nfoo\n"), "foo") == 0);
	test_assert(strcmp(t_str_oneline("foo\nbar"), "foo bar") == 0);
	test_assert(strcmp(t_str_oneline("foo\n\nbar"), "foo bar") == 0);
	test_assert(strcmp(t_str_oneline("\nfoo\nbar"), "foo bar") == 0);
	test_assert(strcmp(t_str_oneline("foo\nbar\n"), "foo bar") == 0);
	test_assert(strcmp(t_str_oneline("foo\nbar\nbaz"), "foo bar baz") == 0);
	test_assert(strcmp(t_str_oneline("\rfoo"), "foo") == 0);
	test_assert(strcmp(t_str_oneline("foo\r"), "foo") == 0);
	test_assert(strcmp(t_str_oneline("\rfoo\r"), "foo") == 0);
	test_assert(strcmp(t_str_oneline("foo\rbar"), "foobar") == 0);
	test_assert(strcmp(t_str_oneline("foo\r\rbar"), "foobar") == 0);
	test_assert(strcmp(t_str_oneline("\rfoo\rbar"), "foobar") == 0);
	test_assert(strcmp(t_str_oneline("foo\rbar\r"), "foobar") == 0);
	test_assert(strcmp(t_str_oneline("foo\rbar\rbaz"), "foobarbaz") == 0);
	test_assert(strcmp(t_str_oneline("\r\nfoo\r\n"), "foo") == 0);
	test_assert(strcmp(t_str_oneline("foo\r\n"), "foo") == 0);
	test_assert(strcmp(t_str_oneline("\r\nfoo"), "foo") == 0);
	test_assert(strcmp(t_str_oneline("foo\r\nbar"), "foo bar") == 0);
	test_assert(strcmp(t_str_oneline("foo\r\n\r\nbar"), "foo bar") == 0);
	test_assert(strcmp(t_str_oneline("\r\nfoo\r\nbar"), "foo bar") == 0);
	test_assert(strcmp(t_str_oneline("foo\r\nbar\r\n"), "foo bar") == 0);
	test_assert(strcmp(t_str_oneline("foo\r\nbar\r\nbaz"), "foo bar baz") == 0);
	test_end();
}

static void test_t_str_trim(void)
{
	test_begin("t_str_trim");
	test_assert(strcmp(t_str_trim("", " "), "") == 0);
	test_assert(strcmp(t_str_trim(" ", " "), "") == 0);
	test_assert(strcmp(t_str_trim(" \t ", "\t "), "") == 0);
	test_assert(strcmp(t_str_trim("f \t ", "\t "), "f") == 0);
	test_assert(strcmp(t_str_trim("foo", ""), "foo") == 0);
	test_assert(strcmp(t_str_trim("foo", " "), "foo") == 0);
	test_assert(strcmp(t_str_trim("foo ", " "), "foo") == 0);
	test_assert(strcmp(t_str_trim(" foo", " "), "foo") == 0);
	test_assert(strcmp(t_str_trim(" foo ", " "), "foo") == 0);
	test_assert(strcmp(t_str_trim("\tfoo ", "\t "), "foo") == 0);
	test_assert(strcmp(t_str_trim(" \tfoo\t ", "\t "), "foo") == 0);
	test_assert(strcmp(t_str_trim("\r \tfoo\t \r", "\t \r"), "foo") == 0);
	test_assert(strcmp(t_str_trim("\r \tfoo foo\t \r", "\t \r"), "foo foo") == 0);
	test_assert(strcmp(t_str_trim("\tfoo\tfoo\t", "\t \r"), "foo\tfoo") == 0);
	test_end();
}

static void test_t_str_ltrim(void)
{
	test_begin("t_str_ltrim");
	test_assert(strcmp(t_str_ltrim("", " "), "") == 0);
	test_assert(strcmp(t_str_ltrim(" ", " "), "") == 0);
	test_assert(strcmp(t_str_ltrim(" \t ", "\t "), "") == 0);
	test_assert(strcmp(t_str_ltrim(" \t f", "\t "), "f") == 0);
	test_assert(strcmp(t_str_ltrim("foo", ""), "foo") == 0);
	test_assert(strcmp(t_str_ltrim("foo", " "), "foo") == 0);
	test_assert(strcmp(t_str_ltrim("foo ", " "), "foo ") == 0);
	test_assert(strcmp(t_str_ltrim(" foo", " "), "foo") == 0);
	test_assert(strcmp(t_str_ltrim(" foo ", " "), "foo ") == 0);
	test_assert(strcmp(t_str_ltrim("\tfoo ", "\t "), "foo ") == 0);
	test_assert(strcmp(t_str_ltrim(" \tfoo\t ", "\t "), "foo\t ") == 0);
	test_assert(strcmp(t_str_ltrim("\r \tfoo\t \r", "\t \r"), "foo\t \r") == 0);
	test_assert(strcmp(t_str_ltrim("\r \tfoo foo\t \r", "\t \r"), "foo foo\t \r") == 0);
	test_assert(strcmp(t_str_ltrim("\tfoo\tfoo\t", "\t \r"), "foo\tfoo\t") == 0);
	test_end();
}

static void test_t_str_rtrim(void)
{
	test_begin("t_str_rtrim");
	test_assert(strcmp(t_str_rtrim("", " "), "") == 0);
	test_assert(strcmp(t_str_rtrim(" ", " "), "") == 0);
	test_assert(strcmp(t_str_rtrim(" \t ", "\t "), "") == 0);
	test_assert(strcmp(t_str_rtrim("f \t ", "\t "), "f") == 0);
	test_assert(strcmp(t_str_rtrim("foo", ""), "foo") == 0);
	test_assert(strcmp(t_str_rtrim("foo", " "), "foo") == 0);
	test_assert(strcmp(t_str_rtrim("foo ", " "), "foo") == 0);
	test_assert(strcmp(t_str_rtrim(" foo", " "), " foo") == 0);
	test_assert(strcmp(t_str_rtrim(" foo ", " "), " foo") == 0);
	test_assert(strcmp(t_str_rtrim("\tfoo ", "\t "), "\tfoo") == 0);
	test_assert(strcmp(t_str_rtrim(" \tfoo\t ", "\t "), " \tfoo") == 0);
	test_assert(strcmp(t_str_rtrim("\r \tfoo\t \r", "\t \r"), "\r \tfoo") == 0);
	test_assert(strcmp(t_str_rtrim("\r \tfoo foo\t \r", "\t \r"), "\r \tfoo foo") == 0);
	test_assert(strcmp(t_str_rtrim("\tfoo\tfoo\t", "\t \r"), "\tfoo\tfoo") == 0);
	test_end();
}

static const char *const test_strarray_input[] = {
	"", "hello", "world", "", "yay", "", NULL
};
static const struct {
	const char *separator;
	const char *output;
} test_strarray_outputs[] = {
	{ "", "helloworldyay" },
	{ " ", " hello world  yay " },
	{ "!-?", "!-?hello!-?world!-?!-?yay!-?" }
};

static const char *const test_strarray_input2[] = {
	"", "", "hello", "world", "", "yay", "", NULL
};
static struct {
	const char *separator;
	const char *output;
} test_strarray_outputs2[] = {
	{ "", "helloworldyay" },
	{ " ", "  hello world  yay " },
	{ "!-?", "!-?!-?hello!-?world!-?!-?yay!-?" }
};

static const char *const test_strarray_input3[] = {
	"hello", "", "", "yay", NULL
};
static struct {
	const char *separator;
	const char *output;
} test_strarray_outputs3[] = {
	{ "", "helloyay" },
	{ " ", "hello   yay" },
	{ "!-?", "hello!-?!-?!-?yay" }
};

static void test_t_strarray_join(void)
{
	const char *null = NULL;
	unsigned int i;

	test_begin("t_strarray_join()");

	/* empty array -> empty string */
	test_assert(strcmp(t_strarray_join(&null, " "), "") == 0);

	for (i = 0; i < N_ELEMENTS(test_strarray_outputs); i++) {
		test_assert_idx(strcmp(t_strarray_join(test_strarray_input,
						       test_strarray_outputs[i].separator),
				       test_strarray_outputs[i].output) == 0, i);
	}
	for (i = 0; i < N_ELEMENTS(test_strarray_outputs2); i++) {
		test_assert_idx(strcmp(t_strarray_join(test_strarray_input2,
						       test_strarray_outputs2[i].separator),
				       test_strarray_outputs2[i].output) == 0, i);
	}
	for (i = 0; i < N_ELEMENTS(test_strarray_outputs3); i++) {
		test_assert_idx(strcmp(t_strarray_join(test_strarray_input3,
						       test_strarray_outputs3[i].separator),
				       test_strarray_outputs3[i].output) == 0, i);
	}
	test_end();
}

static void test_p_array_const_string_join(void)
{
	ARRAY_TYPE(const_string) arr;
	unsigned int i;
	char *res;

	test_begin("p_array_const_string_join()");

	i_array_init(&arr, 2);
	/* empty array -> empty string */
	test_assert(strcmp(t_array_const_string_join(&arr, " "), "") == 0);

	array_append(&arr, test_strarray_input,
		     str_array_length(test_strarray_input));
	for (i = 0; i < N_ELEMENTS(test_strarray_outputs); i++) {
		res = p_array_const_string_join(default_pool, &arr,
						test_strarray_outputs[i].separator);
		test_assert_idx(strcmp(res, test_strarray_outputs[i].output) == 0, i);
		i_free(res);
	}

	array_free(&arr);
	test_end();
}

static void test_mem_equals_timing_safe(void)
{
	const struct {
		const char *a, *b;
	} tests[] = {
		{ "", "" },
		{ "a", "a" },
		{ "b", "a" },
		{ "ab", "ab" },
		{ "ab", "ba" },
		{ "ab", "bc" },
	};
	test_begin("mem_equals_timing_safe()");
	for (unsigned int i = 0; i < N_ELEMENTS(tests); i++) {
		size_t len = strlen(tests[i].a);
		i_assert(len == strlen(tests[i].b));
		test_assert((memcmp(tests[i].a, tests[i].b, len) == 0) ==
			    mem_equals_timing_safe(tests[i].a, tests[i].b, len));
		test_assert((memcmp(tests[i].a, tests[i].b, len) == 0) ==
			    mem_equals_timing_safe(tests[i].b, tests[i].a, len));
	}
	test_end();
}

static void test_str_equals_timing_almost_safe(void)
{
	const struct {
		const char *a, *b;
	} tests[] = {
		{ "", "" },
		{ "a", "a" },
		{ "b", "a" },
		{ "ab", "ab" },
		{ "ab", "ba" },
		{ "ab", "bc" },
		{ "a", "" },
		{ "a", "ab" },
		{ "a", "abc" },
		{ "ab", "abc" },
	};
	test_begin("str_equals_timing_almost_safe()");
	for (unsigned int i = 0; i < N_ELEMENTS(tests); i++) {
		test_assert((strcmp(tests[i].a, tests[i].b) == 0) ==
			    str_equals_timing_almost_safe(tests[i].a, tests[i].b));
		test_assert((strcmp(tests[i].a, tests[i].b) == 0) ==
			    str_equals_timing_almost_safe(tests[i].b, tests[i].a));
	}
	test_end();
}

static void test_dec2str_buf(void)
{
	const uintmax_t test_input[] = {
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
		99, 999, 9999, 65535, 65536, 99999, 999999, 9999999,
		99999999, 999999999, 4294967295, 4294967296ULL,
		9999999999999999999ULL,
		18446744073709551615ULL
	};
	char buf[MAX_INT_STRLEN], buf2[MAX_INT_STRLEN];

	test_begin("dec2str_buf()");
	for (unsigned int i = 0; i < N_ELEMENTS(test_input); i++) {
		i_snprintf(buf2, sizeof(buf2), "%ju", test_input[i]);
		test_assert_idx(strcmp(dec2str_buf(buf, test_input[i]),
				       buf2) == 0, i);
	}
	test_end();
}

static void
test_str_match(void)
{
	static const struct {
		const char*s1, *s2; size_t match;
	} tests[] = {
		{ "", "a", 0 },
		{ "a", "a", 1 },
		{ "a", "A", 0 },
		{ "ab", "a", 1 },
		{ "ab", "A", 0 },
		{ "B", "AB", 0 },
		{ "ab", "AB", 0 },
#define MATCH_TEST(common, left, right) { common left, common right, sizeof(common)-1 }
		MATCH_TEST("", "", ""),
		MATCH_TEST("", "x", ""),
		MATCH_TEST("", "", "x"),
		MATCH_TEST("", "foo", "bar"),
		MATCH_TEST("x", "", ""),
		MATCH_TEST("x", "y", "z"),
		MATCH_TEST("blahblahblah", "", ""),
		MATCH_TEST("blahblahblah", "", "bar"),
		MATCH_TEST("blahblahblah", "foo", ""),
		MATCH_TEST("blahblahblah", "foo", "bar"),
#undef MATCH_TEST
	};
	const char *suffix;
	unsigned int i;

	test_begin("str_match");
	for (i = 0; i < N_ELEMENTS(tests); i++)
		test_assert_idx(str_match(tests[i].s1, tests[i].s2) == tests[i].match, i);
	test_end();

	test_begin("str_begins");
	for (i = 0; i < N_ELEMENTS(tests); i++) {
		/* This is just 2 ways of wording the same test, but that also
		   sanity tests the match values above. */
		bool equals = strncmp(tests[i].s1, tests[i].s2, strlen(tests[i].s2)) == 0;
		test_assert_idx(str_begins_with(tests[i].s1, tests[i].s2) == equals, i);
		test_assert_idx(str_begins(tests[i].s1, tests[i].s2, &suffix) == equals &&
				(!equals || suffix == tests[i].s1 + strlen(tests[i].s2)), i);
		test_assert_idx(str_begins(tests[i].s1, tests[i].s2, &suffix) ==
				(strlen(tests[i].s2) == tests[i].match), i);
	}
	/* test literal-optimized versions of these */
	test_assert(str_begins("", "", &suffix) && suffix[0] == '\0');
	test_assert(str_begins("123", "", &suffix) && strcmp(suffix, "123") == 0);
	test_assert(str_begins("123", "1", &suffix) && strcmp(suffix, "23") == 0);
	test_assert(str_begins("123", "123", &suffix) && suffix[0] == '\0');
	suffix = NULL;
	test_assert(!str_begins("123", "1234", &suffix) && suffix == NULL);
	test_assert(!str_begins("", "123", &suffix) && suffix == NULL);
	test_assert(!str_begins("12", "123", &suffix) && suffix == NULL);

	test_assert(str_begins_with("", ""));
	test_assert(str_begins_with("123", ""));
	test_assert(str_begins_with("123", "1"));
	test_assert(str_begins_with("123", "123"));
	test_assert(!str_begins_with("123", "1234"));
	test_assert(!str_begins_with("", "123"));
	test_assert(!str_begins_with("12", "123"));
	test_end();
}

static void
test_str_match_icase(void)
{
	const struct {
		const char *s1, *s2;
		size_t match;
	} tests[] = {
		{ "", "a", 0 },
		{ "a", "a", 1 },
		{ "a", "A", 1 },
		{ "ab", "a", 1 },
		{ "ab", "A", 1 },
		{ "B", "AB", 0 },
		{ "ab", "AB", 2 },
	};
	const char *suffix;
	unsigned int i;

	test_begin("str_match_icase");
	for (i = 0; i < N_ELEMENTS(tests); i++)
		test_assert_idx(str_match_icase(tests[i].s1, tests[i].s2) == tests[i].match, i);
	test_end();

	test_begin("str_begins_icase");
	for (i = 0; i < N_ELEMENTS(tests); i++) {
		/* This is just 2 ways of wording the same test, but that also
		   sanity tests the match values above. */
		bool equals = strncasecmp(tests[i].s1, tests[i].s2, strlen(tests[i].s2)) == 0;
		test_assert_idx(str_begins_icase_with(tests[i].s1, tests[i].s2) == equals, i);
		test_assert_idx(str_begins_icase(tests[i].s1, tests[i].s2, &suffix) == equals &&
				(!equals || suffix == tests[i].s1 + strlen(tests[i].s2)), i);
		test_assert_idx(str_begins_icase(tests[i].s1, tests[i].s2, &suffix) ==
				(strlen(tests[i].s2) == tests[i].match), i);
	}
	/* test literal-optimized versions of these */
	test_assert(str_begins_icase("", "", &suffix) && suffix[0] == '\0');
	test_assert(str_begins_icase("aBc", "", &suffix) && strcmp(suffix, "aBc") == 0);
	test_assert(str_begins_icase("aBc", "a", &suffix) && strcmp(suffix, "Bc") == 0);
	test_assert(str_begins_icase("aBc", "A", &suffix) && strcmp(suffix, "Bc") == 0);
	test_assert(str_begins_icase("aBc", "AbC", &suffix) && suffix[0] == '\0');
	suffix = NULL;
	test_assert(!str_begins_icase("aBc", "AbCd", &suffix) && suffix == NULL);
	test_assert(!str_begins_icase("", "aBc", &suffix) && suffix == NULL);
	test_assert(!str_begins_icase("aB", "AbC", &suffix) && suffix == NULL);

	test_assert(str_begins_icase_with("", ""));
	test_assert(str_begins_icase_with("aBc", ""));
	test_assert(str_begins_icase_with("aBc", "A"));
	test_assert(str_begins_icase_with("aBc", "AbC"));
	test_assert(!str_begins_icase_with("aBc", "aBcD"));
	test_assert(!str_begins_icase_with("", "aBc"));
	test_assert(!str_begins_icase_with("aB", "AbC"));
	test_end();
}

static void test_memspn(void)
{
#undef TEST_CASE
/* we substract 1 to ensure we don't include the final \0 byte */
#define TEST_CASE(a, b, r) { \
	.input = (const unsigned char*)((a)), .input_len = sizeof((a))-1, \
	.accept = (const unsigned char*)((b)), .accept_len = sizeof((b))-1, \
	.result = r, \
}

	static struct {
		const unsigned char *input;
		size_t input_len;
		const unsigned char *accept;
		size_t accept_len;
		size_t result;
	} tests[] = {
		TEST_CASE("", "", 0),
		TEST_CASE("", "123456789", 0),
		TEST_CASE("123456789", "", 0),
		TEST_CASE("hello, world", "helo", 5),
		TEST_CASE("hello, uuuuu", "helo", 5),
		TEST_CASE("\0\0\0\0\0hello", "\0", 5),
		TEST_CASE("\r\r\r\r", "\r", 4),
		TEST_CASE("aaa", "a", 3),
		TEST_CASE("bbb", "a", 0),
		/* null safety test */
		{
			.input = NULL, .accept = NULL,
			.input_len = 0, .accept_len = 0,
			.result = 0,
		}
	};

	test_begin("i_memspn");

	for (unsigned int i = 0; i < N_ELEMENTS(tests); i++) {
		size_t a = i_memspn(tests[i].input, tests[i].input_len,
				    tests[i].accept, tests[i].accept_len);
		test_assert_ucmp_idx(a, ==, tests[i].result, i);
		if (tests[i].input == NULL)
			continue;
		a = i_memspn(tests[i].input, strlen((const char*)tests[i].input),
			     tests[i].accept, strlen((const char*)tests[i].accept));
		size_t b = strspn((const char*)tests[i].input,
				  (const char*)tests[i].accept);
		test_assert_ucmp_idx(a, ==, b, i);
	}

	test_end();
}

static void test_memcspn(void)
{
#undef TEST_CASE
/* we substract 1 to ensure we don't include the final \0 byte */
#define TEST_CASE(a, b, r) { \
	.input = (const unsigned char*)((a)), .input_len = sizeof((a))-1, \
	.reject = (const unsigned char*)((b)), .reject_len = sizeof((b))-1, \
	.result = r, \
}

	static struct {
		const unsigned char *input;
		size_t input_len;
		const unsigned char *reject;
		size_t reject_len;
		size_t result;
	} tests[] = {
		TEST_CASE("", "", 0),
		TEST_CASE("hello", "", 5),
		TEST_CASE("uuuuu, hello", "helo", 7),
		TEST_CASE("\0\0\0\0\0\0hello", "u", 11),
		TEST_CASE("this\0is\0test", "\0", 4),
		TEST_CASE("hello, world\r", "\r", 12),
		TEST_CASE("aaa", "a", 0),
		TEST_CASE("bbb", "a", 3),
		/* null safety test */
		{
			.input = NULL, .reject = NULL,
			.input_len = 0, .reject_len = 0,
			.result = 0,
		}
	};

	test_begin("i_memcspn");

	for (unsigned int i = 0; i < N_ELEMENTS(tests); i++) {
		size_t a = i_memcspn(tests[i].input, tests[i].input_len,
				     tests[i].reject, tests[i].reject_len);
		test_assert_ucmp_idx(a, ==, tests[i].result, i);
		if (tests[i].input == NULL)
			continue;
		a = i_memcspn(tests[i].input, strlen((const char*)tests[i].input),
			      tests[i].reject, strlen((const char*)tests[i].reject));
		size_t b = strcspn((const char*)tests[i].input,
				   (const char*)tests[i].reject);
		test_assert_ucmp_idx(a, ==, b, i);
	}

	test_end();
}

void test_strfuncs(void)
{
	test_p_strdup();
	test_p_strndup();
	test_p_strdup_empty();
	test_p_strdup_until();
	test_p_strarray_dup();
	test_t_strsplit();
	test_t_strsplit_spaces();
	test_t_str_replace();
	test_t_str_oneline();
	test_t_str_trim();
	test_t_str_ltrim();
	test_t_str_rtrim();
	test_t_strarray_join();
	test_p_array_const_string_join();
	test_mem_equals_timing_safe();
	test_str_equals_timing_almost_safe();
	test_dec2str_buf();
	test_str_match();
	test_str_match_icase();
	test_memspn();
	test_memcspn();
}

enum fatal_test_state fatal_strfuncs(unsigned int stage)
{
	switch (stage) {
	case 0:
		test_begin("fatal p_strndup()");
		test_expect_fatal_string("(str != NULL)");
		(void)p_strndup(default_pool, NULL, 100);
		return FATAL_TEST_FAILURE;
	case 1:
		test_expect_fatal_string("(max_chars != SIZE_MAX)");
		(void)p_strndup(default_pool, "foo", SIZE_MAX);
		return FATAL_TEST_FAILURE;
	}
	test_end();
	return FATAL_TEST_FINISHED;
}
