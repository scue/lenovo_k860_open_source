.SUFFIXES : .c .o

CC		= gcc
ASM		=
LINK		=
LIBCC		= ar
RM		= rm

#----------------------------------------------------------------------------------------------
INCPATH		=
LIBPATH		= 
OBJPATH		= .
OUTPATH		= .
SOURCEPATH	= .

LDFLAGS		= -L$(LIBPATH) 
#LFLAGS		= -lm
ASFLAGS		=
ARFLAGS		= -ruv
CFLAGS		= -o 

LIBS =

#---------------------------------------------------------------------------------------
SOURCES = $(OBJECTS:.o=.c)

all:
	$(CC) $(CFLAGS)	sd_fdisk sd_fdisk.c
	$(CC) $(CFLAGS)	mk_bl1ah  mk_bl1ah.c
	$(CC) $(CFLAGS)	mk_bl2_security  mk_bl2_security.c
	$(CC) $(CFLAGS)	mkpad  mkpad.c
	$(CC) $(CFLAGS)	reread_partition_table  reread_partition_table.c

#---------------------------------------------------------------------------------------
.c.o:
		$(CC) $(CFLAGS) -c $< -o $@

dep:
		gccmakedep	$(SOURCES)

#---------------------------------------------------------------------------------------
clean:
		$(RM)		-rf sd_fdisk
		$(RM)		-rf mk_bl1ah
		$(RM)		-rf bl1ah
		$(RM)		-rf mk_bl2_security
		$(RM)		-rf mkpad
		$(RM)		-rf reread_partition_table

new:
		$(MAKE)		clean
		$(MAKE)

