MANAGER = uixo_manager
FORK = uixo_manager_fork_object

MANAGER_OBJS = uixo_manager.o uixo_manager_lib.o utilities.o
FORK_OBJS = uixo_manager_objects_fork.o uixo_manager_lib.o utilities.o

LDLIBS += -lubox -lubus -lblobmsg_json

all: $(MANAGER) $(FORK)
$(MANAGER): $(MANAGER_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ $(LDLIBS) -o $(MANAGER)
$(FORK): $(FORK_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ $(LDLIBS) -o $(FORK)

%.o: %.c
	$(CC) -c $(CFLAGS) $^ $(LDLIBS) -o $@

clean:
	rm -f *.o *.*~  core $(MANAGER) $(FORK)
