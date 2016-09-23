CC = gcc
XX = g++
CFLAGS = -std=c99 -Wall -w -O -g
TARGET = datalink
SOURCES=datalink.c event.c Impl.c Init.c log.c network.c physical.c timer.c
OBJS = $(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCES)))

%.o: %.c

	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.cpp

	$(XX) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	$(XX) $(OBJS) -o $(TARGET)

	chmod a+x $(TARGET)

clean:

	rm -rf *.o datalink