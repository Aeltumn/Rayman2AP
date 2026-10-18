#pragma once

/** Compares to integers in an array. */
static int cmpInt(const void* a, const void* b) {
	int x = *(const int*)a;
	int y = *(const int*)b;
	return (x > y) - (x < y);
}

char* removeSubstring(const char* string, char const* substring);
void replaceSubstring(char* string, char from, char to);
int compareStringCaseInsensitive(char const* a, char const* b);
int compareStringCaseInsensitiveLimited(char const* a, char const* b);
long SPTXT_fn_lGetFmtStringLength(char const* szFmt, va_list args);