

CC = gcc
AR = ar
RM = rm -f


#DBG = -O2
DBG = -g
INCLUDES = -I./ -Iinclude
#LIBINCLUDES = -L.
CFLAGS += $(DBG) -Werror -Wall $(INCLUDES)
LINKFLAGS =

C_SOURCES = $(wildcard *.c)
C_OBJS = $(patsubst %.c, %.c, $(C_SOURCES))

OBJS_1 = tcp_client.o
OBJS_2 = tcp_server.o
LIBS = 

TARGET_1 = tcpc
TARGET_2 = tcps


all:$(TARGET_1) $(TARGET_2)

$(TARGET_1):$(OBJS_1)
	$(CC) $(CFLAGS) $(LINKFLAGS) -o $(TARGET_1) $^ $(LIBS)

$(TARGET_2):$(OBJS_2)
	$(CC) $(CFLAGS) $(LINKFLAGS) -o $(TARGET_2) $^ $(LIBS)


%.o:%.c
	$(CC) -c $(CFLAGS) $< -o $@

.c.o:
	$(CC) -c -o $*.o $(CFLAGS) $(INCLUDES) $*.c

clean:
	-$(RM) *.o $(TARGET_1) $(TARGET_2)

#gcc lsblk.c -I. -L. libsmartcols.a libmount.a libblkid.a libuuid.a


