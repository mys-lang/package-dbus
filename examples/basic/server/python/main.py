from gi.repository import GLib

import dbus
import dbus.service
import dbus.mainloop.glib
from random import randint


MAINLOOP = None


class SomeObject(dbus.service.Object):

    @dbus.service.method("org.example.BasicDemo",
                         in_signature='',
                         out_signature='i')
    def Random(self):
        return randint(0, 100)

    @dbus.service.method("org.example.BasicDemo",
                         in_signature='s',
                         out_signature='s')
    def Hello(self, name):
        print(name)

        return 'Hi!'

    @dbus.service.method("org.example.BasicDemo",
                         in_signature='v',
                         out_signature='v')
    def Echo(self, input):
        return input

    @dbus.service.method("org.example.BasicDemo",
                         in_signature='s',
                         out_signature='ay')
    def Cat(self, path):
        with open(path, 'rb') as fin:
            return fin.read()

    @dbus.service.method("org.example.BasicDemo",
                         in_signature='ai',
                         out_signature='i')
    def Sum(self, values):
        return sum(values)

    @dbus.service.method("org.example.BasicDemo",
                         in_signature='',
                         out_signature='a{ss}')
    def Info(self):
        return {"foo": "1", "bar": "2"}

    @dbus.service.method("org.example.BasicDemo",
                         in_signature='',
                         out_signature='s')
    def Exit(self):
        MAINLOOP.quit()

        return "Bye!"


def main():
    dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)
    session_bus = dbus.SessionBus()
    name = dbus.service.BusName("org.example.BasicDemo", session_bus)
    object = SomeObject(session_bus, '/SomeObject')
    global MAINLOOP
    MAINLOOP = GLib.MainLoop()
    MAINLOOP.run()


if __name__ == '__main__':
    main()
