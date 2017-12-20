CXX = g++
CXXFLAGS = -g -Wall -Wno-write-strings
PROG = mkfs ls cat tee cp dump mkdir fsck ln rm

LINK = $(CXX) $(CXXFLAGS) $+ -o $@

all: $(PROG)

cp: cp.cc Kernel.o IndexNode.o FileDescriptor.o DirectoryEntry.o ProcessContext.o FileSystem.o Stat.o BitBlock.o Block.o SuperBlock.o
	$(LINK)

tee: tee.cc Kernel.o IndexNode.o FileDescriptor.o DirectoryEntry.o ProcessContext.o FileSystem.o Stat.o BitBlock.o Block.o SuperBlock.o
	$(LINK)

ls: ls.cc Stat.o FileDescriptor.o FileSystem.o BitBlock.o IndexNode.o DirectoryEntry.o Block.o SuperBlock.o ProcessContext.o Kernel.o
	$(LINK)

mkdir: mkdir.cc Stat.o FileDescriptor.o FileSystem.o BitBlock.o IndexNode.o DirectoryEntry.o Block.o SuperBlock.o ProcessContext.o Kernel.o
	$(LINK)

cat: cat.cc Stat.o FileDescriptor.o FileSystem.o BitBlock.o IndexNode.o DirectoryEntry.o Block.o SuperBlock.o ProcessContext.o Kernel.o
	$(LINK)

mkfs: mkfs.cc BitBlock.o IndexNode.o DirectoryEntry.o Block.o SuperBlock.o
	$(LINK)

fsck: fsck.cc Kernel.o FileSystem.o FileDescriptor.o IndexNode.o DirectoryEntry.o Stat.o ProcessContext.o SuperBlock.o Block.o BitBlock.o
	$(LINK)

ln: ln.cc BitBlock.o Block.o SuperBlock.o Kernel.o IndexNode.o FileDescriptor.o DirectoryEntry.o FileSystem.o ProcessContext.o Stat.o
	$(LINK)

rm: rm.cc BitBlock.o Block.o SuperBlock.o Kernel.o IndexNode.o FileDescriptor.o DirectoryEntry.o FileSystem.o ProcessContext.o Stat.o
	$(LINK)

dump: dump.cc
	$(LINK)

%.o: %.cc %.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<

format:
	clang-format -i -style=Google *.cc *.h

clean:
	@rm -f *.o $(PROG)
