# Copyright 2020 Google Inc. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

CFLAGS = \
	-Werror \
	-Wall \
	-Wdeclaration-after-statement \
	-Wformat-security \
	-Wmissing-prototypes \
	-Wno-format-zero-length \
	-Wold-style-definition \
	-Woverflow \
	-Wpointer-arith \
	-Wstrict-prototypes \
	-Wunused \
	-Wvla \
	-std=gnu89

OBJS = \
	main.o \
	third_party/nusort/util.o \
	subprocess.o \
	strbuf.o

HDRS = \
	third_party/nusort/util.h \
	subprocess.h \
	strbuf.h

%.o: %.c $(HDRS)
	$(CC) $(CFLAGS) -o $@ -c -iquote . $<

bin/branchx: $(OBJS)
	mkdir -p bin
	$(CC) -o $@ $^

clean:
	rm -f bin/branchx *.o third_party/nusort/*.o
