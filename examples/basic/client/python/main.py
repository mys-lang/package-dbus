import time
import argparse

import dbus
import dbus.mainloop.glib
from gi.repository import GLib


def count_signal_handler(value):
    print(f"Count: {value}")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--exit", action='store_true')
    args = parser.parse_args()

    dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)
    bus = dbus.SessionBus()
    object = bus.get_object("org.example.BasicDemo", "/SomeObject")
    interface = dbus.Interface(object, "org.example.BasicDemo")
    interface.connect_to_signal("Count", count_signal_handler)
    print(interface.Random())
    print(interface.Hello("Hello!"))
    print(interface.Echo(1.0))
    print(interface.Cat(__file__))
    print(interface.Sum([1, 2]))
    print(interface.Info())

    if args.exit:
        print(interface.Exit())
    else:
        loop = GLib.MainLoop()
        loop.run()


if __name__ == '__main__':
    main()
