//@category Portal2
import java.nio.file.*;
import java.util.*;
import java.util.regex.*;
import ghidra.app.decompiler.*;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.symbol.*;

// args: <out> <entryHexAddress> [windowBytes]
// Retail datamap_t objects live in .bss and are filled by static initializers,
// so the class name cannot be read from the image. Instead, walk back from the
// entry to candidate array starts, keep those the code references (the
// initializer writes dataDesc with an immediate), decompile the writer and
// report the "C..." class names it assigns.
public class FindOwner extends GhidraScript {
	public void run() throws Exception {
		String[] a = getScriptArgs();
		StringBuilder sb = new StringBuilder();
		Address entry = currentProgram.getAddressFactory().getDefaultAddressSpace()
			.getAddress(Long.decode(a[1]).longValue());
		int window = a.length > 2 ? Long.decode(a[2]).intValue() : 0x4000;
		DecompInterface d = new DecompInterface();
		d.openProgram(currentProgram);

		for (int off = 0; off <= window; off += 0x40) {
			Address v = entry.subtract(off);
			Reference[] rs = getReferencesTo(v);
			if (rs.length == 0) continue;
			sb.append("candidate array @ ").append(v).append(" index ").append(off / 0x40)
				.append(" refs=").append(rs.length).append('\n');
			Set<Address> funcs = new LinkedHashSet<>();
			for (Reference r : rs) {
				Function f = getFunctionContaining(r.getFromAddress());
				if (f != null) funcs.add(f.getEntryPoint());
			}
			for (Address fp : funcs) {
				Function f = getFunctionAt(fp);
				sb.append("  writer ").append(f.getName(true)).append(" @ ").append(fp).append('\n');
				DecompileResults res = d.decompileFunction(f, 300, monitor);
				if (!res.decompileCompleted()) continue;
				String c = res.getDecompiledFunction().getC();
				Matcher m = Pattern.compile("\"(C[A-Za-z0-9_]{3,60})\"").matcher(c);
				Set<String> names = new LinkedHashSet<>();
				while (m.find()) names.add(m.group(1));
				sb.append("    class names assigned: ").append(names).append('\n');
				// first lines often show the datamap header being filled
				String[] lines = c.split("\n");
				for (int i = 0; i < Math.min(lines.length, 6); i++) {
					sb.append("    | ").append(lines[i]).append('\n');
				}
			}
		}
		d.dispose();
		Files.write(Paths.get(a[0]), sb.toString().getBytes());
	}
}
