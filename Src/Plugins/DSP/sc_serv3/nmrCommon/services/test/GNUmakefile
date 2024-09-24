default: release

CXX=gcc

CXXFLAGS_DEBUG= -g -D_REENTRANT -D_INC_PROCESS -Wall -Wno-unused-function -Wno-sign-compare -Werror

CXXFLAGS_RELEASE= -D_REENTRANT -D_INC_PROCESS -DNDEBUG -Wall -Wno-unused-function -Wno-sign-compare -Werror

INCLUDES= \
	-I. \
	-I.. \
	-I../..
	
HEADER_FILES= \
	$(wildcard *.h) \
	$(wildcard ../*.h) \

SOURCE_FILES= \
	$(wildcard *.cpp) \
	$(wildcard ../*.cpp) \
	$(wildcard ../../threading/thread.cpp) \
	$(wildcard ../../file/fileUtils.cpp)	

OBJECT_FILES= \
	$(addsuffix .o,$(basename $(notdir $(SOURCE_FILES))))

DEBUG_OBJECTS= \
	$(addprefix debug/,$(OBJECT_FILES))

RELEASE_OBJECTS= \
	$(addprefix release/,$(OBJECT_FILES))

debug/%.o:	../../threading/%.cpp $(HEADER_FILES)
	$(CXX) $(CXXFLAGS_DEBUG) $(INCLUDES) -c $< -o $@

release/%.o:	../../threading/%.cpp $(HEADER_FILES)
	$(CXX) $(CXXFLAGS_RELEASE) $(INCLUDES) -c $< -o $@

debug/%.o:	../../file/%.cpp $(HEADER_FILES)
	$(CXX) $(CXXFLAGS_DEBUG) $(INCLUDES) -c $< -o $@

release/%.o:	../../file/%.cpp $(HEADER_FILES)
	$(CXX) $(CXXFLAGS_RELEASE) $(INCLUDES) -c $< -o $@

debug/%.o:	../%.cpp $(HEADER_FILES)
	$(CXX) $(CXXFLAGS_DEBUG) $(INCLUDES) -c $< -o $@

release/%.o:	../%.cpp $(HEADER_FILES)
	$(CXX) $(CXXFLAGS_RELEASE) $(INCLUDES) -c $< -o $@

debug/%.o:	%.cpp $(HEADER_FILES)
	$(CXX) $(CXXFLAGS_DEBUG) $(INCLUDES) -c $< -o $@

release/%.o: %.cpp $(HEADER_FILES)
	$(CXX) $(CXXFLAGS_RELEASE) $(INCLUDES) -c $< -o $@

releasedir:
	-mkdir -p release

debugdir:
	-mkdir -p debug

release/test:	$(RELEASE_OBJECTS)
	$(CXX) $(CXXFLAGS_RELEASE) $(RELEASE_OBJECTS) -lrt -lsocket -lnsl -lpthread -lstdc++ -lz -lm

debug/test:	$(DEBUG_OBJECTS)
	$(CXX) $(CXXFLAGS_DEBUG) $(DEBUG_OBJECTS) -lrt -lsocket -lnsl -lpthread -lstdc++ -lz -lm

release: releasedir $(RELEASE_OBJECTS) release/test

debug: debugdir $(DEBUG_OBJECTS) debug/test

clean:
	rm -rf release
	rm -rf debug

all: release debug

