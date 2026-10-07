# Prints the message of every fatal engine error as it happens (loaded by
# every azahar_harness.py debug run).
break Sys_Error
commands
silent
printf "Sys_Error: %s\n", (char *)$r0
continue
end
break Error
commands
silent
printf "Error: %s\n", (char *)$r0
continue
end
break Warning
commands
silent
printf "Warning: %s", (char *)$r0
x/s $r1
continue
end
