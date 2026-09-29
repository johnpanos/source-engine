# encoding: utf-8
# output_lock.py -- one writing Waf command per output directory at a time
#
# Several sessions build in one checkout, each in its own output directory
# (`build-*`, `out`), and nothing stopped two `./waf build` (or configure,
# install, clean) runs from writing the same directory at once: half-written
# objects, a clobbered c4che, a reconfigure under a running build. AGENTS.md
# asks for clean isolated output directories and repeatable builds, and RFC
# 0005 for evidence a concurrent writer cannot corrupt.
#
# install() wraps waflib.Scripting.run_command. Before the first command of an
# invocation that may write the output directory, it takes an exclusive
# flock(2) on <out>/.lock-output-dir and keeps it until the process exits.
# The kernel drops the lock when the holder dies, so there is no stale-lock
# cleanup. The descriptor is not inherited (PEP 446), so compilers and daemons
# a build starts never keep it.
#
# - Keyed by the output directory's real path: different trees never block
#   each other.
# - Contention waits. One line names the directory and the holder (its PID and
#   command, recorded in the lock file), then a reminder every
#   WAF_OUTPUT_LOCK_REMINDER seconds (default 60).
# - WAF_OUTPUT_LOCK=0 skips the lock (emergencies only).
# - WAF_OUTPUT_LOCK_TIMEOUT=<seconds> fails the command with an error instead
#   of waiting longer.
# - Read-only commands (list, dist, distcheck, init, shutdown, options; --help
#   exits before any command) never take it.
# - Nested Waf: while a process holds the lock, WAF_OUTPUT_LOCK_HELD in its
#   environment names the directory and its PID. A child Waf on that directory
#   whose ancestor is the recorded holder runs under the parent's lock instead
#   of waiting for it (which would deadlock).
# - The file name starts with `.lock`, which `waf clean` never deletes.
#
# The lock functions below do not import waflib, so the self-test
# (tools/quality/tests/test_waf_output_lock.py) can also drive them directly.

import errno
import os
import sys
import time

try:
	import fcntl
except ImportError:
	# Windows has no flock(2); Waf runs there without the output lock.
	fcntl = None

LOCK_NAME = '.lock-output-dir'
ENV_DISABLE = 'WAF_OUTPUT_LOCK'
ENV_TIMEOUT = 'WAF_OUTPUT_LOCK_TIMEOUT'
ENV_REMINDER = 'WAF_OUTPUT_LOCK_REMINDER'
ENV_HELD = 'WAF_OUTPUT_LOCK_HELD'

# Commands that never write the output directory.
READ_ONLY_COMMANDS = frozenset(('init', 'shutdown', 'options', 'list', 'dist', 'distcheck'))

POLL_SECONDS = 0.2
DEFAULT_REMINDER_SECONDS = 60.0

# Real path -> file descriptor of every output lock this process holds.
_held = {}


class LockTimeout(Exception):
	pass


def enabled(environ=None):
	if fcntl is None:
		return False
	value = (environ if environ is not None else os.environ).get(ENV_DISABLE, '')
	return value.strip().lower() not in ('0', 'off', 'no', 'false')


def _seconds(name, default):
	value = os.environ.get(name, '').strip()
	if not value:
		return default
	try:
		return float(value)
	except ValueError:
		raise ValueError('%s=%r is not a number of seconds' % (name, value))


def read_holder(directory):
	"""The holder line the current owner wrote, or '' when unknown."""
	try:
		with open(os.path.join(directory, LOCK_NAME)) as handle:
			return handle.read().strip()
	except (IOError, OSError):
		return ''


def _holder_pid(line):
	for field in line.split():
		if field.startswith('pid='):
			try:
				return int(field[4:])
			except ValueError:
				return None
	return None


def _held_markers(environ=None):
	"""WAF_OUTPUT_LOCK_HELD as {real path: pid}."""
	markers = {}
	value = (environ if environ is not None else os.environ).get(ENV_HELD, '')
	for entry in value.split(os.pathsep):
		path, separator, pid = entry.rpartition('=')
		if separator and path:
			try:
				markers[path] = int(pid)
			except ValueError:
				pass
	return markers


def _set_marker(directory, pid):
	markers = _held_markers()
	markers[directory] = pid
	os.environ[ENV_HELD] = os.pathsep.join('%s=%d' % item for item in sorted(markers.items()))


def _parent_pid(pid):
	"""Parent of pid from /proc, or None where /proc is unavailable."""
	try:
		with open('/proc/%d/stat' % pid) as handle:
			stat = handle.read()
	except (IOError, OSError):
		return None
	# The command name is parenthesised and may contain spaces.
	fields = stat[stat.rfind(')') + 2:].split()
	return int(fields[1]) if len(fields) > 1 else None


def _alive(pid):
	try:
		os.kill(pid, 0)
	except OSError as error:
		return error.errno == errno.EPERM
	return True


def _is_ancestor(pid):
	"""True when pid is an ancestor of this process (alive, where /proc is missing)."""
	if not os.path.isdir('/proc/self'):
		return _alive(pid)
	current = os.getppid()
	for _ in range(256):
		if current == pid:
			return True
		if current in (None, 0, 1):
			return False
		current = _parent_pid(current)
	return False


