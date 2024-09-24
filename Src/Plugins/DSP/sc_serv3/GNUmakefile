SHELL = /bin/sh

.SUFFIXES:
.SUFFIXES: .cpp .o .d .h

OS := $(shell uname | sed 'y/abcdefghijklmnopqrstuvwxyz/ABCDEFGHIJKLMNOPQRSTUVWXYZ/')
x64 := $(shell uname -m)
ENDIAN := $(shell uname -m)
ENDIAN := $(patsubst i%86,LITTLE_ENDIAN,$(ENDIAN))
ENDIAN := $(patsubst x86_64,LITTLE_ENDIAN,$(ENDIAN))
ENDIAN := $(patsubst Power Macintosh,BIG_ENDIAN,$(ENDIAN))
CXX=g++ -std=c++11 -Wno-deprecated

##########################################################
########### Platform specific stuff #####################
##########################################################

ifeq ($(OS),LINUX)

	### Raspbian is reported as a Linux OS
	### and reports 'armv6l' via uname -m
	### so we check -m to detect it properly
	ifeq ($(x64),armv6l)

		#PLATFORM_DEFINES=-DPLATFORM_ARMv6
		# this is for GCC 4.6.x to make a legacy compatible compile ??
		PLATFORM_DEFINES=-DPLATFORM_ARMv6 -mcpu=arm1176jzf-s -mfpu=vfp -mfloat-abi=hard -ffunction-sections -fdata-sections -Wl,--gc-sections

		PLATFORM_LIBS=\
			libs/Aol_XML/ARMv6/libexpat.a\
			libs/libcurl/ARMv6/libcurl.a\
			libs/libcurl/ARMv6/libssl.a\
			libs/libcurl/ARMv6/libcrypto.a\
			libs/zlib/ARMv6/libz.a\
			-lrt\
			-lpthread\
			-ldl
	### the RPi2 is reported as a Linux OS
	### and reports 'armv7l' via uname -m
	### so we check -m to detect it properly
	else ifeq ($(x64),armv7l)
		# https://www.raspberrypi.org/forums/viewtopic.php?p=684549#p684549
		# this is for GCC 4.6.x
		#PLATFORM_DEFINES=-DPLATFORM_ARMv7 -march=armv7-a -mfpu=neon-vfpv4 -mfloat-abi=hard
		# this is for GCC 4.8.x (default is 4.6.x on Raspbian)
		PLATFORM_DEFINES=-DPLATFORM_ARMv7 -mcpu=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard -Wno-unused-result -Wno-ignored-qualifiers -Wno-long-long -Wno-missing-field-initializers -ffunction-sections -fdata-sections -Wl,--gc-sections
		# this is for GCC 4.6.x to make a legacy compatible compile ??
		#PLATFORM_DEFINES=-DPLATFORM_ARMv6 -mcpu=arm1176jzf-s -mfpu=vfp -mfloat-abi=hard

		PLATFORM_LIBS=\
			libs/Aol_XML/ARMv7/libexpat.a\
			libs/libcurl/ARMv7/libcurl.a\
			libs/libcurl/ARMv7/libssl.a\
			libs/libcurl/ARMv7/libcrypto.a\
			libs/zlib/ARMv7/libz.a\
			-lrt\
			-lpthread\
			-ldl
	else
		PLATFORM_DEFINES=-DPLATFORM_LINUX -Wno-unused-result -Wno-ignored-qualifiers -Wno-long-long -Wno-missing-field-initializers -ffunction-sections -fdata-sections -Wl,--gc-sections

		### Linux platform specific includes
		### with 32-bit and 64-bit handling
		ifeq ($(x64),x86_64)
			INCLUDES=-Ideps/x86_64/include
			PLATFORM_LIBS=\
				-Ldeps/x86_64/lib -lexpat -lcurl -lssl -lcrypto -lpthread -lz -lrt -ldl
		else
			CXX += -m32
			INCLUDES=-Ideps/i686/include
			PLATFORM_LIBS=\
				-Ldeps/i686/lib -lexpat -lcurl -lssl -lcrypto -lpthread -lz -lrt -ldl
		endif
	endif

###################################

else 
	ifeq ($(OS),FREEBSD)
#		CXX=g++44
		PLATFORM_DEFINES=-DPLATFORM_BSD -Wno-long-long -Wno-missing-field-initializers

		PLATFORM_LIBS=\
			libs/Aol_XML/BSD/libexpat.a\
			libs/libcurl/BSD/libcurl.a\
			libs/libcurl/BSD/libssl.a\
			libs/libcurl/BSD/libcrypto.a\
			libs/zlib/BSD/libz.a\
			-lpthread\
#			-R /usr/local/lib/gcc44

	else 
		ifeq ($(OS),DARWIN)
			PLATFORM_DEFINES=-DPLATFORM_MAC -Wno-long-long -Wno-missing-field-initializers

			PLATFORM_LIBS=\
				libs/Aol_XML/Darwin/libexpat.a\
				libs/libcurl/Darwin/libcurl.a\
				libs/libcurl/Darwin/libssl.a\
				libs/libcurl/Darwin/libcrypto.a\
				libs/zlib/Darwin/libz.a\
				-lpthread
		else
			ERR = $(error Unknown operating system $(OS))
		endif
	endif
endif

