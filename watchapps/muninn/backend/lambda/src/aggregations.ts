import { PutCommand, type DynamoDBDocumentClient } from '@aws-sdk/lib-dynamodb';
import { getHistoryCount } from './db.ts';
import { ScanCommand } from '@aws-sdk/lib-dynamodb';
import { AGGREGATION_DOC_ID, HISTORY_TABLE_NAME, METADATA_TABLE_NAME } from './constants.ts';
import type { DbDocument, GlobalStatItem, MetadataDocument } from './types.js';
import { sortByCount } from './util.ts';

type AggregateItem = {
  name: string;
  totalBatteryLife: number;
  totalRate: number;
  count: number;
};

/**
 * Get battery life for a row - using all-time rate.
 *
 * @param {DbDocument} row - Row to use.
 * @returns {number} Rounded battery life using it's all-time rate.
 */
const getBatteryLife = (row: DbDocument) => {
  const rate = row.stats?.allTimeRate;
  if (!rate || rate === 0) return 0; 

  return 100 / rate;
};

/**
 * Main aggregation function.
 *
 * @param {DbDocument[]} rows - All DB rows, representing all watches.
 * @param {string} key - Key in DbDocument to aggregate on.
 * @returns {GlobalStatItem[]} List of results.
 */
export const aggregateAllByKey = (rows: DbDocument[], key: keyof DbDocument) => {
  const buckets: Record<string, AggregateItem> = {};
  
  for (const row of rows) {
    const name = String(row[key]);

    // Have we already seen this value of the key?
    if (!buckets[name]) {
      // This is a new one
      buckets[name] = {
        name,
        totalBatteryLife: getBatteryLife(row) || 0,
        totalRate: row.stats.allTimeRate || 0,
        count: 1
      };
    } else {
      // Update current aggregate
      const b = buckets[name];
      b.totalBatteryLife += getBatteryLife(row) || 0;
      b.totalRate += row.stats.allTimeRate || 0;
      b.count++;
    }
  }

  // Round sums at the end to avoid losing all the cumulated remainders
  return Object
    .values(buckets)
    .map(b => {
      const avgBatteryLife = Math.round(b.totalBatteryLife / b.count);
      const avgRate = Math.round(b.totalRate / b.count);

      return {
        name: b.name,
        count: b.count,
        avgBatteryLife,
        avgRate
      };
    });
};

/**
 * Update daily aggregations.
 *
 * @param {DynamoDBDocumentClient} docClient - DynamoDB document client.
 */
export const updateAggregations = async (docClient: DynamoDBDocumentClient) => {
  const start = Date.now();

  // Get all uploaded records
  const res = await docClient.send(
    // Likely okay until about 5000, then pagination will be needed
    new ScanCommand({ TableName: HISTORY_TABLE_NAME, Limit: 5000 }),
  );
  if (!res.Items?.length) throw new Error('Failed to get records for aggregations');
  console.log(`Scanned ${res.Items?.length} items`);

  const allRows: DbDocument[] = res.Items as unknown as DbDocument[];

  // Filter out some rows (i.e: test data)
  const ignores = ['qemu', 'TEST', 'test'];
  const rows = allRows.filter((p) => {
    return !(ignores.some((q) => p.model.includes(q) || p.platform.includes(q)))
  });

  const historyCount = await getHistoryCount(docClient) || 0;
  const models: GlobalStatItem[] = aggregateAllByKey(rows, 'model');
  const platforms: GlobalStatItem[] = aggregateAllByKey(rows, 'platform');
  // top IDs?
  // firmwares? Less meaningful without model/platform context

  // Save all
  const doc: MetadataDocument = {
    id: AGGREGATION_DOC_ID,
    historyCount,
    models: models.sort(sortByCount),
    platforms: platforms.sort(sortByCount),
    updatedAt: Date.now(),
  };
  await docClient.send(
    new PutCommand({ TableName: METADATA_TABLE_NAME, Item: doc }),
  );
  console.log(`Updated aggregations in ${Date.now() - start}ms`);
};
