CC = gcc	
CFLAGS = -g -Wall -Wextra -Wno-unused  

LAB = 4
DISTDIR = lab4-$(USER)

all: addtest

TESTS = $(wildcard test*.sh)
TEST_BASES = $(subst .sh,,$(TESTS))

Addtest_SOURCES = \
	lab4.c

Addtest_OBJECTS = $(subst .c,.o,$(Addtest_SOURCES))

DIST_SOURCES = \
	$(Addtest_SOURCES) makefile \
	$(TESTS) README

addtest: $(Addtest_OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(Addtest_OBJECTS) -lpthread

dist: $(DISTDIR).tar.gz

$(DISTDIR).tar.gz: $(DIST_SOURCES) 
	rm -fr $(DISTDIR)
	tar -czf $@.tmp --transform='s,^,$(DISTDIR)/,' $(DIST_SOURCES)
	mv $@.tmp $@

check: $(TEST_BASES)

$(TEST_BASES): addtest
	./$@.sh

clean:
	rm -fr *.o *~ *.bak *.tar.gz core *.core *.tmp addtest $(DISTDIR)

.PHONY: all dist check $(TEST_BASES) clean