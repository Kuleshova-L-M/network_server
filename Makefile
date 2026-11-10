TARGET = vcalc_server
TEST_TARGET = unit_tests

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -I.
LIBS = -lcryptopp -lpthread -lboost_program_options
TEST_LIBS = $(LIBS) -lUnitTest++

SOURCES = main.cpp \
          ConfigHandler.cpp \
          Logger.cpp \
          ServerException.cpp \
          UserBase.cpp \
          Authenticator.cpp \
          DataProcessor.cpp \
          Server.cpp

TEST_SOURCES = tests.cpp \
               ConfigHandler.cpp \
               Logger.cpp \
               ServerException.cpp \
               UserBase.cpp \
               Authenticator.cpp \
               DataProcessor.cpp

OBJECTS = $(SOURCES:.cpp=.o)
TEST_OBJECTS = $(TEST_SOURCES:.cpp=.o)

all: release

release: CXXFLAGS += -O2 -DNDEBUG
release: $(TARGET)

debug: CXXFLAGS += -g -O0
debug: $(TARGET)

test: CXXFLAGS += -O2 -DNDEBUG
test: $(TEST_TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) $(LIBS) -o $(TARGET)

$(TEST_TARGET): $(TEST_OBJECTS)
	$(CXX) $(TEST_OBJECTS) $(TEST_LIBS) -o $(TEST_TARGET)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: release
	./$(TARGET)

run_test: test
	./$(TEST_TARGET)

clean:
	rm -f $(OBJECTS) $(TEST_OBJECTS) $(TARGET) $(TEST_TARGET)
	rm -f vcalc.log unit_test_*.log unit_test_*.conf

.PHONY: all release debug test run run_test clean
