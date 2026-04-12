import { performAggregations } from '../src/aggregations.ts';
import { readFile } from 'node:fs/promises';
import { unmarshall } from '@aws-sdk/util-dynamodb';

const main = async () => {
  try {
    const data = await readFile('./test/dump.json', 'utf-8');
    const res = JSON.parse(data);
    const cleanRows = (res.Items || []).map(unmarshall);

    const doc = performAggregations(cleanRows);
    // console.log(JSON.stringify(doc, null, 2));

    // Show stats
    for (const m of doc.models) {
      console.log(m.groupName);
      console.log(`  Items: ${m.count}`);
      console.log(`  Days: min ${m.minBatteryLife} / avg ${m.avgBatteryLife} median ${m.medianBatteryLife} / max ${m.maxBatteryLife} (range ${m.batteryLifeRange})`);
      console.log(`  Avg. Rate: ${m.avgRate}%/d`);
      console.log(`  [${m.values.join(' ')}]`);
      console.log();
    }
  } catch (e) {
    console.error(e);
  }
};

main();
