# Leaf count R_BuildWorldLists produced and the view origin, for 3 views.
set $n = 0
break R_BuildWorldLists
commands
silent
set $info = $r1
continue
end
break R_DrawWorldLists
commands
silent
set $n = $n + 1
if $n % 50 == 0
printf "WORLDLISTS leaves %d fogvol %d origin %f %f %f\n", *((int *)$info + 1), *(int *)$info, ((float *)&g_CurrentViewOrigin)[0], ((float *)&g_CurrentViewOrigin)[1], ((float *)&g_CurrentViewOrigin)[2]
end
if $n == 150
delete
end
continue
end
