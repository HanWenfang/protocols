

include common.mk

default: cppRaft

cppRaft: $(obj-cppRaft)
	$(CPP) $^ -o $@ $(LIBRARY) $(SYSLIBS)

%.o: %.cpp
	$(CPP) -Wall $(INCLUDE) -c $< -o $@

clean:
	rm -rf cppRaft $(obj-cppRaft)

