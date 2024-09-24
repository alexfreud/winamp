# ex: set ts=8 noet:

MODULE_NAME := nx

OS := $(shell sh -c 'uname -s 2>/dev/null || echo not')

ifeq ($(OS),Linux)
DIR := linux
DIRARCH := linux
else ifeq ($(OS),Darwin)
DIR := osx
DIRARCH := osx-amd64
else
$(error OS unrecognized ($OS))
endif

CSOURCES := $(DIR)/nxstring.c $(DIR)/nxuri.c $(DIR)/nxdata.c $(DIR)/nxsemaphore.c $(DIRARCH)/nxonce.c $(DIR)/nxpath.c $(DIR)/nxcondition.c $(DIR)/nxthread.c $(DIR)/nxfile.c $(DIR)/nxsleep.c
CPPSOURCES := $(DIR)/NXFileObject.cpp $(DIR)/NXFileProgressiveDownloader.cpp

LIBRARY_FILENAME := lib$(MODULE_NAME).so
OUTPUT_PATH := ../build/$(MODULE_NAME)
ARCHIVE_PATH := ../build/lib
LIBRARY_FILEPATH := ../build/lib/$(LIBRARY_FILENAME)

CPPOBJS := $(patsubst %.cpp,$(OUTPUT_PATH)/%.o,$(CPPSOURCES))
CPPDEPS := $(patsubst %.o,$(OUTPUT_PATH)/%.d,$(CPPOBJS))
COBJS := $(patsubst %.c,$(OUTPUT_PATH)/%.o,$(CSOURCES))
CDEPS := $(patsubst %.o,$(OUTPUT_PATH)/%.d,$(COBJS))

OBJS := $(CPPOBJS) $(COBJS)
DEPS := $(CPPDEPS) $(CDEPS)

CFLAGS= -I.. -DREPLICANT_NO_ICY -DREPLICANT_NO_ULTRAVOX -DREPLICANT_NO_HTTP -D_LARGEFILE64_SOURCE -fPIC #-fvisibility=hidden
CPPFLAGS := ${CFLAGS}
CFLAGS += -std=gnu99 # C but not CPP, sneaky

LDFLAGS =-lc -lpthread -lz #-fvisibility=hidden

ifeq ($(OS),Linux)
LDFLAGS += -lrt
endif

ifeq ($(OS),Darwin)
CFLAGS += -F /System/Library/Frameworks/CoreServices.framework/Frameworks
LDFLAGS += -framework CoreFoundation -framework CoreServices
endif

build: build-dir $(LIBRARY_FILEPATH)

build-dir:
	@mkdir -p $(OUTPUT_PATH)/$(DIR) > /dev/null 2> /dev/null
	@mkdir -p $(OUTPUT_PATH)/$(DIRARCH) > /dev/null 2> /dev/null
	@mkdir -p $(OUTPUT_PATH) > /dev/null 2> /dev/null
	@mkdir -p $(ARCHIVE_PATH) > /dev/null 2> /dev/null

dep:
	@rm ${DEPS}

$(OUTPUT_PATH)/%.o: %.cpp
	#@echo Compiling $*.cpp
	@$(CXX) $(CPPFLAGS) -MMD -MF $(OUTPUT_PATH)/$*.d -MT $(OUTPUT_PATH)/$*.o -c $*.cpp -o $(OUTPUT_PATH)/$*.o

$(OUTPUT_PATH)/$(DIR)/%.o: %.cpp
	#@echo Compiling $*.cpp
	@$(CXX) $(CPPFLAGS) -MMD -MF $(OUTPUT_PATH)/$(DIR)/$*.d -MT $(OUTPUT_PATH)/$(DIR)/$*.o -c $*.cpp -o $(OUTPUT_PATH)/$(DIR)/$*.o

$(OUTPUT_PATH)/%.o: %.c
	#@echo Compiling $*.c
	@$(CC) $(CFLAGS) -MMD -MF $(OUTPUT_PATH)/$*.d -MT $(OUTPUT_PATH)/$*.o -c $*.c -o $(OUTPUT_PATH)/$*.o

$(OUTPUT_PATH)/.*/%.o: %.c
	#@echo Compiling $*.c
	@$(CC) $(CFLAGS) -MMD -MF $(OUTPUT_PATH)/$*.d -MT $(OUTPUT_PATH)/$*.o -c $*.c -o $(OUTPUT_PATH)/$(DIR)/$*.o

ifeq ($(OS),Darwin)
LIBLDFLAGS = -Wl,-install_name,libnx.so
else
LIBLDFLAGS = -Wl,-soname,libnx.so
endif

$(LIBRARY_FILEPATH): ${OBJS}
	$(MAKE) -C ../nu
	$(MAKE) -C ../jnetlib
	@$(CXX) -Wl,-L,'../build/lib' -shared $(LIBLDFLAGS) -o $(LIBRARY_FILEPATH) ${OBJS} ${LDFLAGS} -lnu -ljnet

clean:
	-rm -f ${OBJS} $(LIBRARY_FILENAME) ${DEPS}

-include $(DEPS)
