#!/bin/bash

SERVER_BIN="./wg_server"
SERVER_PORT=5002
SERVER_HOST="127.0.0.1"

# 检查是否有正在运行的服务器
SERVER_PID=$(pgrep -f "$SERVER_BIN")
if [ -n "$SERVER_PID" ]; then
    echo "Server already running with PID $SERVER_PID. Killing it."
    kill -9 $SERVER_PID
fi

# 启动服务器
$SERVER_BIN &
SERVER_PID=$!
echo "Server started with PID $SERVER_PID."

# 等待服务器启动
sleep 2

# 测试服务器
echo "Testing server using telnet..."
echo "hello server" | nc $SERVER_HOST $SERVER_PORT > output.log 2>&1

# 检查测试结果
if grep -q "ok" output.log; then
    echo "Server responded correctly. Test passed!"
else
    echo "Server did not respond correctly. Test failed!"
    cat output.log
fi

# 清理
echo "Cleaning up..."
kill -9 $SERVER_PID
rm -f output.log

echo "Test completed."
