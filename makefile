# Author: i0gan
# Do    : For compile awd waf program
# Date  : 2020-12-15

GCC  := gcc
CFLAGS := 
LOG_PATH := /tmp/.i0gan
ARCH := 64

HEX := hex
RM = rm -rf

TEST_PWN_SRC := src/test_pwn.c
HEX_SRC    := src/hex.c
LOGGER_SRC := src/logger.c
WAF_SRC    := src/waf.c

all : catch i0gan redir test_pwn

test_pwn :
	@mkdir $(LOG_PATH)
	$(GCC) $(TEST_PWN_SRC) -o $@
	@cp $@ $(LOG_PATH)/pwn
	@echo -e "127.0.0.1:10100\n" > $(LOG_PATH)/hosts

clean :
	$(RM) catch i0gan redir test_pwn $(LOG_PATH)

catch :
	$(GCC) $(CFLAGS) $(WAF_SRC) $(LOGGER_SRC) -DRUN_MODE=RUN_CATCH -DLOG_PATH=\"$(LOG_PATH)\" -DARCH=$(ARCH) -o $@

i0gan :
	$(GCC) $(CFLAGS) $(WAF_SRC) $(LOGGER_SRC) -DRUN_MODE=RUN_I0GAN -DLOG_PATH=\"$(LOG_PATH)\" -DARCH=$(ARCH) -o $@

redir : 
	$(GCC) $(CFLAGS) $(WAF_SRC) $(LOGGER_SRC) -DRUN_MODE=RUN_REDIR -DLOG_PATH=\"$(LOG_PATH)\" -DARCH=$(ARCH) -o $@
