TARGET=../bin/ts
CPPFLAGS+=-Wall
LDLIBS+=-pthread

all: $(TARGET)

$(TARGET): ts.o
	$(LINK.cpp) $^ $(LOADLIBES) $(LDLIBS) -o $@

clean:
	rm -f $(TARGET) *.o
