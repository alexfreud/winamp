## Target to built

TARGET 			=libvp6e

## TOOLS
CC      		= ecc
LD      		= ecc
AR      		= ar
OBJDUMP 		= objdump
RM      		= rm -f

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
ALLINCLUDES     =-I${CXGENERIC} -I${VP6INCLUDE} -I${CDXVINCLUDE} -I${CORELIBSINCLUDE} -I${PRIVATEINCLUDE} -I${PRIVATEINCLUDE2} 
VP6DEFINES		=-DPREDICT_2D -DVFW_COMP -DCOMPDLL -DPOSTPROCESS -DCPUISLITTLEENDIAN -DNORMALIZED
ETIDEFINES      =-DMAPCA
ALLDEFINES      =${VP6DEFINES} ${ETIDEFINES}
DEBUG			=-O2
CFLAGS 			=-msvc -align 8 -ms -etswp -mP3OPT_nonlocal_calls_through_register=true \
				-mP2OPT_suppress_library_call_conv_warnings=TRUE -maalign_branch_target \
				-magen_interroutine_padding
ALLFLAGS 		= $(CFLAGS) ${ALLDEFINES} ${ALLINCLUDES} ${DEBUG}


## Files
OBJS		= 	cx\bsp\PackVideo.o		 \
				cx\bsp\PickModes.o		 \
				cx\generic\RawBuffer.o	 \
				cx\bsp\bspTokenize.o	 \
				cx\generic\Transform.o	 \
				cx\bsp\encode.o		     \
				cx\bsp\encodembs.o		 \
				cx\bsp\encodemode.o		 \
				cx\generic\encodemv.o	 \
				cx\bsp\mcomp.o			 \
				cx\generic\misc_common.o \
				cx\generic\twopass.o	 \
				cx\bsp\vfwcomp.o		 \
				cx\generic\vfwcomp_if.o	 \
				cx\bsp\bspComp_Globals.o \
				cx\bsp\mcompopt.o		 \
				cx\bsp\bsptransform.o	 \
				cx\bsp\CSystemDependant.o 

SRCS			= $(OBJS:.o=.c)
 
ARTARGET		= ${TARGET}.a

# archive

ARTARGET:${OBJS}
			${AR} -cr ${ARTARGET} ${OBJS}
			mv ${ARTARGET} ${LIBDIR}

${OBJS} : ${SRCS}
			$(CC) $(ALLFLAGS) -c $*.c -o $*.o

clean:
			${RM} ${OBJS} ${ARTARGET}


