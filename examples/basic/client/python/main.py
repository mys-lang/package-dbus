import dbus


def main():
    bus = dbus.SessionBus()
    remote_object = bus.get_object("org.example.BasicDemo", "/SomeObject")
    interface = dbus.Interface(remote_object, "org.example.BasicDemo")
    print(interface.Random())
    print(interface.Hello("Hello!"))
    print(interface.Echo(1.0))
    print(interface.Cat(__file__))
    print(interface.Sum([1, 2]))
    print(interface.Info())
    print(interface.Exit())


if __name__ == '__main__':
    main()
