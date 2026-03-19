import { ScanCommand } from '@aws-sdk/client-dynamodb';
import { HISTORY_TABLE_NAME, IDS_TABLE_NAME } from './constants.ts';
import { GetCommand, PutCommand, QueryCommand, type DynamoDBDocumentClient } from '@aws-sdk/lib-dynamodb';
import type { DbDocument } from './types.js';

export const getHistoryRows = async (docClient: DynamoDBDocumentClient) => {
  return docClient.send(
    new ScanCommand({ TableName: HISTORY_TABLE_NAME, Select: 'COUNT' }),
  );
};

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

export const getIfIdExists = async (docClient: DynamoDBDocumentClient, id: string) => {
  const res = await docClient.send(
    new GetCommand({ TableName: IDS_TABLE_NAME, Key: { id } }),
  );

  return !!res.Item;
};

export const saveId = async (docClient: DynamoDBDocumentClient, id: string, watchToken: string) => {
  return docClient.send(
    new PutCommand({ TableName: IDS_TABLE_NAME, Item: { id, watchToken } }),
  );
};

export const saveHistory = async (docClient: DynamoDBDocumentClient, doc: DbDocument) => {
  return docClient.send(
    new PutCommand({ TableName: HISTORY_TABLE_NAME, Item: doc }),
  );
};

export const getHistoryData = async (docClient: DynamoDBDocumentClient, id: string) => {
  const found = await docClient.send(
    new GetCommand({ TableName: HISTORY_TABLE_NAME, Key: { id } })
  );
  
  return found.Item;
};
