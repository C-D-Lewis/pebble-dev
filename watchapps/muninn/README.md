# muninn :zap: :bird:

_Odin tasked Muninn with memory of the land... Over time, he will provide you with battery wisdom._

<table>
  <tr>
    <td><img src="screenshots/diorite.png" /></td>
    <td><img src="screenshots/diorite-menu.png" /></td>
    <td><img src="screenshots/diorite-log.png" /></td>
  </tr>
</table>

Muninn is an extremely lightweight battery estimation app that uses the
Wakeup API instead of the Background Worker to monitor battery discharging
trends and provide insight without negatively affecting battery life. It uses a
moving average of six-hourly readings over time to estimate remaining days of
battery life and rate of discharge.

Estimates will begin to appear after two of these readings have been taken.

Features include:
- Negligible battery impact.
- Moving-average estimation of remaining battery life, weighted towards recent values.
- Display of last observed charge and predicted next charge date.
- Notifies when there is about one day of charge remaining.
- Attempt to set AppGlance to show estimate in the launcher menu.
- View detailed logs of recent estimates.
- View general battery tips and Muninn lore.
- Ability to disable temporarily.
- Completely delete and reset all app data.

Options:
- Vibrate when a sample is taken.
- Advise when battery charge is below a chosen threshold (50%/20%/10%).
- Set a timeline pin at noon the day charge may run out.
- Be notified if the rate of drain is 2x average.

Important notes:

Once installed it must be woken up, and if the watch is off at the next
six-hourly interval, the app must be launched to allow wakeups to be resumed.
If the first period is less than six hours, the initial estimate may be an
overestimate but will smooth out soon. Any missed wakeups will be ignored.

## Notes

- Can we use wakeup instead of background worker to track battery life?
- Can we do it without storing huge amounts of samples?
- Initial launch -> wakeup estimate will be on a shorter period of time
- Watches reporting 10% increments may overestimate?
- In scope: current level, remaining days, rate per day, moving average
- Out of scope: fancy graphs, using background worker

## Challenges:

- Detecting charging: charge level goes up or is constant
- Extremely short charge: use elapsed period instead
- Handling partial discharges: use calculated rate instead of all samples
- Handling changing demands/watchface etc: moving rate
- Zero change since last sample (<1% per hour) - ignore

## Bugs

Please report bugs including - watch model and age (i.e.: Pebble Time Steel, 
from 2016) and what kind of anomalous behavior you encountered.

## TODO:

- [ ] Self-evaluation of prediction accuracy over time
- [ ] Extremely simple chart using log timestamps? 
- [ ] Export/import data log to/from JS
- [ ] Show more detailed data and charts on JS settings page?
- [ ] Use `pebble-packet` when `pebble-events` is updated with `flint`
- [ ] Use ContentSize API more
- [ ] Simpler Chalk layouts
