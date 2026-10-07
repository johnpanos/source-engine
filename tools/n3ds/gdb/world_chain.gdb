# Counts calls along the client's world-draw chain and prints the counts every
# 100 client views (three summaries, then the breakpoints are removed).
set $views = 0
set $renderview = 0
set $drawscene = 0
set $drawworld = 0
set $buildlists = 0
set $drawlists = 0
set $summaries = 0
break CViewRender::RenderView
commands
silent
set $renderview = $renderview + 1
continue
end
break CViewRender::ViewDrawScene
commands
silent
set $drawscene = $drawscene + 1
continue
end
break CRendering3dView::DrawWorld
commands
silent
set $drawworld = $drawworld + 1
continue
end
break R_BuildWorldLists
commands
silent
set $buildlists = $buildlists + 1
continue
end
break R_DrawWorldLists
commands
silent
set $drawlists = $drawlists + 1
continue
end
break CHLClient::View_Render
commands
silent
set $views = $views + 1
if $views % 100 == 0
printf "CHAIN views %d renderview %d drawscene %d drawworld %d buildlists %d drawlists %d\n", $views, $renderview, $drawscene, $drawworld, $buildlists, $drawlists
set $summaries = $summaries + 1
if $summaries == 3
delete
end
end
continue
end
