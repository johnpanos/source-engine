# Logs every search path the file system adds: AddSearchPathInternal(this,
# path, pathID, addType, bAddPackFiles) -> r1 path, r2 path ID.
break CBaseFileSystem::AddSearchPathInternal
commands
silent
printf "AddSearchPath %s [%s]\n", (char *)$r1, (char *)$r2
continue
end
