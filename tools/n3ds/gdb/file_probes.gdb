# Logs every file the file system opens on the SD card (CStdioFile::FS_fopen:
# r0 is the path), to see what the startup probes.
break CStdioFile::FS_fopen
commands
silent
printf "fopen %s\n", (char *)$r0
continue
end