def _describe(command):
	argv = ' '.join([os.path.basename(sys.argv[0])] + sys.argv[1:]) if sys.argv else '?'
	if len(argv) > 200:
		argv = argv[:197] + '...'
	return 'pid=%d command=%s since=%s cmd: %s' % (
		os.getpid(), command, time.strftime('%Y-%m-%dT%H:%M:%S'), argv)


def _same_file(fd, path):
	try:
		return os.fstat(fd).st_ino == os.stat(path).st_ino
	except OSError:
		return False


def acquire(directory, command='?', create=False, log=None):
	"""Take the exclusive lock of an output directory, waiting while another
	process holds it. Returns 'held', 'acquired', 'nested', 'disabled' or
	'skipped' (the directory does not exist and create is False)."""
	log = log or (lambda message: sys.stderr.write(message + '\n'))
	directory = os.path.realpath(directory)
	if directory in _held:
		return 'held'
	if not enabled():
		return 'disabled'
	if create:
		try:
			os.makedirs(directory)
		except OSError as error:
			if error.errno != errno.EEXIST:
				raise
	elif not os.path.isdir(directory):
		return 'skipped'

	path = os.path.join(directory, LOCK_NAME)
	timeout = _seconds(ENV_TIMEOUT, None)
	reminder = _seconds(ENV_REMINDER, DEFAULT_REMINDER_SECONDS)
	marker = _held_markers().get(directory)
	start = time.time()
	next_reminder = None
	while True:
		try:
			fd = os.open(path, os.O_RDWR | os.O_CREAT, 0o666)
		except OSError as error:
			if not os.path.isdir(directory):
				return 'skipped'
			log('Waf: cannot open %s (%s); continuing without the output lock' % (path, error))
			return 'disabled'
		try:
			fcntl.flock(fd, fcntl.LOCK_EX | fcntl.LOCK_NB)
		except (IOError, OSError) as error:
			os.close(fd)
			if error.errno not in (errno.EAGAIN, errno.EACCES, errno.EWOULDBLOCK):
				raise
			holder = read_holder(directory)
			if marker is not None and _holder_pid(holder) == marker and _is_ancestor(marker):
				# A Waf started by the holder: run under its lock.
				return 'nested'
			now = time.time()
			if next_reminder is None:
				log('Waf: waiting for output directory %s, locked by another Waf (%s)'
					% (directory, holder or 'holder unknown'))
				next_reminder = now + reminder
			elif now >= next_reminder:
				log('Waf: still waiting for output directory %s after %ds (%s)'
					% (directory, now - start, holder or 'holder unknown'))
				next_reminder = now + reminder
			if timeout is not None and now - start >= timeout:
				raise LockTimeout('gave up after %gs (%s) waiting for output directory %s, '
					'locked by another Waf (%s)' % (timeout, ENV_TIMEOUT, directory,
					holder or 'holder unknown'))
			time.sleep(POLL_SECONDS)
			continue
		# The holder may have deleted or replaced the file (distclean) while
		# this process waited on the old one; lock the file the path names now.
		if not _same_file(fd, path):
			os.close(fd)
			continue
		os.ftruncate(fd, 0)
		os.write(fd, (_describe(command) + '\n').encode('utf-8', 'replace'))
		_held[directory] = fd
		_set_marker(directory, os.getpid())
		if next_reminder is not None:
			log('Waf: output directory %s is free after %ds' % (directory, time.time() - start))
		return 'acquired'


def release_all():
	"""Drop every lock this process holds (process exit does the same)."""
	markers = _held_markers()
	for directory, fd in list(_held.items()):
		os.close(fd)
		del _held[directory]
		markers.pop(directory, None)
	if markers:
		os.environ[ENV_HELD] = os.pathsep.join('%s=%d' % item for item in sorted(markers.items()))
	else:
		os.environ.pop(ENV_HELD, None)


def output_directory(cmd_name):
	"""The output directory a Waf command writes, or None when it has none yet."""
	from waflib import Configure, Context, Options
	for cls in Context.classes:
		if getattr(cls, 'cmd', None) == cmd_name:
			break
	else:
		cls = None
	if cls is not None and issubclass(cls, Configure.ConfigurationContext):
		# ConfigurationContext.init_dirs, without its side effects.
		out = (Options.options.out or getattr(Context.g_module, Context.OUT, None)
			or Options.lockfile.replace('.lock-waf_%s_' % sys.platform, '').replace('.lock-waf', ''))
		return os.path.realpath(out), True
	if Context.out_dir:
		return os.path.realpath(Context.out_dir), False
	# distclean and similar commands read the lock file themselves.
	from waflib import ConfigSet
	env = ConfigSet.ConfigSet()
	try:
		env.load(os.path.join(Context.run_dir or os.getcwd(), Options.lockfile))
	except EnvironmentError:
		return None, False
	return (os.path.realpath(env.out_dir), False) if env.out_dir else (None, False)


def install():
	"""Wrap waflib.Scripting.run_command so writing commands take the lock."""
	from waflib import Errors, Logs, Scripting
	if getattr(Scripting.run_command, 'output_lock', False):
		return
	run_command = Scripting.run_command

	def locked_run_command(cmd_name):
		if cmd_name not in READ_ONLY_COMMANDS and enabled():
			directory, create = output_directory(cmd_name)
			if directory:
				try:
					acquire(directory, cmd_name, create=create, log=Logs.warn)
				except (LockTimeout, ValueError) as error:
					raise Errors.WafError(str(error))
		return run_command(cmd_name)

	locked_run_command.output_lock = True
	Scripting.run_command = locked_run_command
