# The allocation that ran out of memory: its size, call stack, and the heap
# (dumped through the harness while the guest is halted here) for
# tools/n3ds/heap_census.py build-3ds/heap_oom.bin.
break MemAllocOOMError
commands
silent
printf "OOM size %u\n", $r0
bt 12
shell python3 /home/john/src/source-engine-3ds/tools/n3ds/azahar_harness.py send dump 8000000 117440512 /home/john/src/source-engine-3ds/build-3ds/heap_oom.bin
continue
end
