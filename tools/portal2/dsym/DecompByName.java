//@category Portal2
// Local research aid; output is decompiler pseudocode, not source.
import java.io.*;
import java.nio.file.*;
import ghidra.app.decompiler.*;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.*;
public class DecompByName extends GhidraScript {
	public void run() throws Exception {
		String[] a = getScriptArgs();
		StringBuilder sb = new StringBuilder();
		DecompInterface d = new DecompInterface();
		d.openProgram(currentProgram);
		sb.append("program " + currentProgram.getName() + "\n");
		for (int i = 1; i < a.length; i++) {
			for (Function f : currentProgram.getFunctionManager().getFunctions(true)) {
				String n = f.getName(true);
				if (!n.contains(a[i])) continue;
				sb.append("==== " + n + " @ " + f.getEntryPoint() + "\n");
				DecompileResults r = d.decompileFunction(f, 120, monitor);
				if (r.decompileCompleted()) sb.append(r.getDecompiledFunction().getC());
			}
		}
		Files.write(Paths.get(a[0]), sb.toString().getBytes());
	}
}
