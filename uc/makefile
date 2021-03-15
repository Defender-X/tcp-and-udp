TARGET=../bin/uc
LDLIBS+=-pthread

all: $(TARGET)

$(TARGET): uc.o
	$(LINK.cpp) $^ $(LOADLIBES) $(LDLIBS) -o $@

clean:
	rm -f $(TARGET) *.o
