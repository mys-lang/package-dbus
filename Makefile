all:
	mys test
	mys -C examples/basic/client build
	mys -C examples/basic/server build
	mys -C examples/basic/client_bus_methods build
