src = $(wildcard *.c)
obj = $(src:.c=.o)

LDFLAGS =

chat_app:	$(obj)
	$(CC) -o $@ $^ $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(obj) chat_app