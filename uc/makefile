TARGET=../bin/uc
CPPFLAGS+=-Wall
LDLIBS+=-pthread

all: $(TARGET)

$(TARGET): uc.o
	$(LINK.cpp) $^ $(LOADLIBES) $(LDLIBS) -o $@

clean:
	rm -f $(TARGET) *.o
