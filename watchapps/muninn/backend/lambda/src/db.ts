import { ScanCommand } from '@aws-sdk/lib-dynamodb';
import {
  AGGREGATION_DOC_ID,
  HISTORY_TABLE_NAME,
  IDS_TABLE_NAME,
  METADATA_TABLE_NAME,
} from './constants.ts';
import {
  GetCommand,
  PutCommand,
  QueryCommand,
  type DynamoDBDocumentClient,
} from '@aws-sdk/lib-dynamodb';
import type { DbDocument, GetGlobalStatsResponse } from './types.js';

/**
 * Get history table row count.
 *
 * @param {DynamoDBDocumentClient} docClient - DynamoDB document client.
 * @returns {Promise<number | undefined>} Count result if available.
 */
export const getHistoryCount = async (docClient: DynamoDBDocumentClient) => {
  const res = await docClient.send(
    new ScanCommand({ TableName: HISTORY_TABLE_NAME, Select: 'COUNT' }),
  );

  return res.Count;
};

/**
 * Get ID for this watch token.
 *
 * @param {DynamoDBDocumentClient} docClient - DynamoDB document client.
 * @param {string} watchToken - Submitted watch token.
 * @returns {Promise<string | undefined>} ID if stored previously.
 */
export const getId = async (docClient: DynamoDBDocumentClient, watchToken: string) => {
  const found = await docClient.send(
    new QueryCommand({
      TableName: IDS_TABLE_NAME,
      IndexName: 'WatchTokenIndex',
      KeyConditionExpression: 'watchToken = :token',
      ExpressionAttributeValues: { ':token': watchToken },
    }),
  );

  return found?.Items?.[0]?.id;
};

/**
 * Check if an ID has been stored previously.
 *
 * @param {DynamoDBDocumentClient} docClient - DynamoDB document client.
 * @param {string} id - Submitted ID.
 * @returns {Promise<boolean>} True if the ID was found, false otherwise
 */
export const getIfIdExists = async (docClient: DynamoDBDocumentClient, id: string) => {
  const res = await docClient.send(
    new GetCommand({ TableName: IDS_TABLE_NAME, Key: { id } }),
  );

  return !!res.Item;
};

/**
 * Save an ids table row.
 *
 * @param {DynamoDBDocumentClient} docClient - DynamoDB document client.
 * @param {string} id - Generated ID.
 * @param {string} watchToken - Submitted watch token.
 */
export const saveId = async (docClient: DynamoDBDocumentClient, id: string, watchToken: string) => {
  return docClient.send(
    new PutCommand({ TableName: IDS_TABLE_NAME, Item: { id, watchToken } }),
  );
};

/**
 * Save a history table row.
 *
 * @param {DynamoDBDocumentClient} docClient - DynamoDB document client.
 * @param {DbDocument} doc - Complete row document to store.
 */
export const saveHistory = async (docClient: DynamoDBDocumentClient, doc: DbDocument) => {
  return docClient.send(
    new PutCommand({ TableName: HISTORY_TABLE_NAME, Item: doc }),
  );
};

/**
 * Get history data for this ID.
 *
 * @param {DynamoDBDocumentClient} docClient - DynamoDB document client.
 * @param {string} id - Submitted ID.
 * @returns {Promise<object>} DB row if found.
 */
export const getHistoryData = async (docClient: DynamoDBDocumentClient, id: string) => {
  const found = await docClient.send(
    new GetCommand({ TableName: HISTORY_TABLE_NAME, Key: { id } })
  );
  
  return found.Item;
};

/**
 * Fetch latest aggregation table data.
 *
 * @param {DynamoDBDocumentClient} docClient - DynamoDB document client.
 */
export const getAggregations = async (docClient: DynamoDBDocumentClient): Promise<GetGlobalStatsResponse> => {
  const found = await docClient.send(
    new GetCommand({ TableName: METADATA_TABLE_NAME, Key: { id: AGGREGATION_DOC_ID } })
  );

  // No data yet
  if (!found.Item) {
    return {
      historyCount: 0,
      models: [],
      platforms: [],
      updatedAt: -1,
    }
  }

  return {
    historyCount: found.Item.historyCount,
    models: found.Item.models,
    platforms: found.Item.platforms,
    updatedAt: found.Item.updatedAt,
  };
};
