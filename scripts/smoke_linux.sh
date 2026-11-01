#!/usr/bin/env bash
# Run inside xvfb-run. Verify a visible launcher, not just a live process.
set -euo pipefail
pack=$(realpath "${1:?package directory required}")
evidence=$(realpath "${2:?evidence directory required}")
cd "$pack"
export SDL_AUDIODRIVER=dummy
export LIBGL_ALWAYS_SOFTWARE=1
./run.sh > "$evidence/launcher.log" 2>&1 &
child=$!
trap 'kill "$child" 2>/dev/null || true' EXIT
window=""
for _ in {1..30}; do
  if ! kill -0 "$child" 2>/dev/null; then
    cat "$evidence/launcher.log"
    echo 'Launcher exited before showing its window' >&2
    exit 1
  fi
  window=$(xdotool search --onlyvisible --pid "$child" 2>/dev/null | head -1 || true)
  [[ -z "$window" ]] || break
  sleep 1
done
test -n "$window"
sleep 3
kill -0 "$child"
xwininfo -id "$window" > "$evidence/window.txt"
import -window "$window" "$evidence/launcher.png"
test -s "$evidence/launcher.png"
echo 'Linux launcher window and screenshot verified.'
