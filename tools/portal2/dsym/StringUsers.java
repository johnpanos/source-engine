//@category Portal2
import java.nio.file.*;
import java.util.*;
import ghidra.app.decompiler.*;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.*;
import ghidra.program.model.symbol.*;
import ghidra.program.model.address.*;
import ghidra.program.model.mem.*;
// args: <out> <exact string>...  Decompiles every function that references each exact C string.
public class StringUsers extends GhidraScript {
	public void run() throws Exception {
		String[] a = getScriptArgs();
		StringBuilder sb = new StringBuilder();
		DecompInterface d = new DecompInterface();
		d.openProgram(currentProgram);
		Memory mem = currentProgram.getMemory();
		Set<Function> done = new HashSet<>();
		for (int i = 1; i < a.length; i++) {
			byte[] pat = (a[i] + "\0").getBytes("latin1");
			Address start = currentProgram.getMinAddress();
			while (true) {
				Address hit = mem.findBytes(start, pat, null, true, monitor);
				if (hit == null) break;
				start = hit.add(1);
				// require a NUL before (string start)
				try { if (mem.getByte(hit.subtract(1)) != 0) continue; } catch (Exception e) {}
				sb.append("## string " + a[i] + " @ " + hit + "\n");
				for (Reference r : getReferencesTo(hit)) {
					Function f = getFunctionContaining(r.getFromAddress());
					if (f == null || done.contains(f)) continue;
					done.add(f);
					sb.append("==== " + f.getName(true) + " @ " + f.getEntryPoint() + " (ref " + r.getFromAddress() + ")\n");
					DecompileResults res = d.decompileFunction(f, 180, monitor);
					if (res.decompileCompleted()) sb.append(res.getDecompiledFunction().getC());
				}
			}
		}
		Files.write(Paths.get(a[0]), sb.toString().getBytes());
	}
}
