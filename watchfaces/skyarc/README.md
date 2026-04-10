# Skyarc

Daily weather-focused watchface with a comparatively minimal design. General
weather conditions for the whole day (24H) are shown around the outside of the
main arc, and relative temperatures around the inside.

If an hour is 'clear' weather, no arc segment is shown. Temperatures are shown
relative to the min/max for the day - coldest is darkest blue with two extra
gradations for 1 and 2 degrees above, and hottest is darkest red with a similar
scheme of two ligher shades.

When shaken, the outer weather arc is replaced with chance of precipitation. The
darker the shade of blue, the higher chance of rain. The battery level is shown
in the middle bar, and is green when Bluetooth is connected or grey otherwise.

App configuration includes choice of temperature unit.

Weather data from [OpenMeteo](https://open-meteo.com/).

## TODO

- [x] Fetch and format weather data
- [x] Send to watch
- [x] Large clear time in interesting clean font
- [x] Smaller date display
- [x] Dual arcs:
  - [x] Outer: general weather (clear, sunny, cloudy)
  - [x] Inner: Relative temperature vs min/max
- [x] Battery fills in separator
- [x] Regular weather fetch (1h)
- [x] Scale to display width for square watches
- [x] Disconnect feature
- [x] Arr value offset (+50) for negative number handling
- [x] Current weather / temp display
- [x] Weather failure appmessage
- [x] Animations
- [x] Design details (backgrounds, notches, etc.)
- [x] Launcher icon
- [x] New name
- [x] Configuration: temp units, ?
- [x] Update pattern used in `data.c`
- [ ] Wind speed?
