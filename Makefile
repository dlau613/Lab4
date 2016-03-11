CC ?= gcc	
CFLAGS = -g -Wall -Wextra -Wno-unused  

LAB = 4
DISTDIR = lab4-$(USER)

TESTS = $(wildcard test*.sh)
TEST_BASES = $(subst .sh,,$(TESTS))

ADDTEST_SOURCES = lab4.c
ADDTEST_OBJECTS = $(subst .c,.o,$(ADDTEST_SOURCES))

SLTEST_SOURCES = sltest2.c
SLTEST_OBJECTS = $(subst .c,.o,$(SLTEST_SOURCES))

DIST_SOURCES = \
	$(ADDTEST_SOURCES) \
	Makefile \
	$(TESTS) \
	README.md

all: addtest sltest

addtest: $(ADDTEST_OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(ADDTEST_OBJECTS) -lpthread -lrt

sltest: $(SLTEST_OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(SLTEST_OBJECTS) -lpthread -lrt

dist: $(DISTDIR).tar.gz

$(DISTDIR).tar.gz: $(DIST_SOURCES) 
	rm -fr $(DISTDIR)
	tar -czf $@.tmp --transform='s,^,$(DISTDIR)/,' $(DIST_SOURCES)
	mv $@.tmp $@

check: $(TEST_BASES)

$(TEST_BASES): addtest
	./$@.sh

clean:
	rm -fr *.o *~ *.bak *.tar.gz core *.core *.tmp addtest sltest $(DISTDIR)

.PHONY: all dist check $(TEST_BASES) clean