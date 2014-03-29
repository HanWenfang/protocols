
INCLUDE = -I. -I/usr/local/include
CPP 	= g++
LIBRARY = -L. -L/usr/local/lib
SYSLIBS = -lPocoFoundation

#core
obj-cppRaft += Raft.o
obj-cppRaft += ArgumentsParser.o
obj-cppRaft += ComputeCore.o
obj-cppRaft += Connect.o
obj-cppRaft += AsynCore.o
obj-cppRaft += Message.o
obj-cppRaft += Protocol.o
obj-cppRaft += Entry.o
obj-cppRaft += cJSON.o