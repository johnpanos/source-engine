#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Typed Python client for the engine debug API (source.debug.v1, JSON-RPC 2.0).

Methods, parameter and result types come from the generated bindings of
debugapi/proto (tools/debugapi/generated, pinned protoc); nothing here restates
the schema. Framing and transport are pluggable like the engine's:

    from client import DebugApiClient
    with DebugApiClient.connect_unix("/run/user/1000/source-debugapi-123.sock") as api:
        print(api.call("status"))
        api.call("waitFor", condition="WAIT_CONDITION_CLIENT_ACTIVE", timeoutMs=60000)
        shot = api.call("screenshot", mode="SCREENSHOT_MODE_INLINE_JPEG")

Requires the pinned runtime in tools/debugapi/requirements.txt.
"""

import collections
import json
from pathlib import Path
import select
import socket
import sys
import time

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE / "generated"))

from google.protobuf import json_format  # noqa: E402
from google.protobuf import message_factory  # noqa: E402
from source.debug.v1 import debug_api_pb2 as pb  # noqa: E402

SERVICE = pb.DESCRIPTOR.services_by_name["DebugApi"]
NOTIFICATIONS = pb.DESCRIPTOR.services_by_name["DebugApiNotifications"]


def lower_camel(name):
    return name[:1].lower() + name[1:]


METHODS = {lower_camel(m.name): m for m in SERVICE.methods}
NOTIFICATION_METHODS = {lower_camel(m.name): m for m in NOTIFICATIONS.methods}


def message_class(descriptor):
    return message_factory.GetMessageClass(descriptor)


def to_json_dict(message):
    """The canonical proto3 JSON the engine emits (every field present)."""
    return json_format.MessageToDict(message, always_print_fields_with_no_presence=True)


class RpcError(Exception):
    def __init__(self, error):
        self.code = error.get("code")
        self.message = error.get("message", "")
        self.error_code = (error.get("data") or {}).get("errorCode", "")
        super().__init__("%s (%s): %s" % (self.error_code or "error", self.code, self.message))


class ProtocolError(Exception):
    pass


# ---------------------------------------------------------------------------
# Framing (mirrors public/debugapi/debugapi_framing.h)
# ---------------------------------------------------------------------------

class NewlineFraming:
    name = "newline"

    def encode(self, payload):
        data = payload.encode("utf-8")
        if b"\n" in data:
            raise ValueError("newline framing cannot carry a payload containing a newline")
        return data + b"\n"

    def decoder(self):
        return _NewlineDecoder()


class _NewlineDecoder:
    def __init__(self):
        self.buffer = b""

    def feed(self, data):
        self.buffer += data
        frames = []
        while b"\n" in self.buffer:
            line, self.buffer = self.buffer.split(b"\n", 1)
            line = line[:-1] if line.endswith(b"\r") else line
            if line.strip():
                frames.append(line.decode("utf-8"))
        return frames


class ContentLengthFraming:
    name = "content-length"

    def encode(self, payload):
        data = payload.encode("utf-8")
        return b"Content-Length: %d\r\n\r\n" % len(data) + data

    def decoder(self):
        return _ContentLengthDecoder()


class _ContentLengthDecoder:
    def __init__(self):
        self.buffer = b""
        self.length = None

    def feed(self, data):
        self.buffer += data
        frames = []
        while True:
            if self.length is None:
                end = self.buffer.find(b"\r\n\r\n")
                if end < 0:
                    return frames
                headers = self.buffer[:end].decode("ascii").split("\r\n")
                self.buffer = self.buffer[end + 4:]
                lengths = [line.split(":", 1)[1].strip() for line in headers
                           if line.lower().startswith("content-length:")]
                if len(lengths) != 1 or not lengths[0].isdigit():
                    raise ProtocolError("invalid Content-Length header block")
                self.length = int(lengths[0])
            if len(self.buffer) < self.length:
                return frames
            frames.append(self.buffer[:self.length].decode("utf-8"))
            self.buffer = self.buffer[self.length:]
            self.length = None


FRAMINGS = {f.name: f for f in (NewlineFraming, ContentLengthFraming)}


# ---------------------------------------------------------------------------
# Transports
# ---------------------------------------------------------------------------

class SocketTransport:
    """A connected stream socket (Unix, or any socket.socket)."""

    def __init__(self, sock):
        self.sock = sock

    @classmethod
    def unix(cls, path, timeout=10.0):
        deadline = time.monotonic() + timeout
        while True:
            sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
            try:
                sock.connect(str(path))
                return cls(sock)
            except (FileNotFoundError, ConnectionRefusedError):
                sock.close()
                if time.monotonic() >= deadline:
                    raise
                time.sleep(0.05)

    def send(self, data):
        self.sock.sendall(data)

    def receive(self, timeout):
        """Bytes available within `timeout` seconds; b"" at end of stream;
        None when nothing arrived."""
        ready, _, _ = select.select([self.sock], [], [], max(0.0, timeout))
        if not ready:
            return None
        return self.sock.recv(1 << 20)

    def close(self):
        self.sock.close()


# ---------------------------------------------------------------------------
# Client
# ---------------------------------------------------------------------------

class DebugApiClient:
    def __init__(self, transport, framing=None):
        self.transport = transport
        self.framing = framing or NewlineFraming()
        self.decoder = self.framing.decoder()
        self.next_id = 1
        self.responses = {}
        self.methods_by_id = {}
        self.notifications = collections.deque()
        self.closed = False

    @classmethod
    def connect_unix(cls, path, framing="newline", timeout=10.0):
        return cls(SocketTransport.unix(path, timeout), FRAMINGS[framing]())

    def __enter__(self):
        return self

    def __exit__(self, *exc):
        self.close()

    def close(self):
        if not self.closed:
            self.closed = True
            self.transport.close()

    # -- requests -----------------------------------------------------------

    def request(self, method, params=None, **fields):
        """Sends a request; returns its id. `params` may be a message or dict."""
        descriptor = self._method(method)
        request_id = self.next_id
        self.next_id += 1
        self._send(self._envelope(descriptor, method, params, fields, request_id))
        return request_id

    def notify(self, method, params=None, **fields):
        """Sends a JSON-RPC notification (no id; the server never replies)."""
        self._send(self._envelope(self._method(method), method, params, fields, None))

    def result(self, request_id, timeout=30.0):
        """Waits for a response; returns the typed result or raises RpcError."""
        deadline = time.monotonic() + timeout
        while request_id not in self.responses:
            if not self._pump(deadline - time.monotonic()):
                raise TimeoutError("no response to request %d within %.1fs" % (request_id, timeout))
        response = self.responses.pop(request_id)
        if "error" in response:
            raise RpcError(response["error"])
        return self._result_message(response["_method"], response["result"])

    def call(self, method, params=None, timeout=30.0, **fields):
        return self.result(self.request(method, params, **fields), timeout)

    def raw(self, payload, timeout=5.0):
        """Sends arbitrary JSON text; returns every message received until quiet."""
        self._send(payload)
        messages = []
        deadline = time.monotonic() + timeout
        while self._pump(deadline - time.monotonic(), messages):
            pass
        return messages

    def next_notification(self, timeout=5.0):
        deadline = time.monotonic() + timeout
        while not self.notifications:
            if not self._pump(deadline - time.monotonic()):
                return None
        return self.notifications.popleft()

    def wait_closed(self, timeout=30.0):
        """True once the server closes the connection."""
        deadline = time.monotonic() + timeout
        while not self.closed and time.monotonic() < deadline:
            self._pump(deadline - time.monotonic())
        return self.closed

    # -- internals ----------------------------------------------------------

    def _method(self, method):
        if method not in METHODS:
            raise ValueError("unknown method %r (known: %s)" % (method, ", ".join(sorted(METHODS))))
        return METHODS[method]

    def _envelope(self, descriptor, method, params, fields, request_id):
        cls = message_class(descriptor.input_type)
        if params is None:
            # Keyword fields use JSON names or proto names, enums by name.
            params = json_format.ParseDict(fields, cls())
        elif isinstance(params, dict):
            params = json_format.ParseDict(params, cls())
        elif fields:
            raise ValueError("pass either a params message/dict or keyword fields")
        envelope = {"jsonrpc": "2.0"}
        if request_id is not None:
            envelope["id"] = request_id
        envelope["method"] = method
        envelope["params"] = json_format.MessageToDict(params)
        if request_id is not None:
            self.methods_by_id[request_id] = method
        return json.dumps(envelope, separators=(",", ":"), ensure_ascii=False)

    def _result_message(self, method, result):
        cls = message_class(METHODS[method].output_type)
        return json_format.ParseDict(result, cls())

    def _send(self, payload):
        if self.closed:
            raise ProtocolError("connection closed")
        self.transport.send(self.framing.encode(payload))

    def _pump(self, timeout, sink=None):
        """Reads once; False when nothing arrived before the timeout or the
        connection closed."""
        if self.closed:
            return False
        data = self.transport.receive(timeout)
        if data is None:
            return False
        if data == b"":
            self.closed = True
            return False
        for frame in self.decoder.feed(data):
            message = json.loads(frame)
            if sink is not None:
                sink.append(message)
                continue
            self._dispatch(message)
        return True

    def _dispatch(self, message):
        if isinstance(message, list):
            for member in message:
                self._dispatch(member)
            return
        if "method" in message and "id" not in message:
            descriptor = NOTIFICATION_METHODS.get(message["method"])
            if descriptor is None:
                raise ProtocolError("unknown notification %r" % message["method"])
            params = json_format.ParseDict(message.get("params", {}),
                                           message_class(descriptor.input_type)())
            self.notifications.append((message["method"], params))
            return
        request_id = message.get("id")
        method = self.methods_by_id.pop(request_id, None)
        if method is None:
            if "error" in message:
                raise RpcError(message["error"])
            raise ProtocolError("response for unknown id %r" % (request_id,))
        message["_method"] = method
        self.responses[request_id] = message
