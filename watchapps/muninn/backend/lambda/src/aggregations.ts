import { GetCommand, PutCommand, type DynamoDBDocumentClient, type ScanCommandOutput } from '@aws-sdk/lib-dynamodb';
import { ScanCommand } from '@aws-sdk/lib-dynamodb';
import { AGGREGATION_DOC_ID, HISTORY_TABLE_NAME, METADATA_TABLE_NAME } from './constants.ts';
import type { AggregateItem, DbDocument, GlobalStatItem, MetadataDocument } from './types.js';
import { sortByName } from './util.ts';

/**
 * Get battery life for a row - using all-time rate.
 *
 * @param {DbDocument} row - Row to use.
 * @returns {number} Rounded battery life using it's all-time rate.
 */
const getBatteryLife = (row: DbDocument) => {
  const rate = row.stats?.allTimeRate;
  if (!rate || rate === 0) return 0;

  // Don't round here to preserve accuracy at the end
  return 100 / rate;
};

/**
 * Groups: pebble_2_duo, pebble_2, pebble_time_steel, pebble_time_2, pebble_time_round, pebble_time
 *
 * @param {string} name - Name to map and group.
 * @returns {string} Mapped group name
 */
const getGroupName = (name: string): string => {
  // Explicit names in case a 'pebble_' catchall is too aggresive
  const ogNames = [
    'pebble_black',
    'pebble_white',
    'pebble_red',
    'pebble_blue',
  ];

  if (name.includes('pebble_2_duo')) return 'Pebble 2 Duo';
  if (name.includes('pebble_2_hr')) return 'Pebble 2 HR';
  if (name.includes('pebble_2')) return 'Pebble 2';
  if (name.includes('pebble_time_steel')) return 'Pebble Time Steel';
  if (name.includes('pebble_time_2')) return 'Pebble Time 2';
  if (name.includes('pebble_time_round')) return 'Pebble Time Round';
  if (name.includes('pebble_time')) return 'Pebble Time';
  if (name.includes('pebble_round_2')) return 'Pebble Round 2';
  if (name.includes('pebble_steel')) return 'Pebble Steel';
  if (ogNames.some(p => name.includes(p))) return 'Pebble (original)';

  return name;
};

/**
 * Main aggregation function.
 *
 * @param {DbDocument[]} rows - All DB rows, representing all watches.
 * @param {string} key - Key in DbDocument to aggregate on.
 * @returns {GlobalStatItem[]} List of results.
 */
export const aggregateAllByKey = (rows: DbDocument[], key: keyof DbDocument): GlobalStatItem[] => {
  const buckets: Record<string, AggregateItem> = {};

  for (const row of rows) {
    const name = (row[key] ?? '').toString().trim() || 'Unknown';
    const groupName = name.includes('pebble_') ? getGroupName(name) : name;
    const batteryLife = getBatteryLife(row);
    if (batteryLife === 0) continue; // Skip entries with no rate

    if (!buckets[groupName]) {
      // This is a new one
      buckets[groupName] = {
        groupName,
        names: [name],
        totalBatteryLife: batteryLife || 0,
        totalRate: row.stats?.allTimeRate || 0,
        count: 1,
        minBatteryLife: batteryLife || 0,
        maxBatteryLife: batteryLife || 0,
        batteryLifeRange: 0,
        values: [batteryLife],
      };
    } else {
      // Update current aggregate
      const b = buckets[groupName];
      b.totalBatteryLife += batteryLife || 0;
      b.totalRate += row.stats?.allTimeRate || 0;
      b.count++;
      if (batteryLife < b.minBatteryLife) {
        b.minBatteryLife = batteryLife;
      }
      if (batteryLife > b.maxBatteryLife) {
        b.maxBatteryLife = batteryLife;
      }
      b.batteryLifeRange = b.maxBatteryLife - b.minBatteryLife;
      if (!b.names.includes(name)) b.names.push(name);
      b.values.push(batteryLife);
    }
  }

  const getMedian = (arr: number[]): number => {
    if (!arr.length) return 0;
  
    const sorted = [...arr].sort((a, b) => a - b);
    const index = Math.floor(sorted.length / 2);
    return sorted[index] as number;
  };

  // Round sums at the end to avoid losing all the cumulated remainders
  return Object
    .values(buckets)
    .map(b => {
      // Note: this is calculated before outliers are removed...
      const avgRate = Math.round(b.totalRate / b.count);

      // Remove outliers using Median Absolute Deviation (MAD)
      // Standard deviation is more sensitive to extreme outliers but used to filter outliers
      const median = getMedian(b.values);
      const mad = getMedian(b.values.map(v => Math.abs(v - median)));
      const madCutoff = Math.max(3 * mad, 1.5);

      const filtered = b.values.filter(v => Math.abs(v - median) <= madCutoff);
      const sorted = filtered.map(v => Math.round(v)).sort((a, b) => a - b);

      const totalBatteryLife = filtered.reduce((sum, v) => sum + v, 0);
      const avgBatteryLife = filtered.length ? totalBatteryLife / filtered.length : 0;

      // Median as alternative to average, less affected by outliers
      const medianBatteryLife = getMedian(filtered);

      // Find most common battery life, but not as useful
      const modeBatteryLife = sorted.reduce((mode, value, _, arr) => {
        const count = arr.filter(v => v === value).length;
        return count > mode.count ? { value, count } : mode;
      }, { value: 0, count: 0 }).value;

      const minBatteryLife = sorted.length ? sorted[0]! : 0;
      const maxBatteryLife = sorted.length ? sorted[sorted.length - 1]! : 0;

      return {
        groupName: b.groupName,
        names: b.names,
        count: filtered.length,
        avgBatteryLife: Math.round(avgBatteryLife),
        avgRate,
        medianBatteryLife,

        // Not yet used in UI, just for testing in localAggregations.mjs
        minBatteryLife,
        maxBatteryLife,
        batteryLifeRange: sorted.length ? (maxBatteryLife - minBatteryLife) : 0,
        values: sorted,
        modeBatteryLife,
      };
    });
};

