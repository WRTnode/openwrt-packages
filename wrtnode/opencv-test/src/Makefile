PROGS     = cv_test 

INSTDIR   = $(prefix)/usr/bin
INSTMODE  = 0755
INSTOWNER = root
INSTGROUP = root

OBJS = cv_test.o
LDLIBS += -lm -lopencv_core -lopencv_highgui -lopencv_calib3d \
	   -lopencv_contrib -lopencv_features2d -lopencv_flann \
	   -lopencv_imgproc -lopencv_legacy -lopencv_ml \
	   -lopencv_nonfree -lopencv_objdetect -lopencv_ocl \
	   -lopencv_photo -lopencv_video -lopencv_videostab  

all: $(PROGS)
$(PROGS): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ $(LDLIBS) -o $@

%.o: %.c
	$(CC) -c $(CFLAGS) $^ -o $@

install: $(PROGS)
	$(INSTALL) -d $(INSTDIR)
	$(INSTALL) -m $(INSTMODE) -o $(INSTOWNER) -g $(INSTGROUP) $(PROGS) $(INSTDIR)

clean:
	rm -f $(PROGS) *.o

