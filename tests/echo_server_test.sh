#!/usr/bin/env bash
set -euo pipefail

# Find binaries
BIN_DIR=$(dirname "$0")/../build/bin

SERVER_BIN=$(find . -type f -name echo_server | head -n 1)
CLIENT_BIN=$(find . -type f -name tcp_client | head -n 1)

if [[ -z "$SERVER_BIN" || -z "$CLIENT_BIN" ]]; then
    echo "❌ tcp_client or tcp_echo_server not found"
    exit 1
fi

echo "✅ Found server: $SERVER_BIN"
echo "✅ Found client: $CLIENT_BIN"

# Start server in background
$SERVER_BIN &
SERVER_PID=$!

# Give the server a moment to start
sleep 1

# Run the client with a timeout (60 seconds)
if ! timeout 60s $CLIENT_BIN; then
    echo "❌ Client failed or timed out"
    kill $SERVER_PID
    wait $SERVER_PID || true
    exit 1
fi

# Stop the server
kill $SERVER_PID
wait $SERVER_PID || true

echo "✅ Integration test passed"
