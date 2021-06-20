# Author: i0gan
# Do    : For compile waf program
# Date  : 2020-12-15

GCC    := gcc
CFLAGS := -O2
RM     := rm -rf

# configure
# log path
LOG_PATH    := /tmp/.waf
ARCH        := 64

# src
TEST_PWN_SRC := src/test_pwn.c
HEX_SRC      := src/hex.c
LOGGER_SRC   := src/logger.c
WAF_SRC      := src/waf.c

all : catch i0gan forward test_pwn

test_pwn :
	@mkdir $(LOG_PATH)
	$(GCC) $(TEST_PWN_SRC) -o $@
	@cp $@ $(LOG_PATH)/pwn
	@cp hosts.txt $(LOG_PATH)/

clean :
	$(RM) catch i0gan forward test_pwn $(LOG_PATH)

catch :
	$(GCC) $(CFLAGS) $(WAF_SRC) $(LOGGER_SRC) -DRUN_MODE=RUN_CATCH -DLOG_PATH=\"$(LOG_PATH)\" -DARCH=$(ARCH) -o $@

i0gan :
	$(GCC) $(CFLAGS) $(WAF_SRC) $(LOGGER_SRC) -DRUN_MODE=RUN_I0GAN -DLOG_PATH=\"$(LOG_PATH)\" -DARCH=$(ARCH) -o $@

forward : 
	$(GCC) $(CFLAGS) $(WAF_SRC) $(LOGGER_SRC) -DRUN_MODE=RUN_FORWARD -DLOG_PATH=\"$(LOG_PATH)\" -DARCH=$(ARCH) -DSERVER_IP=\"$(SERVER_IP)\" -DSERVER_PORT=$(SERVER_PORT) -o $@
