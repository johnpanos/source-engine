//@category Portal2
import java.nio.file.*;
import ghidra.app.script.GhidraScript;
// args: <out>  -- writes the program's image base, address range and section list.
public class ProgramInfo extends GhidraScript {
	public void run() throws Exception {
		StringBuilder sb = new StringBuilder();
		sb.append("program: ").append(currentProgram.getName()).append("\n");
		sb.append("executable: ").append(currentProgram.getExecutablePath()).append("\n");
		sb.append("image base: ").append(currentProgram.getImageBase()).append("\n");
		sb.append("min: ").append(currentProgram.getMinAddress());
		sb.append("  max: ").append(currentProgram.getMaxAddress()).append("\n");
		sb.append("language: ").append(currentProgram.getLanguageID()).append("\n");
		sb.append("sha256: ").append(currentProgram.getExecutableSHA256()).append("\n");
		sb.append("sections:\n");
		for (ghidra.program.model.mem.MemoryBlock b : currentProgram.getMemory().getBlocks()) {
			sb.append(String.format("  %-16s %s - %s  %s%n", b.getName(), b.getStart(), b.getEnd(),
				b.isExecute() ? "X" : b.isWrite() ? "W" : "R"));
		}
		Files.write(Paths.get(getScriptArgs()[0]), sb.toString().getBytes());
	}
}
