import { GetCommand, PutCommand, type DynamoDBDocumentClient } from '@aws-sdk/lib-dynamodb';
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
  if (name.includes('pebble_2_duo')) return 'Pebble 2 Duo';
  if (name.includes('pebble_2_hr')) return 'Pebble 2 HR';
  if (name.includes('pebble_2')) return 'Pebble 2';
  if (name.includes('pebble_time_steel')) return 'Pebble Time Steel';
  if (name.includes('pebble_time_2')) return 'Pebble Time 2';
  if (name.includes('pebble_time_round')) return 'Pebble Time Round';
  if (name.includes('pebble_time')) return 'Pebble Time';

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
    }
  }

  // Round sums at the end to avoid losing all the cumulated remainders
  return Object
    .values(buckets)
    .map(b => {
      const avgBatteryLife = Math.round(b.totalBatteryLife / b.count);
      const avgRate = Math.round(b.totalRate / b.count);

      return {
        groupName: b.groupName,
        names: b.names,
        count: b.count,
        avgBatteryLife,
        avgRate,
        minBatteryLife: Math.round(b.minBatteryLife),
        maxBatteryLife: Math.round(b.maxBatteryLife),
        batteryLifeRange: Math.round(b.batteryLifeRange),
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

  // Get all uploaded records
  const res = await docClient.send(
    // Likely okay until about 5000, then pagination will be needed
    new ScanCommand({ TableName: HISTORY_TABLE_NAME, Limit: 5000 }),
  );
  if (!res.Items?.length) throw new Error('Failed to get records for aggregations');
  console.log(`Scanned ${res.Items?.length} items`);

  const allRows: DbDocument[] = res.Items as unknown as DbDocument[];
  const doc = performAggregations(allRows);

  await docClient.send(
    new PutCommand({ TableName: METADATA_TABLE_NAME, Item: doc }),
  );
  console.log(`Updated aggregations in ${Date.now() - start}ms`);
};
