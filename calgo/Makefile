EXEC = toto
CFLAGS = -Wall -W -lm -g
CFILES = $(wildcard src/*.c)
OFILES = $(CFILES:src/%.c=obj/%.o)
DFILES = $(CFILES:src/%.c=obj/%.d)

-include *.d

all: bin obj $(EXEC)


bin:
	mkdir -p bin

obj:
	mkdir -p obj


$(EXEC): $(OFILES)
	gcc -o $@ $^ $(CFLAGS)

obj/%.o: src/%.c
	gcc -o $@ -c $< $(CFLAGS) -MMD -MF $(@:.o=.d) -MT $@

.PHONY: clean mrproper all

clean:
	rm -f $(OFILES)
	rm -f $(DFILES)

mrproper: clean
	rm -f $(EXEC)