/**
 * Core aggregation function.
 *
 * @param {DbDocument[]} allRows - All DB rows, representing all watches.
 * @returns {MetadataDocument} Document to save with aggregations results.
 */
export const performAggregations = (allRows: DbDocument[]): MetadataDocument => {
  // Filter out some rows (i.e: test data)
  const ignores = ['qemu', 'TEST', 'test'];
  const rows = allRows.filter((p) => {
    return !(ignores.some((q) => p.model.includes(q) || p.platform.includes(q)))
  });

  const models: GlobalStatItem[] = aggregateAllByKey(rows, 'model');
  const platforms: GlobalStatItem[] = aggregateAllByKey(rows, 'platform');
  // top IDs?
  // firmwares? Less meaningful without model/platform context

  // Save all
  const doc: MetadataDocument = {
    id: AGGREGATION_DOC_ID,
    historyCount: rows.length,
    models: models.sort(sortByName),
    platforms: platforms.sort(sortByName),
    updatedAt: Date.now(),
  };
  return doc;
};

/**
 * Update daily aggregations.
 *
 * @param {DynamoDBDocumentClient} docClient - DynamoDB document client.
 */
export const updateAggregations = async (docClient: DynamoDBDocumentClient) => {
  const start = Date.now();

  // Limit how often this can be called
  const metaRes = await docClient.send(
    new GetCommand({ TableName: METADATA_TABLE_NAME, Key: { id: AGGREGATION_DOC_ID } }),
  );
  const lastUpdated = metaRes.Item?.updatedAt ?? 0;
  const fiveMins = 5 * 60 * 1000;
  if (start - lastUpdated < fiveMins) {
    console.log('Aggregations were updated recently, skipping update.');
    return;
  }

  // Get all uploaded records (paginated due to 1MB limit)
  const allItems = [];
  let lastEvaluatedKey;
  do {
    const res: ScanCommandOutput = await docClient.send(
      new ScanCommand({
        TableName: HISTORY_TABLE_NAME,
        Limit: 5000,
        ExclusiveStartKey: lastEvaluatedKey
      }),
    );
    if (res.Items) {
      allItems.push(...res.Items);
    }
    lastEvaluatedKey = res.LastEvaluatedKey;
    console.log({ fetched: allItems.length, lastEvaluatedKey });
  } while (lastEvaluatedKey);
  console.log(`Fetched ${allItems.length} records for aggregation`);

  const allRows: DbDocument[] = allItems as unknown as DbDocument[];
  const doc = performAggregations(allRows);

  await docClient.send(
    new PutCommand({ TableName: METADATA_TABLE_NAME, Item: doc }),
  );
  console.log(`Updated aggregations in ${Date.now() - start}ms`);
};
