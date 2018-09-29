

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

OBJS_1 = pkt_mpls_snd.o
LIBS = 

TARGET_1 = pkt_mpls_snd


all:$(TARGET_1)

$(TARGET_1):$(OBJS_1)
	$(CC) $(CFLAGS) $(LINKFLAGS) -o $(TARGET_1) $^ $(LIBS)


%.o:%.c
	$(CC) -c $(CFLAGS) $< -o $@

.c.o:
	$(CC) -c -o $*.o $(CFLAGS) $(INCLUDES) $*.c

clean:
	-$(RM) *.o $(TARGET_1)


