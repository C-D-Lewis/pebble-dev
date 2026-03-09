import { DynamoDBClient } from '@aws-sdk/client-dynamodb';
import {
  DynamoDBDocumentClient,
  PutCommand,
  GetCommand,
  QueryCommand,
  ScanCommand,
} from '@aws-sdk/lib-dynamodb';
import { generateId, badRequest, error, success, validateHistory } from './util.js';
import type {
  GetHistoryResponse,
  GetStatsResponse,
  LambdaEvent,
  PostHistoryBody,
  PostIdBody
} from './types.js';
import { IDS_TABLE_NAME, HISTORY_TABLE_NAME, ALLOWED_ORIGINS } from './constants.js';

const client = new DynamoDBClient({});
const docClient = DynamoDBDocumentClient.from(client);

/**
 * Handle POST /id route.
 *
 * @param {PostIdBody} body - Request body.
 * @returns {Promise<{ id: string }>} - Response with the generated ID.
 */
const handlePostId = async (body: PostIdBody) => {
  const { watchToken } = body;
  if (!watchToken || watchToken.length !== 32) return badRequest('Invalid watchToken');

  // Check if the watch token is already has an ID
  const items = await docClient.send(
    new QueryCommand({
      TableName: IDS_TABLE_NAME,
      IndexName: 'WatchTokenIndex',
      KeyConditionExpression: 'watchToken = :token',
      ExpressionAttributeValues: { ':token': watchToken },
    }),
  );
  if (items.Items && items.Items.length > 0) {
    return success({ id: items.Items[0]?.id });
  }

  // Check in ids DynamoDB table if it is already used
  let id = generateId();
  let existing = await docClient.send(
    new GetCommand({ TableName: IDS_TABLE_NAME, Key: { id } }),
  );
  let attempts = 100;
  while (existing.Item && attempts > 0) {
    id = generateId();
    existing = await docClient.send(new GetCommand({ TableName: IDS_TABLE_NAME, Key: { id } }));
    attempts--;
  }
  if (attempts === 0) return error('Failed to generate unique ID');

  // Store it in DynamoDB with the watch token
  try {
    await docClient.send(
      new PutCommand({ TableName: IDS_TABLE_NAME, Item: { id, watchToken } }),
    );
  } catch (e) {
    console.error(e);
    return error('Failed to store ID');
  }

  return success({ id });
};

/**
 * Handle POST /history route.
 *
 * @param {PostHistoryBody} body - Request body.
 * @returns {Promise<{ success: boolean }>} - Response with the success status.
 */
const handlePostHistory = async (body: PostHistoryBody) => {
  const { id, history, platform, model, firmware } = body;
  if (!id) return badRequest('id is required');
  if (!platform) return badRequest('platform is required');
  if (!model) return badRequest('model is required');
  if (!firmware) return badRequest('firmware is required');
  if (!history || !Array.isArray(history) || !validateHistory(history))
    return badRequest('Invalid history');

  // Check if the ID exists in DynamoDB
  const existing = await docClient.send(
    new GetCommand({ TableName: IDS_TABLE_NAME, Key: { id } })
  );
  if (!existing.Item) return badRequest('id not found');

  // Store the history data in DynamoDB (replace previous for this ID)
  try {
    await docClient.send(
      new PutCommand({
        TableName: HISTORY_TABLE_NAME,
        Item: { id, history, platform, model, firmware },
      }),
    );
    return success({ success: true });
  } catch (e) {
    console.error(e);
    return error('Failed to store history');
  }
};

/**
 * Handle GET /history/{id} route.
 *
 * @param {LambdaEvent} event - Lambda event object.
 * @param {string} id - ID from the path parameters.
 * @returns {Promise<{ history: HistoryItem[] }>} - Response with the history data.
 */
const handleGetHistory = async (event: LambdaEvent, id?: string) => {
  if (!id) return badRequest('id is required');

  // Check if the ID exists in DynamoDB
  const existing = await docClient.send(
    new GetCommand({ TableName: IDS_TABLE_NAME, Key: { id } })
  );
  if (!existing.Item) return badRequest('id not found');

  // Get the history data from DynamoDB
  try {
    const found = await docClient.send(
      new GetCommand({ TableName: HISTORY_TABLE_NAME, Key: { id } })
    );
    if (!found.Item) return badRequest('history not found for this id');

    // TODO: Distill down to only data the client needs:
    //   platform, model, firmware, history GRAPH points for x/y
    // Unless we want to show the same prediction info, but it's less useful long-term

    const origin = event.headers.origin || '';
    const responseHeader = ALLOWED_ORIGINS.includes(origin) ? origin : ALLOWED_ORIGINS[1];

    const body: GetHistoryResponse = {
      history: found.Item.history,
      platform: found.Item.platform,
      model: found.Item.model,
      firmware: found.Item.firmware,
    };
    return success(
      body,
      { 'Access-Control-Allow-Origin': responseHeader },
    );
  } catch (e) {
    console.error(e);
    return error('Failed to get history');
  }
}

/**
 * Handle GET /stats route.
 *
 * @returns {Promise<GetStatsResponse>} - Response with the stats data.
 */
const handleGetStats = async () => {
  try {
    const idsData = await docClient.send(
      new ScanCommand({
        TableName: IDS_TABLE_NAME,
        Select: 'COUNT',
      }),
    );
    const historyData = await docClient.send(
      new ScanCommand({
        TableName: HISTORY_TABLE_NAME,
        Select: 'COUNT',
      }),
    );

    const body: GetStatsResponse = {
      totalIds: idsData.Count || -1,
      totalUploads: historyData.Count || -1,
    };
    return success(body);
  } catch (e) {
    console.error(e);
    return error('Failed to get stats');
  }
}

/**
 * Main Lambda handler function.
 *
 * @param {LambdaEvent} event - Lambda event object.
 * @returns Promise<LambdaResponse> - Response object as required by API Gateway.
 */
export const handler = async (event: LambdaEvent) => {
  const route = event.routeKey;
  const body = event.body ? JSON.parse(event.body) : {};
  const { id } = event.pathParameters || {};
  console.log({ route, body, id });

  try {
    // Send watch token, get unique short ID
    // { watchToken } -> { id }
    if (route === 'POST /id') return handlePostId(body);

    // Store history data for a given ID previously exchanged
    // { id, history } -> { success: true }
    if (route === 'POST /history') return handlePostHistory(body);

    // Get history data for a given ID
    // -> { history }
    if (route === 'GET /history/{id}') return handleGetHistory(event, id);

    // Get stats about the data population
    // -> { totalIds, totalUploads }
    if (route === 'GET /stats') return handleGetStats();

    throw new Error('Route not found');
  } catch (e) {
    console.error(e);
    const errMsg = e instanceof Error ? e.message : 'Unknown error';
    return error(errMsg);
  }
};
