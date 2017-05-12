CC=gcc
#CFLAGS=-Wall -Wpedantic -O2
CFLAGS=-Wall -Wpedantic -g -O0
IFLAGS=-Isrc
LFLAGS=-lm

COMPILE=$(CC) $(CFLAGS) $(IFLAGS) $(LFLAGS)

SOURCES:=$(shell find src -path "src/heads" -prune -o -name "*.[ch]" -print)
FRAGMENTS:=$(shell find src -path "src/heads" -prune -o -name "*.[ch]f" -print)
ALL_SOURCES:=$(SOURCES) $(FRAGMENTS)
HEADS:=$(shell find src/heads -name "*.c" -print)
DEBUG_ALL:=-DDEBUG_SELECT -DDEBUG_COHORT

OBJS:=$(shell find src -path "src/heads" -prune -o -name "*.c" -print | sed "s/\.c/.o/g" | sed "s/^src/obj/")

DBG_OBJS:=$(shell find src -path "src/heads" -prune -o -name "*.c" -print | sed "s/\.c/.o.d/g" | sed "s/^src/obj/")

.PHONY: list
list:
	@echo $(ALL_SOURCES)
	@echo $(HEADS)
	@echo $(OBJS)

obj/%.o: src/%.c
	mkdir -p $(@D)
	$(COMPILE) -c $< -o $@

obj/%.o.d: src/%.c
	mkdir -p $(@D)
	$(COMPILE) $(DEBUG_ALL) -c $< -o $@

bin/test: $(ALL_SOURCES) $(OBJS) src/heads/test.c
	$(COMPILE) $(OBJS) src/heads/test.c -o $@

bin/test_debug: $(ALL_SOURCES) $(DBG_OBJS) src/heads/test.c
	$(COMPILE) $(DEBUG_ALL) $(DBG_OBJS) src/heads/test.c -o $@

bin/rng: $(ALL_SOURCES) $(OBJS) src/heads/rng.c
	$(COMPILE) $(OBJS) src/heads/rng.c -o $@

test/%.gv: bin/test
	./bin/test

test/%.svg: test/%.gv
	dot -Tsvg $< > $@

SVGS:=$(shell find test -name "*.gv" | sed -e "s/\.gv$$/.svg/")

.PHONY: graphviz
graphviz: $(SVGS)

.PHONY: test
test: bin/test
	./bin/test

.PHONY: test_quiet
test_quiet: bin/test
	./bin/test | grep "\.\.\."

.PHONY: test_debug
test_debug: bin/test_debug
	./bin/test_debug

.PHONY: rng
rng: bin/rng
	./bin/rng 1000

.PHONY: clean
clean:
	rm -R obj/*
	rm bin/*
