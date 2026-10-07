# From the startup graphic on, log each file open and stat and its result.
break CVideoMode_Common::SetupStartupGraphic
commands
silent
enable 2 3 4
continue
end
break _open_r
commands
silent
printf "open %s\n", (char *)$r1
continue
end
break _stat_r
commands
silent
printf "stat %s\n", (char *)$r1
continue
end
break _fopen_r
commands
silent
printf "fopen %s\n", (char *)$r1
continue
end
disable 2 3 4
