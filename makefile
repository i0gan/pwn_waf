# Author: i0gan
# Do    : For compile waf program
# Date  : 2020-12-15

GCC    := gcc
CFLAGS := -O2 --static
RM     := rm -rf

# configure
# log path
LOG_METHOD  := OPEN
LOG_PATH    := /tmp/.waf
ARCH        := 64

# Just used in FORWARD mode
FORWARD_IP   := 127.0.0.1
FORWARD_PORT := 20000

# src
TEST_PWN_SRC := src/test_pwn.c
HEX_SRC      := src/hex.c
LOGGER_SRC   := src/logger.c
WAF_SRC      := src/waf.c

all : catch i0gan forward forward_multi test_pwn

test_pwn :
	@mkdir $(LOG_PATH)
	$(GCC) $(TEST_PWN_SRC) -o $@
	@cp $@ $(LOG_PATH)/pwn
	@cp hosts.txt $(LOG_PATH)/

clean :
	$(RM) catch i0gan forward forward_multi test_pwn $(LOG_PATH)

catch :
	$(GCC) $(CFLAGS) $(WAF_SRC) $(LOGGER_SRC) -DRUN_MODE=CATCH -DLOG_METHOD=$(LOG_METHOD) -DLOG_PATH=\"$(LOG_PATH)\" -DARCH=$(ARCH) -o $@

i0gan :
	$(GCC) $(CFLAGS) $(WAF_SRC) $(LOGGER_SRC) -DRUN_MODE=I0GAN -DLOG_METHOD=$(LOG_METHOD) -DLOG_PATH=\"$(LOG_PATH)\" -DARCH=$(ARCH) -o $@

forward : 
	$(GCC) $(CFLAGS) $(WAF_SRC) $(LOGGER_SRC) -DRUN_MODE=FORWARD -DLOG_METHOD=$(LOG_METHOD) -DLOG_PATH=\"$(LOG_PATH)\" -DARCH=$(ARCH) -DFORWARD_IP=\"$(FORWARD_IP)\" -DFORWARD_PORT=$(FORWARD_PORT) -o $@

forward_multi : 
	$(GCC) $(CFLAGS) $(WAF_SRC) $(LOGGER_SRC) -DRUN_MODE=FORWARD_MULTI -DLOG_METHOD=$(LOG_METHOD) -DLOG_PATH=\"$(LOG_PATH)\" -DARCH=$(ARCH) -o $@
