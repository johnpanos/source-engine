# Counts texture calls into each linked CShaderAPIEmpty (the stock empty API
# at 0x141xxx, the PICA one at 0x1b3xxx) and CTexture::Download.
set $e_vtf = 0
set $e_img = 0
set $e_create = 0
set $p_vtf = 0
set $p_img = 0
set $p_create = 0
set $download = 0
set $views = 0
break *0x00141aa0
commands
silent
set $e_vtf = $e_vtf + 1
continue
end
break *0x00141d04
commands
silent
set $e_img = $e_img + 1
continue
end
break *0x00141d1c
commands
silent
set $e_create = $e_create + 1
continue
end
break *0x001b3634
commands
silent
set $p_vtf = $p_vtf + 1
continue
end
break *0x001b3190
commands
silent
set $p_img = $p_img + 1
continue
end
break *0x001b2aa8
commands
silent
set $p_create = $p_create + 1
continue
end
break CTexture::Download
commands
silent
set $download = $download + 1
continue
end
break CHLClient::View_Render
commands
silent
set $views = $views + 1
if $views == 60
printf "TEX empty: vtf %d img %d create %d | pica: vtf %d img %d create %d | downloads %d\n", $e_vtf, $e_img, $e_create, $p_vtf, $p_img, $p_create, $download
delete
end
continue
end
