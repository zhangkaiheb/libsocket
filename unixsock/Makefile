

CC = gcc
AR = ar
RM = rm -f


INCLUDES = -I./ -Iinclude
#LIBINCLUDES = -L.
CFLAGS = -O2 -Werror -Wall $(INCLUDES)
LINKFLAGS =

C_SOURCES = $(wildcard *.c)
C_OBJS = $(patsubst %.c, %.c, $(C_SOURCES))

OBJS_1 = dgram_client.o
OBJS_2 = dgram_server.o
OBJS_3 = stream_client.o
OBJS_4 = stream_server.o

LIBS = 

TARGET_1 = dclient
TARGET_2 = dserver
TARGET_3 = sclient
TARGET_4 = sserver


all:$(TARGET_1) $(TARGET_2) $(TARGET_3) $(TARGET_4)

$(TARGET_1):$(OBJS_1)
	$(CC) $(CFLAGS) $(LINKFLAGS) -o $(TARGET_1) $^ $(LIBS)

$(TARGET_2):$(OBJS_2)
	$(CC) $(CFLAGS) $(LINKFLAGS) -o $(TARGET_2) $^ $(LIBS)

$(TARGET_3):$(OBJS_3)
	$(CC) $(CFLAGS) $(LINKFLAGS) -o $(TARGET_3) $^ $(LIBS)

$(TARGET_4):$(OBJS_4)
	$(CC) $(CFLAGS) $(LINKFLAGS) -o $(TARGET_4) $^ $(LIBS)


%.o:%.c
	$(CC) -c $(CFLAGS) $< -o $@

.c.o:
	$(CC) -c -o $*.o $(CFLAGS) $(INCLUDES) $*.c

clean:
	-$(RM) *.o $(TARGET_1) $(TARGET_2) $(TARGET_3) $(TARGET_4)


