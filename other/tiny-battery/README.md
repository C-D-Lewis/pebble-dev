# tiny-battery

Can we use wakeup instead of background worker to more efficiently track
battery life? Can we do it without storing huge amounts of samples?

## In scope:

- Current battery charge
- Time since last charge
- Estimate time remaining
- Average battery life
- Enable/disable wakeups

## Out of scope:

- Fancy graphs
- Draining battery with worker etc.

## Challenges:

- Detecting charging: first sample that is_charging
- Extremely short charged: min 1 day before counting?
- Handling partial discharges: use calculated rate instead of all samples?
- Handling changing demands/watchface etc: moving rate?
- Zero change since last sample (<1% per hour)

## Names:

- Tiny Battery
- Muninn

## TODO:

- [ ] Rolling window of N sample values for better average
- [ ] App UI (summary > menu > state view)
- [ ] Wakeup UI
