## Target to built

TARGET 		=libvp6d

## TOOLS
CC      	= ecc
LD      	= ecc
AR      	= ar
OBJDUMP 	= objdump
RM      	= rm -f

## Directories
TOPDIR  		=C:\DuckSoft
PRIVATEINCLUDE  =${TOPDIR}\private\include
PRIVATEINCLUDE2 =${TOPDIR}\private\include\vp60
CORELIBSINCLUDE =${TOPDIR}\private\corelibs\include
CDXVINCLUDE     =${TOPDIR}\private\corelibs\cdxv\include 
VP6INCLUDE      =${TOPDIR}\private\corelibs\cdxv\vp60\vp60\include
CXGENERIC       =${TOPDIR}\private\corelibs\cdxv\vp60\vp60\cx\generic
OBJDIR          =${TOPDIR}\ObjectCode\bspvp6e
CURRENTDIR 		=${TOPDIR}\private\corelibs\cdxv\vp60\vp60 
LIBDIR			=${TOPDIR}\private\corelibs\lib\mapca 

## Compile Flags
ALLINCLUDES     =-I${VP6INCLUDE} -I${CDXVINCLUDE} -I${CORELIBSINCLUDE} -I${PRIVATEINCLUDE} -I${PRIVATEINCLUDE2} 
VP6DEFINES		=-DPREDICT_2D -DVFW_COMP -DCOMPDLL -DPOSTPROCESS -DCPUISLITTLEENDIAN -DNORMALIZED
ETIDEFINES      =-DMAPCA
ALLDEFINES      =${VP6DEFINES} ${ETIDEFINES}
DEBUG			=-O2
CFLAGS 			=-msvc -align 8 -etswp -mP3OPT_nonlocal_calls_through_register=true \
				-mP2OPT_suppress_library_call_conv_warnings=TRUE -maalign_branch_target \
				-magen_interroutine_padding
ALLFLAGS 		= $(CFLAGS) ${ALLDEFINES} ${ALLINCLUDES} ${DEBUG}


## Files
OBJS		= 	bsp\boolhuff.o				\
				generic\decodembs.o			\
				generic\decodemode.o		\
				generic\decodemv.o			\
				generic\DFrameR.o			\
				generic\FrameIni.o			\
				generic\Huffman.o			\
				generic\pb_globals.o		\
				generic\quantize.o			\
				generic\recon.o				\
				generic\TokenEntropy.o		\
				bsp\bspQuantize.o			\
				bsp\DSystemDependant.o		\
				bsp\duck_mem.o			\
				generic\vfwpbdll_if.o   

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


