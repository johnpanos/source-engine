//@category Portal2
// Local research aid; output is decompiler pseudocode, not source.
import java.nio.file.*;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.*;
import ghidra.program.model.symbol.*;
public class Callers extends GhidraScript {
	public void run() throws Exception {
		String[] a = getScriptArgs();
		StringBuilder sb = new StringBuilder();
		FunctionManager fm = currentProgram.getFunctionManager();
		for (int i = 1; i < a.length; i++) {
			for (Function f : fm.getFunctions(true)) {
				if (!f.getName(true).contains(a[i])) continue;
				sb.append("== " + f.getName(true) + " @ " + f.getEntryPoint() + "\n");
				for (Reference r : getReferencesTo(f.getEntryPoint())) {
					Function c = fm.getFunctionContaining(r.getFromAddress());
					sb.append("   <- " + (c == null ? "?" : c.getName(true)) + " " + r.getReferenceType() + " @" + r.getFromAddress() + "\n");
				}
			}
		}
		Files.write(Paths.get(a[0]), sb.toString().getBytes());
	}
}
