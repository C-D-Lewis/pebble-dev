# tiny-battery

Can we use wakeup instead of background worker to more efficiently track
battery life? Can we do it without storing huge amounts of samples?

## In scope:

- Time since last charge
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

## Names:

- Tiny Battery
- Muninn
- Battery Guardian
- Light Battery Monitor (LBM)

## TODO:

- [x] Rolling window of N sample values for better average
- [x] App UI (summary > menu > state view)
- [x] Wakeup UI
- [ ] Bird blinking
- [ ] Disable confirmation window
- [ ] UI polish (more animations?)
