# Logs every allocation of at least 1 MB with its caller chain, then
# continues: where the 3DS heap goes during startup.
break malloc if $r0 >= 1048576
commands
silent
printf "malloc %u\n", $r0
bt 7
continue
end
break realloc if $r1 >= 1048576
commands
silent
printf "realloc %u\n", $r1
bt 7
continue
end
break memalign if $r1 >= 1048576
commands
silent
printf "memalign %u\n", $r1
bt 7
continue
end
