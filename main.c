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

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "strbuf.h"
#include "subprocess.h"
#include "third_party/nusort/util.h"

static void trim_suffix(struct strbuf *s, char const *suffix)
{
	size_t suf_len = strlen(suffix);
	char const *s_buf = strbuf_c_str(s);
	if (strbuf_len(s) < suf_len ||
	    strcmp(suffix, s_buf + strbuf_len(s) - suf_len))
		DIE(0, "does not end with '%s': %s", suffix, s_buf);
	strbuf_trim_end(s, suf_len);
}

/* TODO: be agnostic to hash algorithm */
#define SHA1_STR_LEN 40
#define SHA1_FMT "%.40s"

static void set_ref(FILE *update_ref, char const *name, char const *hash)
{
	char update_spec[128];

	int size = snprintf(update_spec, sizeof(update_spec),
			    "update h/%s%c" SHA1_FMT "%c%c",
			    name, 0, hash, 0, 0);
	if (size >= sizeof(update_spec))
		DIE(0, "update spec is too long: %d (%s...)",
		    size, update_spec);

	if (!fwrite(update_spec, size, 1, update_ref))
		DIE(0, "Failed to send update spec to 'git update-ref': %s",
		    update_spec);
}

static void run_log(
	struct strbuf *o,
	struct strbuf const *branch_args,
	FILE *update_ref)
{
	struct strbuf log_cmd = {0};
	struct strbuf merge_base_cmd = {0};
	struct strbuf merge_base = {0};
	size_t merge_base_len;
	char const *fmt_flag = "'--format=format:%H  %ce%C(auto)%d\n%s\n '";

	strbuf_appendf(
		&merge_base_cmd,
		"git show-branch --merge-base %s",
		strbuf_c_str(branch_args));

	strbuf_run_for_stdout(&merge_base, strbuf_c_str(&merge_base_cmd), NULL);
	merge_base_len = strbuf_len(&merge_base);
	if (merge_base_len != SHA1_STR_LEN + 1)
		DIE(0, "%ld", merge_base_len);
	if (merge_base.el[merge_base_len - 1] != '\n')
		DIE(0, "merge_base invalid: '%s'", strbuf_c_str(&merge_base));
	strbuf_trim_end(&merge_base, 1);

	set_ref(update_ref, "branchx_base", strbuf_c_str(&merge_base));

	strbuf_appendf(
		&log_cmd,
		"git log --graph --color=always %s %s ^%s^@ --",
		fmt_flag, strbuf_c_str(branch_args), strbuf_c_str(&merge_base));

	strbuf_run_for_stdout(o, strbuf_c_str(&log_cmd), NULL);

	DESTROY_ARRAY(log_cmd);
	DESTROY_ARRAY(merge_base);
	DESTROY_ARRAY(merge_base_cmd);
}

static int is_hash(char const *s)
{
	size_t p;
	for (p = 0; p < SHA1_STR_LEN; p++) {
		if (s[p] >= '0' && s[p] <= '9')
			continue;
		if (s[p] >= 'a' && s[p] <= 'f')
			continue;
		return 0;
	}
	return 1;
}

static void process_log_output(struct strbuf const *o, FILE *update_ref)
{
	size_t output_pos = 0;
	size_t hash_count = 1;

	while (o->el[output_pos]) {
		struct strbuf line_buf = {0};
		struct strbuf line_prefix = {0};
		struct strbuf sym_refs = {0};

		while (o->el[output_pos]) {
			if (o->el[output_pos] == '\n') {
				output_pos++;
				break;
			} else if (strbuf_len(&line_prefix) ||
				   !is_hash(&o->el[output_pos])) {
				strbuf_append_ch(&line_buf, o->el[output_pos]);
				output_pos++;
			} else {
				struct strbuf tag_name = {0};
				char const *hash = o->el + output_pos;

				strbuf_appendf(&tag_name, "%ld", hash_count);
				set_ref(update_ref, strbuf_c_str(&tag_name),
					hash);

				DESTROY_ARRAY(tag_name);

				strbuf_appendf(&line_buf,
					       "\e[48;5;213m\e[30m %ld \e[m",
					       hash_count);
				hash_count++;

				strbuf_appendf(&line_prefix,
					       "\e[38;5;147m%.8s\e[m", hash);
				output_pos += SHA1_STR_LEN;
			}
		}

		fprintf(stdout, "%8s  %s%s\n",
			strbuf_c_str(&line_prefix),
			strbuf_c_str(&line_buf),
			strbuf_c_str(&sym_refs));

		DESTROY_ARRAY(line_buf);
		DESTROY_ARRAY(line_prefix);
		DESTROY_ARRAY(sym_refs);
	}
}

