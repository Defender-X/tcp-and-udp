TARGET=../bin/us
LDLIBS+=-pthread

all: $(TARGET)

$(TARGET): us.o
		$(LINK.cpp) $^ $(LOADLIBES) $(LDLIBS) -o $@

clean:
	rm -f $(TARGET) *.o
