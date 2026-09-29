//@category Portal2
import java.nio.file.*;
import ghidra.app.decompiler.*;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.*;
import ghidra.program.model.address.*;
import ghidra.program.model.mem.*;
// args: <out> <hexaddr>...  An address prefixed with '*' is a pointer slot: read it, then decompile the target.
public class DerefDecomp extends GhidraScript {
	public void run() throws Exception {
		String[] a = getScriptArgs();
		StringBuilder sb = new StringBuilder();
		Memory mem = currentProgram.getMemory();
		AddressSpace sp = currentProgram.getAddressFactory().getDefaultAddressSpace();
		DecompInterface d = new DecompInterface(); d.openProgram(currentProgram);
		for (int i = 1; i < a.length; i++) {
			String s = a[i]; boolean deref = s.startsWith("*"); if (deref) s = s.substring(1);
			Address ad = sp.getAddress(Long.parseLong(s, 16));
			if (deref) {
				long v = mem.getInt(ad) & 0xffffffffL;
				sb.append("## slot " + ad + " -> " + Long.toHexString(v) + " next " + Long.toHexString(mem.getInt(ad.add(4)) & 0xffffffffL) + "\n");
				ad = sp.getAddress(v);
			}
			Function f = getFunctionContaining(ad);
			if (f == null) f = createFunction(ad, null);
			if (f == null) { sb.append("no function at " + ad + "\n"); continue; }
			sb.append("==== " + f.getName() + " @ " + f.getEntryPoint() + "\n");
			DecompileResults r = d.decompileFunction(f, 240, monitor);
			if (r.decompileCompleted()) sb.append(r.getDecompiledFunction().getC());
		}
		Files.write(Paths.get(a[0]), sb.toString().getBytes());
	}
}
