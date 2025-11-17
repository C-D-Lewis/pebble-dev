# muninn

Muninn is a simple battery tracking application that uses the Wakeup API
instead of the Background Worker to monitor battery discharging trends.

> Can we use wakeup instead of background worker to more efficiently track battery life?
> Can we do it without storing huge amounts of samples?

## In scope:

- Time since last unplugged (rough)
- Estimate time remaining
- Enable/disable wakeups

## Out of scope:

- Fancy graphs
- Using background worker slot which may impact battery life

## Challenges:

- Detecting charging: first sample that is_charging
- Extremely short charge: min 1 day before counting?
- Handling partial discharges: use calculated rate instead of all samples?
- Handling changing demands/watchface etc: moving rate?
- Zero change since last sample (<1% per hour)

## TODO:

- [x] Rolling window of N values for better average
- [x] App UI (summary > menu > state view)
- [x] Wakeup UI
- [x] Bird blinking
- [x] Disable confirmation (long press)
- [x] UI polish
- [x] Handle missing samples while off
- [ ] Settings? Custom alert?
- [ ] Emery screen size support
