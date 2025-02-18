GCC = g++ -Wall -Wextra

SRCS = testing.c apis.c
EXEC = a.out

all: $(EXEC)

$(EXEC): $(SRCS)
	$(GCC) $(CFLAGS) -o $@ $^

test: all
	./$(EXEC)

clean:
	rm -f $(EXEC)