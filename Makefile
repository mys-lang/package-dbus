all:
	mys test
	mys -C examples/basic/client build
	mys -C examples/basic/server build
