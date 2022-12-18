Replace the controller included with the Feit 48ft. LED Outdoor Weatherproof Color Changing String Light Set (Feit Model # 72031) available from Costco ( https://www.costco.com/Feit-48ft.-LED-Outdoor-Weatherproof-Color-Changing-String-Light-Set%2C-Black.product.100306514.html ) with a NodeMCU board.

The included controller only has a few settings and colors, & no fading.

This project attempts to add the items above, MQTT support, while allowing the continued use of the included remote.

When using rtl_433 to decode this signal, the following command describes the signal:
```bash
rtl_433 -R 0 -X 'n=feit,m=OOK_PPM,s=208,l=608,g=620,r=6068,rows>=4,repeats=2,bits=24'
```
