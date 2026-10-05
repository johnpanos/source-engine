//@category Portal2
import java.nio.file.*;
import java.util.*;
import ghidra.app.decompiler.*;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.*;
import ghidra.program.model.symbol.*;
import ghidra.program.model.address.*;
import ghidra.program.model.mem.*;

// args: <out> <string>...
//
// Retail Portal 2 datadesc lookup.
//
// The retail binaries tail-merge string literals, so a key name is often only
// a suffix of a longer one; every occurrence of <string>\0 is found and the
// start of the merged literal is resolved. References come from two places:
//
//  * code: static initializers that build a datadesc at runtime (Ghidra's
//    reference manager finds these immediates);
//  * data: typedescription slots in .data filled by R_386_RELATIVE
//    relocations. Ghidra applies those values but creates no references, so
//    this script scans writable memory for the dword itself.
//
// Retail typedescription_t layout, discovered empirically (stride 0x40):
//   +0x00 fieldType (int), +0x04 fieldName (char*), +0x08 fieldOffset (int),
//   +0x0c fieldSize (u16), +0x0e flags (s16), +0x10 externalName (char*),
//   +0x14 pSaveRestoreOps, +0x18 inputFunc, +0x1c td,
//   +0x20 fieldSizeInBytes, +0x24 (1 for scalar keyfields, else 0).
// A non-null inputFunc is decompiled.
public class DatadescLookup extends GhidraScript {
	private static final int EXT_OFF = 0x10;
	private static final int FUNC_OFF = 0x18;

	private DecompInterface decomp;
	private Set<Address> seenFunc = new HashSet<>();
	private Map<Integer, List<Address>> dataIndex;

	private String strAt(Address a) {
		try {
			Memory mem = currentProgram.getMemory();
			StringBuilder sb = new StringBuilder();
			for (int i = 0; i < 512; i++) {
				Address p = a.add(i);
				if (!mem.contains(p)) break;
				int b = mem.getByte(p) & 0xff;
				if (b == 0) break;
				if (b < 0x20 || b > 0x7e) return null;
				sb.append((char) b);
			}
			return sb.toString();
		} catch (Exception e) {
			return null;
		}
	}

	private int intAt(Address a) {
		try {
			return currentProgram.getMemory().getInt(a);
		} catch (Exception e) {
			return 0;
		}
	}

	private Address addr(int p) {
		try {
			return currentProgram.getAddressFactory().getDefaultAddressSpace().getAddress(p & 0xffffffffL);
		} catch (Exception e) {
			return null;
		}
	}

	private void decompile(StringBuilder sb, Function f) {
		if (f == null || seenFunc.contains(f.getEntryPoint())) return;
		seenFunc.add(f.getEntryPoint());
		DecompileResults res = decomp.decompileFunction(f, 240, monitor);
		if (res.decompileCompleted()) {
			sb.append(res.getDecompiledFunction().getC());
			if (sb.length() > 0 && sb.charAt(sb.length() - 1) != '\n') sb.append('\n');
		} else {
			sb.append("     [decompile failed: ").append(res.getErrorMessage()).append("]\n");
		}
	}

	private Map<Integer, List<Address>> indexWritable() {
		Memory mem = currentProgram.getMemory();
		Map<Integer, List<Address>> idx = new HashMap<>();
		for (MemoryBlock b : mem.getBlocks()) {
			// non-executable only: typedescription arrays live in .data and,
			// for const descs, in .rodata; code hits are handled as refs.
			if (b.isExecute() || !b.isInitialized()) continue;
			Address end = b.getEnd();
			for (Address a = b.getStart(); a.compareTo(end) <= 0; a = a.add(4)) {
				int v;
				try {
					v = mem.getInt(a);
				} catch (Exception e) {
					break;
				}
				List<Address> l = idx.get(v);
				if (l == null) {
					l = new ArrayList<>(2);
					idx.put(v, l);
				}
				l.add(a);
			}
		}
		return idx;
	}

	private boolean decodeEntry(StringBuilder sb, Address base, Address ref, int target, String role) {
		if (!currentProgram.getMemory().contains(base)) return false;
		if (intAt(base) < 0 || intAt(base) > 30) return false;
		Address fnA = addr(intAt(base.add(4)));
		if (fnA == null || !currentProgram.getMemory().contains(fnA)) return false;
		String fn = strAt(fnA);
		if (fn == null) return false;
		int extP = intAt(base.add(EXT_OFF));
		Address extA = addr(extP);
		String ext = extP == 0 ? null
			: (extA != null && currentProgram.getMemory().contains(extA) ? strAt(extA) : null);
		if (role.equals("externalName")) {
			if (extP != target || ext == null) return false;
		} else {
			if (extP != 0 && ext == null) return false;
		}
		sb.append("==== data ref at ").append(ref).append(" as ").append(role);
		sb.append(", entry @ ").append(base).append('\n');
		sb.append("     fieldType=").append(intAt(base));
		sb.append(" fieldName=\"").append(fn).append('"');
		sb.append(" externalName=").append(ext == null ? "null" : "\"" + ext + "\"");
		int flags = intAt(base.add(0x0e)) & 0xffff;
		sb.append(String.format(" flags=0x%04x", flags));
		sb.append(String.format(" size=%d", intAt(base.add(0x0c)) & 0xffff));
		int funcP = intAt(base.add(FUNC_OFF));
		if (funcP != 0) {
			Address fA = addr(funcP);
			if (fA != null && currentProgram.getMemory().contains(fA)) {
				sb.append(String.format(" inputFunc@0x%08x", (funcP & 0xffffffffL)));
			}
		}
		sb.append('\n');
		String owner = findDatamap(base);
		if (owner != null) sb.append("     class ").append(owner).append('\n');
		if (funcP != 0) {
			Address fA = addr(funcP);
			if (fA != null && currentProgram.getMemory().contains(fA)) {
				Function f = getFunctionAt(fA);
				if (f == null) f = createFunction(fA, "input_" + fn);
				if (f != null) {
					sb.append("     inputFunc ").append(f.getName(true)).append('\n');
					decompile(sb, f);
				}
			}
		} else {
			// Many retail inputs have their handler written into the slot by a
			// static initializer instead of a relocation. Find that writer and
			// read the handler address out of its operand.
			findSlotWriter(sb, base.add(FUNC_OFF));
		}
		return true;
	}

