import { SECONDS_PER_DAY, SECONDS_PER_HOUR, STATUS_CHARGED, STATUS_EMPTY, STATUS_NO_CHANGE, TEST_LOG_OK_TESTS } from '../constants';
import { MULTI_CHARGE_SAMPLE_DATA, SAMPLE_DATA } from './sample-data';
import { calculateDischargeRate, calculateLastWeekRate, calculateMeanTimeBetweenCharges, calculateNumCharges, isChargeEvent } from '../stats';
import { HistoryItem } from '../types';

let numTests = 0;

const test = (label: string, cb: Function) => {
  numTests++;

  const [result, expected] = cb();
  const ok = result === expected;
  if (ok) {
    if (TEST_LOG_OK_TESTS) console.log(`[OK] "${label}"`);
  } else {
    console.log(`[FAIL] "${label}": ${JSON.stringify(result)} != ${JSON.stringify(expected)}`);
  }
};

const testIsChargeEvent = () => {
  test('isChargeEvent > correct', () => {
    const event: HistoryItem = {
      result: STATUS_CHARGED,
      chargeDiff: -50,
      chargePerc: 80,
      rate: 10,
      timeDiff: 1000,
      timestamp: Date.now(),
    };
    return [isChargeEvent(event), true];
  });

  test('isChargeEvent > incorrect status', () => {
    const event: HistoryItem = {
      result: STATUS_NO_CHANGE,
      chargeDiff: -50,
      chargePerc: 80,
      rate: 10,
      timeDiff: 1000,
      timestamp: Date.now(),
    };
    return [isChargeEvent(event), false];
  });

  test('isChargeEvent > incorrect chargeDiff', () => {
    const event: HistoryItem = {
      result: STATUS_CHARGED,
      chargeDiff: -2,
      chargePerc: 80,
      rate: 10,
      timeDiff: 1000,
      timestamp: Date.now(),
    };
    return [isChargeEvent(event), false];
  });
};

const testCalculateNumCharges = () => {
  test('calculateNumCharges > correct', () => {
    const history = [{
      result: STATUS_CHARGED,
      chargeDiff: -30,
      chargePerc: 80,
      rate: 10,
      timeDiff: 1000,
      timestamp: Date.now(),
    }, {
      result: STATUS_CHARGED,
      chargeDiff: -20,
      chargePerc: 80,
      rate: 10,
      timeDiff: 1000,
      timestamp: Date.now(),
    }, {
      result: 8,
      chargeDiff: 4,
      chargePerc: 80,
      rate: 10,
      timeDiff: 1000,
      timestamp: Date.now(),
    }];

    return [calculateNumCharges(history), 2];
  });
};

const testCalculateDischargeRate = () => {
  test('calculateDischargeRate > correct', () => {
    const history = [{
      timestamp: 1774846807,
      chargePerc: 90,
      timeDiff: 21589,
      chargeDiff: 1,
      rate: 6,
      result: 4,
    }, {
      timestamp: 1774825218,
      chargePerc: 91,
      timeDiff: 21604,
      chargeDiff: 3,
      rate: 5,
      result: 11,
    }, {
      timestamp: 1774803614,
      chargePerc: 94,
      timeDiff: 21614,
      chargeDiff: 2,
      rate: 5,
      result: 7,
    }, {
      timestamp: 1774782000,
      chargePerc: 96,
      timeDiff: 17984,
      chargeDiff: 1,
      rate: 5,
      result: 4,
    }, {
      timestamp: 1774764016,
      chargePerc: 97,
      timeDiff: 21578,
      chargeDiff: 0,
      rate: -1,
      result: STATUS_NO_CHANGE,
    }, {
      timestamp: 1774742438,
      chargePerc: 97,
      timeDiff: 21628,
      chargeDiff: 2,
      rate: 6,
      result: 7,
    }, {
      timestamp: 1774720810,
      chargePerc: 99,
      timeDiff: 21610,
      chargeDiff: 1,
      rate: 6,
      result: 3,
    }, {
      timestamp: 1774699200,
      chargePerc: 100,
      timeDiff: 21594,
      chargeDiff: 0,
      rate: -1,
      result: STATUS_NO_CHANGE,
    }];

    return [calculateDischargeRate(history), 5];
  });

  test('calculateDischargeRate > correct with real world data', () => {
    return [calculateDischargeRate(SAMPLE_DATA), 6];
  });

  test('calculateDischargeRate > correct if no discharges', () => {
    const history = [{
      result: STATUS_CHARGED,
      chargeDiff: -30,
      chargePerc: 80,
      rate: 10,
      timeDiff: 1000,
      timestamp: Date.now(),
    }, {
      result: STATUS_CHARGED,
      chargeDiff: -20,
      chargePerc: 80,
      rate: 10,
      timeDiff: 1000,
      timestamp: Date.now(),
    }];

    return [calculateDischargeRate(history), STATUS_EMPTY];
  });
};

const testCalculateLastWeekRate = () => {
  test('calculateLastWeekRate > correct with real world data', () => {
    const now = Date.now() / 1000;
    const data = SAMPLE_DATA.map((p, i) => {
      // Update timestamps using test time, not ideal but avoids mock Date.now()
      const timestamp = now - (i * 6 * SECONDS_PER_HOUR);
      return { ...p, timestamp };
    })
    return [calculateLastWeekRate(data), 7];
  });

  test('calculateLastWeekRate > correct with not enough samples', () => {
    const history = [{
      timestamp: 1774846807,
      chargePerc: 90,
      timeDiff: 21589,
      chargeDiff: 1,
      rate: 6,
      result: 4,
    }, {
      timestamp: 1774825218,
      chargePerc: 91,
      timeDiff: 21604,
      chargeDiff: 3,
      rate: 5,
      result: 11,
    }, {
      timestamp: 1774803614,
      chargePerc: 94,
      timeDiff: 21614,
      chargeDiff: 2,
      rate: 5,
      result: 7,
    }, {
      timestamp: 1774782000,
      chargePerc: 96,
      timeDiff: 17984,
      chargeDiff: 1,
      rate: 5,
      result: 4,
    }];

    return [calculateLastWeekRate(history), STATUS_EMPTY];
  });
};

const testCalculateMeanTimeBetweenCharges = () => {
  test('calculateMeanTimeBetweenCharges > correct with example data', () => {
    return [calculateMeanTimeBetweenCharges(MULTI_CHARGE_SAMPLE_DATA), 4];
  });
};

export const testStats = () => {
  console.log('[Starting JS tests]');
  testIsChargeEvent();
  testCalculateNumCharges();
  testCalculateDischargeRate();
  testCalculateLastWeekRate();
  testCalculateMeanTimeBetweenCharges();
  console.log(`[Completed ${numTests} JS tests]`);
};
