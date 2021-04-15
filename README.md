Test bed for a wip OHCI stack for nxdk. https://github.com/XboxDev/nxdk

Currently requires my own fork of the nxdk toolchain:
```
cd ~
git clone https://github.com/Ryzee119/nxdk.git
cd nxdk
git checkout usbh
git submodule init
git submodule update
```

Then:

```
cd ~
git clone https://github.com/Ryzee119/nxdk_ohci_test.git
cd nxdk_ohci_test
make NXDK_DIR=/path/to/nxdk -j
```

## Should work
* HID
* USB Audio class (Speaker + microphone)
* CDC Class

## Todo
* Mass storage class example
* USB video class example 