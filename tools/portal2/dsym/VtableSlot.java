//@category Portal2
import java.nio.file.*;
import java.util.*;
import ghidra.app.decompiler.*;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.*;
import ghidra.program.model.address.*;
import ghidra.program.model.mem.*;
// args: <out> <mangled typeinfo name e.g. 23CNPC_Portal_FloorTurret> <byteoffset hex>...
// Finds the primary vtable (offset-to-top 0) whose typeinfo names the class and decompiles the given slots.
public class VtableSlot extends GhidraScript {
	List<Address> findWord(long v) throws Exception {
		Memory mem = currentProgram.getMemory(); List<Address> out = new ArrayList<>();
		byte[] w = {(byte)v,(byte)(v>>8),(byte)(v>>16),(byte)(v>>24)};
		Address s = currentProgram.getMinAddress();
		while (true) { Address r = mem.findBytes(s, w, null, true, monitor); if (r == null) break; out.add(r); s = r.add(1); }
		return out;
	}
	public void run() throws Exception {
		String[] a = getScriptArgs();
		StringBuilder sb = new StringBuilder();
		Memory mem = currentProgram.getMemory();
		AddressSpace sp = currentProgram.getAddressFactory().getDefaultAddressSpace();
		DecompInterface d = new DecompInterface(); d.openProgram(currentProgram);
		byte[] pat = (a[1] + "\0").getBytes("latin1");
		Address s = currentProgram.getMinAddress();
		while (true) {
			Address name = mem.findBytes(s, pat, null, true, monitor);
			if (name == null) break; s = name.add(1);
			try { if (mem.getByte(name.subtract(1)) != 0) continue; } catch (Exception e) {}
			for (Address tiName : findWord(name.getOffset())) {
				Address ti = tiName.subtract(4);
				for (Address vtTi : findWord(ti.getOffset())) {
					if (mem.getInt(vtTi.subtract(4)) != 0) continue; // offset-to-top
					Address vt = vtTi.add(4);
					sb.append("## vtable " + vt + " typeinfo " + ti + "\n");
					for (int i = 2; i < a.length; i++) {
						Address slot = vt.add(Long.parseLong(a[i], 16));
						long fn = mem.getInt(slot) & 0xffffffffL;
						Address fa = sp.getAddress(fn);
						Function f = getFunctionAt(fa); if (f == null) f = createFunction(fa, null);
						sb.append("==== slot +" + a[i] + " -> " + fa + (f == null ? " (no function)" : " " + f.getName()) + "\n");
						if (f != null) { DecompileResults r = d.decompileFunction(f, 240, monitor); if (r.decompileCompleted()) sb.append(r.getDecompiledFunction().getC()); }
					}
				}
			}
		}
		Files.write(Paths.get(a[0]), sb.toString().getBytes());
	}
}