ifneq ($(ENDIAN),LITTLE_ENDIAN)
	ifneq ($(ENDIAN),BIG_ENDIAN)
		ERR = $(error Unknown endian $(ENDIAN))
	endif
endif

##########################################################
##########################################################
##########################################################
##########################################################
##########################################################

INCLUDES += -I. -InmrCommon
#INCLUDES=-I. -InmrCommon -Izlib -Ilibcurl/include -IGeoIP/libGeoIP

LIBS=$(PLATFORM_LIBS)

CPPFLAGS=$(PLATFORM_DEFINES) -MMD -MP -D_REENTRANT -DCURL_STATICLIB -Wno-unused-function -Wno-sign-compare -Wno-unknown-pragmas -Wall -Wextra -pedantic

CFLAGS_RELEASE=-O2 -static-libgcc -static-libstdc++ -rdynamic
CPPFLAGS_RELEASE=$(CPPFLAGS) -O2 -DNDEBUG -g

CFLAGS_DEBUG=-static-libgcc -static-libstdc++ -rdynamic
CPPFLAGS_DEBUG=$(CPPFLAGS) -ggdb -DDEBUG

VPATH=nmrCommon/threading nmrCommon/stacktrace nmrCommon/services nmrCommon/file nmrCommon/unicode webNet aolxml
#VPATH=nmrCommon/threading nmrCommon/stacktrace nmrCommon/services nmrCommon/file nmrCommon/unicode webNet aolxml GeoIP/libGeoIP

C_SOURCES=\
	$(wildcard *.c)

#C_SOURCES=\
#	$(wildcard *.c)\
#	$(wildcard GeoIP/libGeoIP/*.c)

CXX_SOURCES=\
	$(wildcard *.cpp)\
	$(wildcard nmrCommon/threading/thread.cpp)\
	$(wildcard nmrCommon/stacktrace/StackTrace.cpp)\
	$(wildcard nmrCommon/services/logger.cpp)\
	$(wildcard nmrCommon/services/serviceMain.cpp)\
	$(wildcard nmrCommon/services/stdServiceImpl.cpp)\
	$(wildcard nmrCommon/file/fileUtils.cpp)\
	$(wildcard nmrCommon/unicode/*.cpp)\
	$(wildcard webNet/*.cpp)\
	$(wildcard aolxml/*.cpp)

SOURCE_FILES=$(C_SOURCES) $(CXX_SOURCES)

OBJECT_FILES=$(addsuffix .o,$(basename $(notdir $(SOURCE_FILES))))
DEBUG_OBJECTS=$(addprefix debug/,$(OBJECT_FILES))
RELEASE_OBJECTS=$(addprefix release/,$(OBJECT_FILES))

.PHONY:	default
default: release

release/config.o debug/config.o: unixversion.h
release/serviceMain.o debug/serviceMain.o: unixversion.h

##############################################################
### Create unix version header from Win32 resource file
unixversion.h: sc_serv.rc
	@grep --text 'PRODUCTVERSION' sc_serv.rc | sed 's/PRODUCTVERSION/static int & \[VENT\]=\{/' | sed 's/[0-9]*,[0-9]*,[0-9]*,[0-9]*/&\};/' > unixversion_tmp.h
	@echo '#define VENT 4' | cat - unixversion_tmp.h > unixversion.h
	@rm -f unixversion_tmp.h

################# File rules #########################3

debug/%.o: %.cpp
	$(CXX) $(CPPFLAGS_DEBUG) $(INCLUDES) -c $< -o $@

debug/%.o: %.c
	$(CXX) $(CPPFLAGS_DEBUG) $(INCLUDES) -c $< -o $@

release/%.o: %.cpp
	$(CXX) $(CPPFLAGS_RELEASE) $(INCLUDES) -c $< -o $@

release/%.o: %.c
	$(CXX) $(CPPFLAGS_RELEASE) $(INCLUDES) -c $< -o $@

#release/%.o: GeoIP/libGeoIP/%.c
#	$(CXX) $(CPPFLAGS_RELEASE) $(INCLUDES) -c $< -o $@

############################################################

sc_serv_debug: $(DEBUG_OBJECTS)
	$(CXX) -o sc_serv_debug $(CFLAGS_DEBUG) $(DEBUG_OBJECTS) $(LIBS)
	@./sc_serv_debug -v
	@echo

sc_serv: sc_serv_notstripped
	strip -o sc_serv sc_serv_notstripped
	@echo

sc_serv_notstripped: $(RELEASE_OBJECTS)
	$(CXX) -o sc_serv_notstripped $(CFLAGS_RELEASE) $(RELEASE_OBJECTS) $(LIBS)
	@./sc_serv_notstripped -v

releasedir:
	@mkdir -p release

debugdir:
	@mkdir -p debug

.PHONY: clean
.PHONY: release
.PHONY: debug

release: releasedir sc_serv
release_nostrip: releasedir sc_serv_notstripped
debug: debugdir sc_serv_debug

clean:
	@rm -rf debug
	@rm -rf release
	@rm -f unixversion.h

.PHONY:	err
err: ; $(ERR)

##################################################3

ifeq ($(MAKECMDGOALS),release)
-include $(RELEASE_OBJECTS:.o=.d)
else
ifneq ($(MAKECMDGOALS),clean)
-include $(DEBUG_OBJECTS:.o=.d)
endif
endif