static void append_upstream(char const *ref, struct strbuf *dest)
{
	struct strbuf cmd = {0};
	int exit;

	/* Use rev-parse to figure out the upstream branch. */
	strbuf_appendf(
		&cmd, "git rev-parse -q --abbrev-ref '%s@{upstream}'", ref);

	/*
	 * If there is no upstream for the given branch, rev-parse shows an
	 * error message on stderr, so we redirect stderr to /dev/null.
	 */
	strbuf_append_str(&cmd, " 2>/dev/null");

	strbuf_run_for_stdout(dest, strbuf_c_str(&cmd), &exit);
	if (!exit) {
		trim_suffix(dest, "\n");
		strbuf_append_ch(dest, ' ');
	}

	DESTROY_ARRAY(cmd);
}

static void populate_default_branches(struct strbuf *branch_args)
{
	unsigned i;
	unsigned last_start = 0;
	struct strbuf upstream_refs = {0};

	strbuf_run_for_stdout(
		branch_args,
		"git for-each-ref --format='%(refname:short)' "
		"refs/heads/*",
		NULL);
	for (i = 0; i < strbuf_len(branch_args); i++) {
		if (branch_args->el[i] != '\n')
			continue;

		/*
		 * Append the upstream of ref to dest with a trailing
		 * space, or do nothing if there is no upstream.
		 */
		branch_args->el[i] = 0;
		append_upstream(strbuf_c_str(branch_args) + last_start,
				&upstream_refs);
		last_start = i + 1;
		branch_args->el[i] = ' ';
	}

	strbuf_append_str(branch_args, strbuf_c_str(&upstream_refs));
	DESTROY_ARRAY(upstream_refs);
}

static int has_conflicting_ref_name_with_number(int n)
{
	struct strbuf cmd = {0};
	struct strbuf out = {0};
	int exit_ignored;
	int res;

	strbuf_appendf(&cmd, "git rev-parse --verify h/%d 2>/dev/null", n);
	strbuf_run_for_stdout(&out, strbuf_c_str(&cmd), &exit_ignored);
	res = strbuf_len(&out) != 0;

	DESTROY_ARRAY(out);
	DESTROY_ARRAY(cmd);

	return res;
}

#define REF_CONFLICT_CFG "branchx.skip-ref-conflict-check"

#define SET_REF_CONFLICT_CFG "git config --add " REF_CONFLICT_CFG " true"

#define REF_CONFLICT_ERROR (\
"I usually create refs named h/1, h/2, ..., but this repo already has ref(s)\n"\
"named like that. Give them different names and re-run me. Alternatively, to\n"\
"just let me overwrite them, run:\n"\
"\t" SET_REF_CONFLICT_CFG "\n"\
)

static int has_conflicting_ref_name(void)
{
	/*
	 * Use `git rev-parse` to figure out if there is already any ref named
	 * h/1...h/100. If so, the user may have refs overwritten unexpectedly.
	 * If the tool has already run in the past, of course the user will have
	 * refs named h/##, so we first check if branchx.skip_ref_conflict_check
	 * is set. If it has been set, we skip the ref conflict check.
	 */
	struct strbuf out = {0};
	int exit_ignored;
	int res = 0;
	int i;

	strbuf_run_for_stdout(
		&out, "git config --get --bool " REF_CONFLICT_CFG,
		&exit_ignored);
	if (!strcmp(strbuf_c_str(&out), "true\n"))
		goto cleanup;

	for (i = 1; i <= 100 && !res; i++)
		res = has_conflicting_ref_name_with_number(i);

	if (res) {
		fputs(REF_CONFLICT_ERROR, stderr);
	} else {
		int set_cfg = system(SET_REF_CONFLICT_CFG);
		if (set_cfg)
			DIE(0, "system("SET_REF_CONFLICT_CFG") returned: %d",
			    set_cfg);
	}

cleanup:
	DESTROY_ARRAY(out);

	return res;
}

int main(int argc, char **argv)
{
	struct strbuf branch_args = {0};
	struct strbuf log_output = {0};
	FILE *update_ref;

	if (has_conflicting_ref_name())
		return 52;

	while (--argc) {
		strbuf_append_str(&branch_args, *++argv);
		strbuf_append_ch(&branch_args, ' ');
	}

	if (!branch_args.cnt) {
		populate_default_branches(&branch_args);
		strbuf_append_str(&branch_args, "HEAD ");
	}

	update_ref = xpopen("git update-ref --stdin -z", "w");
	run_log(&log_output, &branch_args, update_ref);
	process_log_output(&log_output, update_ref);
	xpclose(&update_ref, NULL);

	DESTROY_ARRAY(branch_args);
	DESTROY_ARRAY(log_output);

	return 0;
}
