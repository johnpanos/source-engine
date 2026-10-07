# The client's view rectangle (x, y, width, height) as the engine hands it
# to CHLClient::View_Render; prints the first few frames.
set $views = 0
break CHLClient::View_Render
commands
silent
set $views = $views + 1
if $views <= 5
printf "VIEWRECT %d %d %d %d\n", *(int *)$r1, *((int *)$r1 + 1), *((int *)$r1 + 2), *((int *)$r1 + 3)
end
if $views == 5
delete
end
continue
end
