# GateKeyNotifier

This is the repository for a companion notification device for [my GateKey system](https://github.com/BrydonLeonard/GateKey). GateKey sends notifications to my phone when people enter the complex gate, but I'm not always with my phone. The notifier runs on an ESP32 with a connected speaker and RGB LED and makes noises to make sure I notice visitors.

## Getting started

You'll need:
- An ESP32
- A speaker (I used [this 1W 8Ω one](https://www.communica.co.za/products/spkr-0-86in-1w-8ohm-22x3mm?utm_source=www.communica.co.za&variant=31669372649545&sfdr_ptcid=31591_617_647087485&sfdr_hash=501cbb957f2f52f7bc093e828bba42e2&gclid=Cj0KCQiAm4WsBhCiARIsAEJIEzWwhFusAfHJxz44kNoDhU6De-1eAeK3aY7RR87a5oOFb1NngHdC2iUaAsVrEALw_wcB))
- A RGB LED (I used [this one](https://www.communica.co.za/products/bmt-rgb-led-4p-5mm-c-cath-10-pk?variant=39397074403401))
- An enclosure to hold everything

Solder up the speaker to any pins that support analog output. When the two were wired to the same ground pin, I encountered some audio interference, so I'd recommend using separate pins.

In `main.cpp` configure:
- The three LED pin numbers
- The speaker pin number
- Your GateKey server's address (if you're using managed GateKey, that'll be `http://brydonleonard.com/register_device`)
- Your WiFi name
- Your WiFi password

That's it! Once plugged in, the device will begin the handshake with GateKey.

## Built with 

![Mosquitto](https://img.shields.io/badge/mosquitto-%233C5280.svg?style=for-the-badge&logo=eclipsemosquitto&logoColor=white)

## Connecting devices to GateKey

> See [the GateKey README](https://github.com/BrydonLeonard/GateKey/blob/mainline/README.md) for more info on the device registration flow and API.

GateKey uses passworldess device registration to add devices to households. Once connected to the internet, this code will make requests to the configured GateKey server to register the device. A user in the desired household then requests the device via their Telegram app, closing the registration loop. The device is then issued with a set of credentials and an MQTT topic to which notifications are published whenever the gate opens.

## License

Distributed under the MIT License. See `LICENSE` for more information.