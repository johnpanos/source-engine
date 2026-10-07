# The clear colours the engine sets on the PICA API (first 8).
set $c = 0
break CShaderAPIEmpty::ClearColor3ub
commands
silent
set $c = $c + 1
if $c <= 8
printf "CLEAR3 %d %d %d caller ", $r1 & 255, $r2 & 255, $r3 & 255
info symbol $lr
end
continue
end
break CShaderAPIEmpty::ClearColor4ub
commands
silent
set $c = $c + 1
if $c <= 8
printf "CLEAR4 %d %d %d caller ", $r1 & 255, $r2 & 255, $r3 & 255
info symbol $lr
end
continue
end
