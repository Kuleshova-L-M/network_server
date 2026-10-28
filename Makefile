TARGET = vcalc_server
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra
LIBS = -lcryptopp -lpthread -lboost_program_options

SOURCES = main.cpp \
          ConfigHandler.cpp \
          Logger.cpp \
          ServerException.cpp \
          UserBase.cpp \
          Authenticator.cpp \
          DataProcessor.cpp \
          Server.cpp

OBJECTS = $(SOURCES:.cpp=.o)

all: release

release: CXXFLAGS += -O2 -DNDEBUG
release: $(TARGET)

debug: CXXFLAGS += -g -O0
debug: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) $(LIBS) -o $(TARGET)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET) vcalc.log

.PHONY: all release debug clean
