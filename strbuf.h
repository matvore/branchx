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

#ifndef STRBUF_H
#define STRBUF_H

#include "third_party/nusort/util.h"

struct strbuf {
	char *el;
	size_t cnt;
	size_t alloc;
};

char const *strbuf_c_str(struct strbuf const *s);
void strbuf_append_str(struct strbuf *s, char const *src);
void strbuf_append_ch(struct strbuf *s, char ch);
void strbuf_trim_end(struct strbuf *s, size_t cnt);
static inline size_t strbuf_len(struct strbuf const *s)
{
	return s->cnt ? s->cnt - 1 : 0;
}

void strbuf_run_for_stdout(struct strbuf *s, char const *cmd, int *exit_code);

/*
 * Move *pos to the next token. If already at the last token, returns 0.
 * To get the first token, set *pos to -1 before calling.
 */
int strbuf_next_token(struct strbuf const *s, char delim, ssize_t *pos);

__attribute__((format (printf, 2, 3)))
void strbuf_appendf(struct strbuf *s, char const *fmt, ...);

#endif
