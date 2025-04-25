SRCS = $(wildcard *.c)
ELFS = $(SRCS:.c=.elf)

all: $(ELFS)

%.elf: %.c
	$(CC) $< -pthread -rdynamic -g -o $@

clean:
	rm *.elf
