// The Node lane of the WebAssembly product (RFC 0029 W1/W2): runs a built
// hl2_launcher.js with a game tree mounted from the host through NODEFS.
//
//   node --experimental-wasm-jspi tools/web/run_node.cjs <hl2_launcher.js> <game tree> [engine args...]
//
// The pinned SDK's Node (dependencies/emsdk-*/node) runs it; the build uses
// JSPI, which that Node has behind the flag.
// The tree is mounted at /game and is the working directory, as the desktop
// product runs from its runtime directory. Exits with the program's status.
'use strict';
const path = require('path');

const [script, tree, ...args] = process.argv.slice(2);
if (!script || !tree) {
	console.error('usage: run_node.cjs <hl2_launcher.js> <game tree> [engine args...]');
	process.exit(2);
}

// SDL's gamepad backend samples navigator.getGamepads (Node 20 has none).
globalThis.navigator ??= { getGamepads: () => [] };

const createSourceEngine = require(path.resolve(script));
createSourceEngine({
	// The engine finds its base directory from the program's path.
	thisProgram: '/game/hl2_launcher',
	preRun: [(module) => {
		// No DOM in Node: SDL's dummy video and audio drivers.
		module.ENV.SDL_VIDEO_DRIVER = 'dummy';
		module.ENV.SDL_AUDIO_DRIVER = 'dummy';
		// Runtime trees link their content by absolute paths: the host
		// directories they point into appear at the same paths.
		for (const dir of ['/home', '/tmp']) {
			try { module.FS.mkdir(dir); } catch (error) { /* exists */ }
			module.FS.mount(module.NODEFS, { root: dir }, dir);
		}
		module.FS.mkdir('/game');
		module.FS.mount(module.NODEFS, { root: path.resolve(tree) }, '/game');
		module.FS.chdir('/game');
	}],
	onExit(status) {
		process.exitCode = status;
	},
}).then(async (module) => {
	// With JSPI, main returns a promise; exit() ends it with an ExitStatus.
	try {
		// RUN_NODE_EXIT_AFTER=<s>: a clean exit after that long, so --cpu-prof
		// writes its profile even when the engine never returns (the thread is
		// free while the engine is suspended).
		if (process.env.RUN_NODE_EXIT_AFTER)
			setTimeout(() => process.exit(124), Number(process.env.RUN_NODE_EXIT_AFTER) * 1000).unref();
		await module.callMain(args);
	} catch (error) {
		if (!(error && error.name === 'ExitStatus'))
			throw error;
	}
});
