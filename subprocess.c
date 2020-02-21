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

#include "third_party/nusort/util.h"
#include "subprocess.h"

FILE *xpopen(char const *cmd, char const *mode)
{
	FILE *stream = popen(cmd, mode);
	if (!stream)
		DIE(1, "popen");
	return stream;
}

void xpclose(FILE **stream, int *exit_code)
{
	int actual_exit_code = pclose(*stream);

	if (actual_exit_code == -1)
		DIE(1, "pclose");

	if (exit_code)
		*exit_code = actual_exit_code;
	else if (actual_exit_code)
		DIE(0, "Child process failed with exit code: %d",
		    actual_exit_code);

	*stream = 0;
}
