Test bed for a wip OHCI stack for nxdk. https://github.com/XboxDev/nxdk

## Compile 
Setup and install [nxdk](https://github.com/XboxDev/nxdk) then:
```
git clone https://github.com/Ryzee119/nxdk_ohci_test.git --recursive
cd nxdk_ohci_test
make NXDK_DIR=/path/to/nxdk
```

## Should work
* HID
* USB Audio class (Speaker + microphone)
* CDC Class
* Mass storage class example
* USB video class (Must have low speed interface and support YUY2 uncompressed video) 
