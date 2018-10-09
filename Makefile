src = $(wildcard *.c)
obj = $(src:.c=.o)

LDFLAGS =

CFLAGS = -std=c99

chat_app:	$(obj)
	$(CC) -o $@ $^ $(LDFLAGS) $(CFLAGS)

.PHONY: clean
clean:
	rm -f $(obj) chat_app