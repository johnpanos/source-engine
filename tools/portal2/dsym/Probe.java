//@category Portal2
import java.nio.file.*;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.symbol.*;
import ghidra.program.model.mem.*;

// args: <out> <hexAddress>...
// Prints, for each Ghidra address: the 4-byte value there, whether a datum
// covers it, and every reference from and to it. Used to find why data
// references to datadesc strings are missing.
public class Probe extends GhidraScript {
	public void run() throws Exception {
		String[] a = getScriptArgs();
		StringBuilder sb = new StringBuilder();
		Memory mem = currentProgram.getMemory();
		for (int i = 1; i < a.length; i++) {
			Address addr = currentProgram.getAddressFactory().getDefaultAddressSpace()
				.getAddress(Long.decode(a[i]).longValue());
			sb.append("== ").append(a[i]).append('\n');
			if (!mem.contains(addr)) {
				sb.append("   not in memory\n");
				continue;
			}
			int v = mem.getInt(addr);
			sb.append(String.format("   dword=0x%08x (%d)\n", v, v));
			Data d = getDataAt(addr);
			sb.append("   dataAt=").append(d == null ? "null" : d.getAddress() + " " + d.getClass().getSimpleName()
				+ " " + d.getLength()).append('\n');
			sb.append("   containing block=").append(mem.getBlock(addr).getName()).append('\n');
			Reference[] refs = getReferencesFrom(addr);
			int n = 0;
			for (Reference r : refs) {
				if (n++ >= 8) break;
				sb.append("   refFrom -> ").append(r.getToAddress()).append(" ")
					.append(r.getReferenceType()).append('\n');
			}
			if (n == 0) sb.append("   refFrom: none\n");
			n = 0;
			for (Reference r : getReferencesTo(addr)) {
				if (n++ >= 8) break;
				sb.append("   refTo <- ").append(r.getFromAddress()).append(" ")
					.append(r.getReferenceType()).append('\n');
			}
			if (n == 0) sb.append("   refTo: none\n");
		}
		Files.write(Paths.get(a[0]), sb.toString().getBytes());
	}
}
