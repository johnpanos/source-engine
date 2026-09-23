// Export per-file pseudocode for a selected Portal 2 source gap list.
// The output is a reverse engineering aid, not original or buildable C++.
//@category Portal2
import java.io.BufferedWriter;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.database.sourcemap.SourceFile;
import ghidra.program.model.listing.Function;
import ghidra.program.model.sourcemap.SourceFileManager;
import ghidra.program.model.sourcemap.SourceMapEntry;

public class ExportMissingSourceTree extends GhidraScript
{
    @Override
    protected void run() throws Exception
    {
        String[] args = getScriptArgs();
        if (args.length != 2)
        {
            throw new IllegalArgumentException("expected missing-list file and output directory");
        }
        List<String> targets = Files.readAllLines(Paths.get(args[0]), StandardCharsets.UTF_8);
        Path outputRoot = Paths.get(args[1]);
        Files.createDirectories(outputRoot);
        SourceFileManager manager = currentProgram.getSourceFileManager();
        Map<String, Set<Function>> functionsByPath = new LinkedHashMap<>();
        for (String target : targets)
        {
            if (!target.isBlank())
            {
                functionsByPath.put(target, new LinkedHashSet<>());
            }
        }
        int matchingSourceFiles = 0;
        for (SourceFile source : manager.getAllSourceFiles())
        {
            String normalized = Paths.get(source.getPath()).normalize().toString();
            for (Map.Entry<String, Set<Function>> item : functionsByPath.entrySet())
            {
                if (!normalized.endsWith(item.getKey()))
                {
                    continue;
                }
                matchingSourceFiles++;
                for (SourceMapEntry entry : manager.getSourceMapEntries(source))
                {
                    Function function = currentProgram.getFunctionManager()
                        .getFunctionContaining(entry.getBaseAddress());
                    if (function != null)
                    {
                        item.getValue().add(function);
                    }
                }
            }
        }
        println("matched source records=" + matchingSourceFiles);
        DecompInterface decompiler = new DecompInterface();
        decompiler.openProgram(currentProgram);
        int filesWritten = 0;
        int functionsWritten = 0;
        int failures = 0;
        for (Map.Entry<String, Set<Function>> item : functionsByPath.entrySet())
        {
            monitor.checkCancelled();
            if (item.getValue().isEmpty())
            {
                continue;
            }
            Path output = outputRoot.resolve(item.getKey() + ".decompiled.c");
            Files.createDirectories(output.getParent());
            try (BufferedWriter writer = Files.newBufferedWriter(output, StandardCharsets.UTF_8))
            {
                writer.write("/* DWARF-guided pseudocode for " + item.getKey() + ".\n");
                writer.write(" * This is not the original C++ source or a buildable translation.\n");
                writer.write(" * Binary: " + currentProgram.getName() + ".\n */\n");
                for (Function function : item.getValue())
                {
                    monitor.checkCancelled();
                    DecompileResults result = decompiler.decompileFunction(function, 60, monitor);
                    writer.write("\n/* " + function.getName(true) + " at " + function.getEntryPoint() + " */\n");
                    if (result.decompileCompleted() && result.getDecompiledFunction() != null)
                    {
                        writer.write(result.getDecompiledFunction().getC());
                        functionsWritten++;
                    }
                    else
                    {
                        writer.write("/* decompilation failed: " + result.getErrorMessage() + " */\n");
                        failures++;
                    }
                }
            }
            filesWritten++;
            println(item.getKey() + " functions=" + item.getValue().size());
        }
        decompiler.dispose();
        println("wrote files=" + filesWritten + " functions=" + functionsWritten + " failures=" + failures);
    }
}
