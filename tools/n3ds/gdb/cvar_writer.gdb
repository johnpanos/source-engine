# Who writes a given ConVar: conditional breakpoints on every copy of the
# ConVar setters, filtered to the objects' addresses ($cv1, $cv2), with a
# short backtrace each time.
set $cv1 = 0x0349c0cc
set $cv2 = 0x03932cf0
break ConVar::InternalSetValue(char const*) if $r0 == $cv1 || $r0 == $cv2
commands
silent
printf "CVARSET str obj %x value %s\n", $r0, (char *)$r1
bt 8
continue
end
break ConVar::InternalSetIntValue(int) if $r0 == $cv1 || $r0 == $cv2
commands
silent
printf "CVARSET int obj %x value %d\n", $r0, $r1
bt 8
continue
end
