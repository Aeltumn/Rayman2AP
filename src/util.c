#pragma once

#include <stdio.h>
#include <string.h>

/** Removes a substring from a larger string. */
char* removeSubstring(const char* string, char const* substring) {
	char* _substr = strstr(string, substring);
	while (_substr != NULL && strcmp(substring, "") != 0) {
		sprintf(_substr, "%s%s", "", _substr + strlen(substring));
		_substr = strstr(string, substring);
	}
	return _substr;
}

/** Replaces a char with another char. */
void replaceSubstring(char* string, char from, char to) {
	int length = strlen(string);
	for (int i = 0; i < length; i++) {
		if (string[i] == from) {
			string[i] = to;
		}
	}
}

// https://stackoverflow.com/questions/5820810/case-insensitive-string-comparison-in-c
int compareStringCaseInsensitive(char const* a, char const* b) {
	return _stricmp(a, b);
}

int compareStringCaseInsensitiveLimited(char const* a, char const* b) {
	return _strnicmp(a, b, strlen(b));
}

// Copied from https://github.com/raytools/ACP_Ray2/blob/master/src/Ray2x/SPTXT/SPTXT.c
long SPTXT_fn_lGetFmtStringLength(char const* szFmt, va_list args) {
	long lSize = vsnprintf(NULL, 0, szFmt, args);
	return lSize + 1;
}