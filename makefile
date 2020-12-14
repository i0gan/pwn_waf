TARGET := pwn
GCC := gcc
TEST := /tmp/.i0gan/pwn
LOG_PATH := /tmp/.i0gan
HEX := hex
RM = rm -rf
MAIN_SRC := src/i0gan.c
TEST_SRC := src/test.c
HEX_SRC := src/hex.c

$(TARGET) : $(MAIN_SRC) $(TEST) $(HEX)
	$(GCC) $(MAIN_SRC) -o $(TARGET)

$(TEST) : $(TEST_SRC)
	@mkdir $(LOG_PATH)
	$(GCC) $< -o $@
	
$(HEX) : $(HEX_SRC)
	$(GCC) $< -o $@
clean:
	$(RM) $(TARGET) $(TEST) $(HEX) $(LOG_PATH)
