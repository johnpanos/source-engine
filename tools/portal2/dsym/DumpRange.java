//@category Portal2
import java.nio.file.*;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.*;
import ghidra.program.model.mem.*;

// args: <out> <startHexAddress> <dwordCount>
// Dumps dwords with string dereferences; used to discover the retail
// typedescription_t layout empirically.
public class DumpRange extends GhidraScript {
	public void run() throws Exception {
		String[] a = getScriptArgs();
		StringBuilder sb = new StringBuilder();
		Memory mem = currentProgram.getMemory();
		Address p = currentProgram.getAddressFactory().getDefaultAddressSpace()
			.getAddress(Long.decode(a[1]).longValue());
		int n = Integer.parseInt(a[2]);
		for (int i = 0; i < n; i++) {
			Address at = p.add(i * 4);
			int v;
			try {
				v = mem.getInt(at);
			} catch (Exception e) {
				sb.append(String.format("%08x: <unreadable>%n", at));
				continue;
			}
			sb.append(String.format("%08x: %08x", at.getOffset(), v & 0xffffffffL));
			Address t = currentProgram.getAddressFactory().getDefaultAddressSpace()
				.getAddress(v & 0xffffffffL);
			if (mem.contains(t)) {
				int b = mem.getByte(t) & 0xff;
				if (b >= 0x20 && b < 0x7f) {
					StringBuilder s = new StringBuilder();
					boolean ok = true;
					for (int k = 0; k < 96; k++) {
						Address q = t.add(k);
						if (!mem.contains(q)) { ok = false; break; }
						int c = mem.getByte(q) & 0xff;
						if (c == 0) break;
						if (c < 0x20 || c > 0x7e) { ok = false; break; }
						s.append((char) c);
					}
					if (ok) sb.append("  -> \"").append(s).append('"');
				}
			}
			sb.append('\n');
		}
		Files.write(Paths.get(a[0]), sb.toString().getBytes());
	}
}