	// Walk back over the array for a datamap_t whose dataDesc points at its
	// start; retail datamap_t is { dataDesc, dataNumFields, className, ... }.
	private String findDatamap(Address entry) {
		for (int off = 0; off <= 0x8000; off += 0x40) {
			Address vAddr;
			try {
				vAddr = entry.subtract(off);
			} catch (Exception e) {
				return null;
			}
			List<Address> slots = dataIndex.get((int) vAddr.getOffset());
			if (slots == null) continue;
			for (Address d : slots) {
				int n = intAt(d.add(4));
				if (n < 1 || n > 1000 || off > n * 0x40) continue;
				Address cnA = addr(intAt(d.add(8)));
				if (cnA == null || !currentProgram.getMemory().contains(cnA)) continue;
				String cn = strAt(cnA);
				if (cn == null || cn.length() < 2) continue;
				return "\"" + cn + "\" datamap@" + d + " fields=" + n + " array@" + vAddr;
			}
		}
		return null;
	}

	private void findSlotWriter(StringBuilder sb, Address slot) {
		Reference[] rs = getReferencesTo(slot);
		for (Reference r : rs) {
			Address from = r.getFromAddress();
			Function w = getFunctionContaining(from);
			sb.append("     inputFunc slot ").append(slot).append(" written by ref @ ")
				.append(from).append(w != null ? (" in " + w.getName(true)) : "").append('\n');
			Instruction ins = getInstructionAt(from);
			if (ins == null) continue;
			for (int i = 0; i < ins.getNumOperands(); i++) {
				Object[] ops = ins.getOpObjects(i);
				for (Object o : ops) {
					if (o instanceof Address) {
						Address oa = (Address) o;
						if (oa.equals(slot)) continue;
						Function h = getFunctionAt(oa);
						if (h == null) h = getFunctionContaining(oa);
						sb.append("       operand ").append(i).append(" -> ").append(oa);
						if (h != null) sb.append(" = ").append(h.getName(true));
						sb.append('\n');
						if (h != null) decompile(sb, h);
					}
				}
			}
		}
	}

	public void run() throws Exception {
		String[] a = getScriptArgs();
		StringBuilder sb = new StringBuilder();
		decomp = new DecompInterface();
		decomp.openProgram(currentProgram);
		Memory mem = currentProgram.getMemory();
		Address min = currentProgram.getMinAddress();
		print("indexing writable memory...");
		dataIndex = indexWritable();

		for (int i = 1; i < a.length; i++) {
			byte[] pat = (a[i] + "\0").getBytes("latin1");
			Address start = min;
			Set<String> reported = new HashSet<>();
			while (true) {
				Address hit = mem.findBytes(start, pat, null, true, monitor);
				if (hit == null) break;
				start = hit.add(1);
				Address s = hit;
				try {
					while (true) {
						Address prev = s.subtract(1);
						if (!mem.contains(prev) || mem.getByte(prev) == 0) break;
						s = prev;
					}
				} catch (Exception e) {
				}
				int suffix = (int) hit.subtract(s);
				String full = strAt(s);
				String key = s + "/" + suffix;
				if (reported.contains(key)) continue;
				reported.add(key);
				sb.append("## string \"").append(a[i]).append("\" @ ").append(hit);
				sb.append("  merged into \"").append(full).append("\" @ ").append(s);
				sb.append("  suffix +").append(suffix).append('\n');

				// code references (static initializers) to the literal start
				// and to the merged suffix
				Set<Address> codeHits = new LinkedHashSet<>();
				for (Reference r : getReferencesTo(s)) codeHits.add(r.getFromAddress());
				if (suffix > 0) {
					for (Reference r : getReferencesTo(hit)) codeHits.add(r.getFromAddress());
				}
				for (Address from : codeHits) {
					Function f = getFunctionContaining(from);
					if (f == null) continue;
					sb.append("==== code ref from ").append(f.getName(true));
					sb.append(" @ ").append(f.getEntryPoint());
					sb.append(" (ref ").append(from).append(")\n");
					decompile(sb, f);
				}

				// data references: scanned, because Ghidra creates none for
				// the R_386_RELATIVE slots that hold these pointers
				for (int target : new int[] { (int) s.getOffset(), (int) hit.getOffset() }) {
					List<Address> hits = dataIndex.get(target);
					if (hits == null) continue;
					for (Address ref : hits) {
						boolean decoded = decodeEntry(sb, ref.subtract(EXT_OFF), ref, target, "externalName");
						if (!decoded) decoded = decodeEntry(sb, ref.subtract(4), ref, target, "fieldName");
						if (!decoded) {
							sb.append("==== data slot @ ").append(ref);
							sb.append(String.format(" = 0x%08x (entry not decoded)", target)).append('\n');
						}
					}
				}
			}
		}
		decomp.dispose();
		Files.write(Paths.get(a[0]), sb.toString().getBytes());
	}
}
