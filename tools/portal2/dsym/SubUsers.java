//@category Portal2
import java.nio.file.*;
import java.util.*;
import ghidra.app.decompiler.*;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.*;
import ghidra.program.model.symbol.*;
import ghidra.program.model.address.*;
// args: <out> <substring>...  Decompiles every function that references a defined C string
// containing any substring (retail strings are exported as s_<text>_<addr> labels).
public class SubUsers extends GhidraScript {
	public void run() throws Exception {
		String[] a = getScriptArgs();
		StringBuilder sb = new StringBuilder();
		DecompInterface d = new DecompInterface();
		d.openProgram(currentProgram);
		Set<Function> done = new HashSet<>();
		for (Data data : currentProgram.getListing().getDefinedData(true)) {
			Object v = data.getValue();
			if (!(v instanceof String)) continue;
			String s = (String) v;
			boolean hit = false;
			for (int i = 1; i < a.length; i++) if (s.contains(a[i])) hit = true;
			if (!hit) continue;
			sb.append("## string \"" + s.replace("\n", "\\n") + "\" @ " + data.getAddress() + "\n");
			for (Reference r : getReferencesTo(data.getAddress())) {
				Function f = getFunctionContaining(r.getFromAddress());
				if (f == null || done.contains(f)) continue;
				done.add(f);
				sb.append("==== " + f.getName(true) + " @ " + f.getEntryPoint() + " (ref " + r.getFromAddress() + ")\n");
				DecompileResults res = d.decompileFunction(f, 180, monitor);
				if (res.decompileCompleted()) sb.append(res.getDecompiledFunction().getC());
			}
		}
		Files.write(Paths.get(a[0]), sb.toString().getBytes());
	}
}
