# Logs every CMemoryStack::Init (its maximum size is allocated whole on the
# 3DS, which has no virtual memory to reserve) with its caller.
break CMemoryStack::Init
commands
silent
printf "CMemoryStack::Init max %u\n", $r1
bt 4
continue
end
