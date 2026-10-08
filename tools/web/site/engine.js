// The browser page of the WebAssembly product (RFC 0029): mounts the game
// tree the server lists (tools/web/serve.py) and runs the engine on this
// thread with JSPI; its frames show on #canvas through the WebGPU adapter.
//
// Query: args=<engine arguments, space separated>; harness=1 posts the
// console to /log, each capture the engine writes to /file/<name> and the exit
// status to /exit (tools/web/browser_lane.py).
'use strict';

const query = new URLSearchParams(location.search);
const harness = query.get('harness') === '1';
const args = (query.get('args') || '-game portal -novid -noip -w 1280 -h 720 +map testchmb_a_01')
	.split(' ').filter((a) => a.length);
const status = document.getElementById('status');
const canvas = document.getElementById('canvas');
const CHUNK = 1 << 20;

// The console, to the page and (harness) the server.
const lines = [];
let sent = 0;
// The engine holds this thread for long stretches (loading reads content
// synchronously), so a timer alone would hold the console back until it
// yields: say() posts synchronously too, at most every 250 ms.
let flushed = 0;
const started = performance.now();
function say(text) {
	lines.push(((performance.now() - started) / 1000).toFixed(1).padStart(7) + ' ' + text);
	if (!harness)
		console.log(text);
	else if (performance.now() - flushed > 250)
		flush(true);
}
function flush(sync) {
	flushed = performance.now();
	if (harness && sent < lines.length) {
		const chunk = lines.slice(sent);
		sent = lines.length;
		const body = chunk.join('\n') + '\n';
		if (sync) {
			const xhr = new XMLHttpRequest();
			xhr.open('POST', '/log', false);
			xhr.send(body);
		} else {
			fetch('/log', { method: 'POST', body, keepalive: true });
		}
	}
}
setInterval(() => flush(false), 250);
// While the engine is suspended this thread is free: a heartbeat shows the
// page is alive, whether the browser counts it visible (a hidden page's
// timers, and so the engine's yields, are throttled) and how far the
// engine's own frames have got.
let beats = 0;
if (harness)
	setInterval(() => say('page: alive ' + (++beats * 5) + ' s, ' + document.visibilityState +
                      (document.hasFocus() ? ', focused' : '')), 5000);
function finish(code) {
	flush();
	if (harness)
		fetch('/exit', { method: 'POST', body: String(code), keepalive: true });
	status.textContent = 'Exited (' + code + ')';
}
addEventListener('error', (e) => { say('[error] ' + e.message); finish(135); });
addEventListener('unhandledrejection', (e) => { say('[rejection] ' + (e.reason && e.reason.stack || e.reason)); finish(135); });

// A byte range of a content file, synchronously: the browser allows a
// synchronous request on this thread only as text, so the bytes come as
// x-user-defined characters (each char code's low byte). The decoder sniffs a
// byte-order mark ahead of that charset (a UTF-16 file would come back as
// half as many characters), so the server puts one zero byte first (pad=1)
// and the text never starts with one.
function fetchRange(url, start, end) {
	const xhr = new XMLHttpRequest();
	xhr.open('GET', url + '?pad=1', false);
	xhr.setRequestHeader('Range', 'bytes=' + start + '-' + end);
	xhr.overrideMimeType('text/plain; charset=x-user-defined');
	xhr.send(null);
	if (xhr.status !== 206 && xhr.status !== 200)
		throw new Error('content ' + url + ': HTTP ' + xhr.status);
	const text = xhr.responseText;
	if (text.length !== end - start + 2)
		throw new Error('content ' + url + ': ' + (text.length - 1) + ' bytes for range ' +
		                start + '-' + end);
	const bytes = new Uint8Array(text.length - 1);
	for (let i = 1; i < text.length; ++i)
		bytes[i - 1] = text.charCodeAt(i) & 0xff;
	return bytes;
}

