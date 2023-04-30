# Target library
lib := libuthread.a
objs    := queue.o uthread.o context.o preempt.o
CC      := gcc

CFLAGS  := -Wall -Wextra -Werror -MMD
#CFLAGS  += -g

ifneq ($(V),1)
Q=@
endif

# Dep tracking *must* be below the 'all' rule
deps := $(patsubst %.o,%.d,$(objs))
-include $(deps)

$(lib): $(objs)
	@echo "CC $@"
	$(Q) ar rcs $(lib) $(objs)


%.o: %.c
	@echo "CC $@"
	$(Q)$(CC) $(CFLAGS) -c -o $@ $<

clean:
	@echo "clean"
	$(Q)rm -f $(lib) $(objs) $(deps)