wg_server:wg_server.cpp
	g++ $^ -o $@ -levent -std=c++14
clean:
	rm -rf $@