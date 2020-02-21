/*
 * Copyright 2020 Google Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdarg.h>

#include "strbuf.h"
#include "subprocess.h"

char const *strbuf_c_str(struct strbuf const *s)
{
	return s->cnt ? s->el : "";
}

static void maybe_init(struct strbuf *s) { if (!s->cnt) GROW_ARRAY_BY(*s, 1); }

void strbuf_append_str(struct strbuf *s, char const *src)
{
	size_t srclen = strlen(src);

	maybe_init(s);

	GROW_ARRAY_BY(*s, srclen);
	strcpy(s->el + s->cnt - 1 - srclen, src);
}

void strbuf_append_ch(struct strbuf *s, char ch)
{
	maybe_init(s);

	GROW_ARRAY_BY(*s, 1);
	s->el[s->cnt - 2] = ch;
}

void strbuf_trim_end(struct strbuf *s, size_t cnt)
{
	if (cnt > strbuf_len(s))
		DIE(0, "strbuf is too short: %zu > [%s]", cnt, strbuf_c_str(s));

	s->cnt -= cnt;
	memset(s->el + s->cnt - 1, 0, cnt);
}

void strbuf_run_for_stdout(struct strbuf *s, char const *cmd, int *exit_code)
{
	FILE *stream = xpopen(cmd, "r");
	int b;
	while ((b = fgetc(stream)) != EOF)
		strbuf_append_ch(s, b);
	if (ferror(stream))
		DIE(0, "error while reading output of command: %s", cmd);
	xpclose(&stream, exit_code);
}

int strbuf_next_token(struct strbuf const *s, char delim, ssize_t *pos)
{
	if (!s->cnt)
		return 0;
	if (*pos == -1) {
		*pos = 0;
		return 1;
	}
	while (s->el[*pos]) {
		if (delim == s->el[(*pos)++])
			break;
	}
	return !!s->el[*pos];
}

void strbuf_appendf(struct strbuf *s, char const *fmt, ...)
{
	int size;
	va_list argp;

	maybe_init(s);

	va_start(argp, fmt);
	size = vsnprintf(0, 0, fmt, argp);
	va_end(argp);

	GROW_ARRAY_BY(*s, size);

	va_start(argp, fmt);
	vsnprintf(s->el + s->cnt - size - 1, size + 1, fmt, argp);
	va_end(argp);
}
