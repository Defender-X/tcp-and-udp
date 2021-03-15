TARGET=../bin/tc
LDLIBS+=-pthread

all: $(TARGET)

$(TARGET): tc.o
	$(LINK.cpp) $^ $(LOADLIBES) $(LDLIBS) -o $@

clean:
	rm -f $(TARGET) *.o
