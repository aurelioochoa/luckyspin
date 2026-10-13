#!/usr/bin/env python3
"""Exercise the real TUI on private saves and pseudo-terminals."""
import fcntl
import os
from pathlib import Path
import pty
import re
import select
import signal
import struct
import subprocess
import tempfile
import termios
import time
import unicodedata

ROOT = Path(__file__).resolve().parents[1]
ANSI = re.compile(r'\x1b\[[0-?]*[ -/]*[@-~]')


def read_until(master, marker, timeout=5):
    data = b''
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        if select.select([master], [], [], .1)[0]:
            try:
                data += os.read(master, 65536)
            except OSError:
                break
            if marker in data:
                return data
    raise AssertionError(f'missing {marker!r} in {data[-1000:]!r}')


def width(text):
    return sum(0 if unicodedata.combining(ch) else
               2 if unicodedata.east_asian_width(ch) in ('W', 'F') else 1 for ch in text)


with tempfile.TemporaryDirectory(prefix='luckyspin-tui-') as save_root:
    for cols, rows in [(100, 30), (80, 24), (44, 24), (32, 18), (100, 18)]:
        master, slave = pty.openpty()
        fcntl.ioctl(slave, termios.TIOCSWINSZ, struct.pack('HHHH', rows, cols, 0, 0))
        original = termios.tcgetattr(slave)
        env = dict(os.environ, XDG_DATA_HOME=f'{save_root}/{cols}-{rows}', NO_COLOR='1')
        process = subprocess.Popen([str(ROOT / 'gambling_game')], stdin=slave, stdout=slave,
                                   stderr=slave, cwd=save_root, env=env)
        try:
            data = read_until(master, b'\x1b[J')
            lines = ANSI.sub('', data.decode('utf-8')).replace('\r', '').strip('\n').split('\n')
            assert 'LUCKY SPIN' in '\n'.join(lines)
            assert len(lines) < rows, (rows, len(lines))
            assert all(width(line) < cols for line in lines), (cols, lines)
            assert not re.search(rb'\x1b\[[0-9;]+m', data), 'NO_COLOR must suppress styling'
            assert b'0  Spin the Wheel' in data
            if cols >= 76 and rows >= 23:
                assert b'YOUR EMPIRE' in data and b'About & help' in data
            os.write(master, b'\x1b[A')
            data = read_until(master, b'\x1b[J')
            assert b'Quit Game' in data, 'Up arrow wraps to quit'
            os.write(master, b'\x1b[B')
            data = read_until(master, b'\x1b[J')
            assert b'Spin the Wheel' in data, 'Down arrow wraps to first table'
            os.write(master, b'?')
            read_until(master, b'ABOUT LUCKY SPIN')
            os.write(master, b'q')
            read_until(master, b'\x1b[J')
            for shortcut, title in [(b'5', b'BUILD THE HOUSE'), (b'6', b'STACK THE ODDS'), (b'7', b'SET YOUR STAKE'), (b'8', b'A FRESH START'), (b'9', b'CHOOSE YOUR SPECIALTY')]:
                os.write(master, shortcut)
                sub = read_until(master, b'\x1b[J')
                assert title in sub, (shortcut, sub)
                sublines = ANSI.sub('', sub.decode('utf-8')).replace('\r', '').strip('\n').split('\n')
                assert len(sublines) < rows, (title, rows, len(sublines))
                assert all(width(line) < cols for line in sublines), (title, cols, sublines)
                os.write(master, b'q')
                read_until(master, b'\x1b[J')
            os.write(master, b'5')
            read_until(master, b'\x1b[J')
            os.write(master, b'1')
            sub = read_until(master, b'\x1b[J')
            assert b'Bought' in sub, sub
            os.write(master, b'e')
            assert b'Buy quantity x10' in read_until(master, b'\x1b[J')
            os.write(master, b'q')
            read_until(master, b'\x1b[J')
            os.write(master, b'v')
            read_until(master, b'Game saved')
            assert list(Path(env['XDG_DATA_HOME']).rglob('lucky_spin_save.txt'))
            read_until(master, b'\x1b[J')
            os.write(master, b'q')
            process.wait(timeout=5)
            assert process.returncode == 0
            assert termios.tcgetattr(slave) == original, 'Terminal settings restored'
            print(f'TUI {cols}x{rows}: layout, arrows, help, save, exit passed')
        finally:
            if process.poll() is None:
                process.send_signal(signal.SIGTERM)
                process.wait(timeout=5)
            os.close(master)
            os.close(slave)
    for args in [['--port', 'abc'], ['--port=65536'], ['--unknown'], []]:
        p = subprocess.run([str(ROOT / 'gambling_game'), *args], stdin=subprocess.DEVNULL,
                           capture_output=True, cwd=save_root)
        assert p.returncode == 2, args
    assert subprocess.run([str(ROOT / 'gambling_game'), '--help'], capture_output=True).returncode == 0
print('TUI verification passed')
