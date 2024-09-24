## Target to built

TARGET 		=libvppp

## TOOLS
CC      	= ecc
LD      	= ecc
AR      	= ar
OBJDUMP 	= objdump
RM      	= rm -f

## Directories
TOPDIR  	=C:\DuckSoft
PRIVATEINCLUDE  =${TOPDIR}\private\include
CORELIBSINCLUDE =${TOPDIR}\private\corelibs\include
CDXVINCLUDE     =${TOPDIR}\private\corelibs\cdxv\include 
VPPPINCLUDE     =${TOPDIR}\private\corelibs\cdxv\vppp\include 

CURRENTDIR 	=${TOPDIR}\private\corelibs\cdxv\vppp
LIBDIR		=${TOPDIR}\private\corelibs\lib\mapca 

## Compile Flags
ALLINCLUDES     =-I${CDXVINCLUDE} -I${CORELIBSINCLUDE} -I${PRIVATEINCLUDE} -I${VPPPINCLUDE}
VP6DEFINES		=-DPREDICT_2D -DVFW_COMP -DCOMPDLL -DPOSTPROCESS -DCPUISLITTLEENDIAN -DNORMALIZED
ETIDEFINES      =-DMAPCA
ALLDEFINES      =${VP6DEFINES} ${ETIDEFINES}

DEBUG			=-O2
CFLAGS 			=-msvc -align 8 -etswp -mP3OPT_nonlocal_calls_through_register=true \
				-mP2OPT_suppress_library_call_conv_warnings=TRUE -maalign_branch_target \
				-magen_interroutine_padding
ALLFLAGS 		=$(CFLAGS) ${ALLDEFINES} ${ALLINCLUDES} ${DEBUG}


## Files
OBJS		= 	bsp\borders.o	 \
			generic\clamp.o	 \
			generic\deblock.o	 \
			generic\DeInterlace.o	 \
			generic\Dering.o	 \
			generic\loopfilter.o	 \
			generic\postproc.o	 \
			generic\scale.o	 \
			generic\simpledeblocker.o \
			generic\doptsystemdependant.o
			

SRCS		= $(OBJS:.o=.c)

ARTARGET	= ${TARGET}.a

# archive

ARTARGET:${OBJS}
	${AR} -cr ${ARTARGET} ${OBJS}
	mv ${ARTARGET} ${LIBDIR}

${OBJS} : ${SRCS}
	$(CC) $(ALLFLAGS) -c $*.c -o $*.o

clean:
	${RM} ${OBJS} ${ARTARGET}


