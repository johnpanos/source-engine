# Samples CEmptyMesh draws in the PICA backend (no type info: raw arguments
# and the caller), to see which paths the world's surfaces take.
set $n = 0
break CEmptyMesh::Draw(CPrimList*, int)
commands
silent
set $n = $n + 1
if $n < 80
printf "primlist prims=%d caller ", $r2
info symbol $lr
end
continue
end
break CEmptyMesh::Draw(int, int)
commands
silent
set $n = $n + 1
if $n < 80
printf "draw first=%d num=%d caller ", $r1, $r2
info symbol $lr
end
continue
end
