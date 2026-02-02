import { HttpMethod } from './types';

/** Timeline API root that coreapp will respond to */
const API_URL_ROOT = 'https://timeline-api.rebble.io';

/** Seconds in a day */
const SECONDS_PER_DAY = 60 * 60 * 24;
/** Predicted low charge time pin ID */
const PIN_ID_PREDICTION = 'muninn-prediction';

/**
 * Send a request to the Rebble public web timeline API.
 * @param pin The JSON pin to insert. Must contain 'id' field.
 * @param method The method of request, either PUT or DELETE.
 */
const timelineRequest = async (pin: TimelinePin, method: HttpMethod) => {
  const url = `${API_URL_ROOT}/v1/user/pins/${pin.id}`;

  const token = await PebbleTS.getTimelineToken();
  const res = await fetch(url, {
    method,
    headers: {
      'Content-Type': 'application/json',
      'X-User-Token': token,
    },
  });

  const json = await res.json();
  return json;
};

/**
 * Insert a pin into the timeline for this user.
 * @param pin The JSON pin to insert.
 */
const insertUserPin = async (pin: TimelinePin) => timelineRequest(pin, 'PUT');

/**
 * Handle request to push a timeline pin.
 */
export const handlePushTimelinePin = async (dict: Record<string, any>) => {
  const days = dict.DAYS_REMAINING;
  const rate = dict.DISCHARGE_RATE;

  // Put the pin in the future at noon, or today late if no days remaining
  const target = new Date(Date.now() + (days * SECONDS_PER_DAY * 1000));
  const targetHour = (days == 0 && new Date().getHours() > 12) ? 23 : 12;
  target.setHours(targetHour);
  target.setMinutes(0);
  target.setSeconds(0);

  // Create the pin
  const pin: TimelinePin = {
    id: PIN_ID_PREDICTION,
    time: target,
    layout: {
      type: 'genericPin',
      title: 'Time to charge!',
      body: `Muninn predicts you will need to charge soon (Est. ${rate}%/day)`,
      // TODO: When SDK is fixed, use publishedMedia for custom icon
      tinyIcon: 'system://images/GENERIC_WARNING',
    },
  };

  console.log(`Inserting pin: ${JSON.stringify(pin)}`);
  return insertUserPin(pin);
};