// A read-only file whose bytes are read from the server in chunks as the
// engine reads them, and kept.
function lazyFile(module, parent, name, url, size) {
	const FS = module.FS;
	const chunks = [];
	const chunk = (n) => {
		if (!chunks[n]) {
			const start = n * CHUNK;
			chunks[n] = fetchRange(url, start, Math.min(start + CHUNK, size) - 1);
		}
		return chunks[n];
	};
	const node = FS.createFile(parent, name, {}, true, false);
	node.contents = { length: size };
	Object.defineProperty(node, 'usedBytes', { get: () => size });
	const copy = (buffer, offset, length, position) => {
		if (position >= size)
			return 0;
		const total = Math.min(size - position, length);
		let done = 0;
		while (done < total) {
			const at = position + done;
			const data = chunk(Math.floor(at / CHUNK));
			const from = at % CHUNK;
			const n = Math.min(total - done, data.length - from);
			if (!(n > 0))
				throw new Error('content ' + url + ': chunk at ' + at + ' holds ' + data.length +
				                ' bytes (file ' + size + ', read ' + position + '+' + length + ')');
			buffer.set(data.subarray(from, from + n), offset + done);
			done += n;
		}
		return total;
	};
	const ops = Object.assign({}, node.stream_ops);
	ops.read = (stream, buffer, offset, length, position) => copy(buffer, offset, length, position);
	ops.mmap = (stream, length, position) => {
		const ptr = module._malloc(length);
		if (!ptr)
			throw new FS.ErrnoError(48); // ENOMEM
		copy(module.HEAPU8, ptr, length, position);
		return { ptr, allocated: true };
	};
	node.stream_ops = ops;
	return node;
}

async function mount(module) {
	const FS = module.FS;
	status.textContent = 'Listing content…';
	const listing = await (await fetch('/content/manifest.json')).json();
	const made = new Set(['/']);
	const dir = (path) => {
		const parts = path.split('/');
		let at = '';
		for (const part of parts) {
			at += '/' + part;
			if (!made.has(at)) {
				try { FS.mkdir(at); } catch (e) { /* exists */ }
				made.add(at);
			}
		}
	};
	dir('game');
	const eager = [];
	for (const [path, size] of listing.files) {
		const slash = path.lastIndexOf('/');
		const parent = '/game' + (slash >= 0 ? '/' + path.slice(0, slash) : '');
		if (slash >= 0)
			dir('game/' + path.slice(0, slash));
		const name = path.slice(slash + 1);
		const url = '/content/' + path.split('/').map(encodeURIComponent).join('/');
		if (size <= listing.eager)
			eager.push([parent + '/' + name, url]);
		else
			lazyFile(module, parent, name, url, size);
	}
	// Small files whole, a few at a time.
	let next = 0, loaded = 0;
	const worker = async () => {
		while (next < eager.length) {
			const [path, url] = eager[next++];
			const data = new Uint8Array(await (await fetch(url)).arrayBuffer());
			FS.writeFile(path, data);
			if (++loaded % 200 === 0)
				status.textContent = 'Loading content… ' + loaded + ' / ' + eager.length;
		}
	};
	await Promise.all(Array.from({ length: 16 }, worker));
	FS.chdir('/game');
	say('page: content mounted: ' + listing.files.length + ' files (' + eager.length + ' loaded whole)');
}

// The engine's captures (-pica_capture_path) go back to the harness.
function handBack(module, text) {
	const match = /^pica: capture (\S+) ok/.exec(text);
	if (!harness || !match)
		return;
	try {
		const data = module.FS.readFile(match[1]);
		fetch('/file/' + match[1].split('/').pop(), { method: 'POST', body: data });
	} catch (e) {
		say('page: capture ' + match[1] + ' unreadable: ' + e);
	}
}

(async () => {
	if (!navigator.gpu) {
		say('[error] this browser has no navigator.gpu');
		return finish(136);
	}
	if (!crossOriginIsolated) {
		say('[error] the page is not cross-origin isolated (no SharedArrayBuffer)');
		return finish(137);
	}
	const script = document.createElement('script');
	script.src = '/engine/hl2_launcher.js';
	await new Promise((resolve, reject) => {
		script.onload = resolve;
		script.onerror = () => reject(new Error('no engine build at /engine/hl2_launcher.js'));
		document.head.appendChild(script);
	});
	let module = null;
	module = await createSourceEngine({
		canvas,
		thisProgram: '/game/hl2_launcher',
		locateFile: (path) => '/engine/' + path,
		print: (text) => { say(text); if (module) handBack(module, text); },
		printErr: (text) => say('[stderr] ' + text),
		onExit: (code) => finish(code),
		onAbort: (what) => { say('[abort] ' + what); finish(134); },
		preRun: [(m) => {
			m.addRunDependency('content');
			mount(m).then(() => m.removeRunDependency('content'),
				(e) => { say('[error] content: ' + e); finish(138); });
		}],
	});
	status.textContent = '';
	canvas.focus();
	try {
		await module.callMain(args);
	} catch (e) {
		if (!(e && e.name === 'ExitStatus'))
			throw e;
	}
})();
