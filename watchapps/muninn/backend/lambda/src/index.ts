import { DynamoDBClient } from '@aws-sdk/client-dynamodb';
import {
  DynamoDBDocumentClient,
  PutCommand,
  GetCommand,
  QueryCommand,
  ScanCommand,
} from '@aws-sdk/lib-dynamodb';
import { generateId, badRequest, error, success, validatePostHistoryBody, buildCorsHeaders, notFound } from './util.js';
import type {
  GetHistoryResponse,
  GetGlobalStatsResponse,
  LambdaEvent,
  PostHistoryBody,
  PostIdBody
} from './types.js';
import { IDS_TABLE_NAME, HISTORY_TABLE_NAME } from './constants.js';

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
  const found = await docClient.send(
    new QueryCommand({
      TableName: IDS_TABLE_NAME,
      IndexName: 'WatchTokenIndex',
      KeyConditionExpression: 'watchToken = :token',
      ExpressionAttributeValues: { ':token': watchToken },
    }),
  );
  if (found.Items && found.Items.length > 0) {
    return success({ id: found.Items[0]?.id });
  }

  // Re-roll IDs until a new one is generated
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
  console.log(`Generated ID ${id} with ${attempts} attempts remaining`);

  // Store it in DynamoDB with the watch token
  try {
    await docClient.send(
      new PutCommand({ TableName: IDS_TABLE_NAME, Item: { id, watchToken } }),
    );
    return success({ id });
  } catch (e) {
    console.error(e);
    return error('Failed to store ID');
  }
};

/**
 * Handle POST /history route.
 *
 * @param {PostHistoryBody} body - Request body.
 * @returns {Promise<{ success: boolean }>} - Response with the success status.
 */
const handlePostHistory = async (body: PostHistoryBody) => {
  const { id, history, platform, model, firmware, stats } = body;

  if (!validatePostHistoryBody(body)) return badRequest('Invalid request body');

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
        Item: { id, history, platform, model, firmware, stats },
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
const handleGetHistoryById = async (event: LambdaEvent, id?: string) => {
  if (!id) return badRequest('id is required');

  // Check if the ID exists in DynamoDB
  const existing = await docClient.send(
    new GetCommand({ TableName: IDS_TABLE_NAME, Key: { id } })
  );
  if (!existing.Item) return notFound('id not found', buildCorsHeaders(event));

  // Get the history data from DynamoDB
  try {
    const found = await docClient.send(
      new GetCommand({ TableName: HISTORY_TABLE_NAME, Key: { id } })
    );
    if (!found.Item) return notFound(
      'history not found for this id',
      buildCorsHeaders(event),
    );

    // TODO: Distill down to only data the client needs:
    //   platform, model, firmware, history GRAPH points for x/y
    // Unless we want to show the same prediction info, but it's less useful long-term

    const body: GetHistoryResponse = {
      history: found.Item.history,
      platform: found.Item.platform,
      model: found.Item.model,
      firmware: found.Item.firmware,
      stats: found.Item.stats,
    };

    return success(body, buildCorsHeaders(event));
  } catch (e) {
    console.error(e);
    return error('Failed to get history');
  }
}

/**
 * Handle GET /globalStats route.
 *
 * @param {LambdaEvent} event - Lambda event.
 * @returns {Promise<GetGlobalStatsResponse>} - Response with the stats data.
 */
const handleGetGlobalStats = async (event: LambdaEvent) => {
  try {
    const historyRows = await docClient.send(
      new ScanCommand({
        TableName: HISTORY_TABLE_NAME,
        Select: 'COUNT',
      }),
    );

    const body: GetGlobalStatsResponse = {
      totalUploads: historyRows.Count || -1,

      // Leaderboard?
      // Averages per model / FW?
    };
    return success(body, buildCorsHeaders(event));
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
    // { id, history, platform, model, firmware, stats } -> { success: true }
    if (route === 'POST /history') return handlePostHistory(body);

    // Get history data for a given ID
    // -> { history, platform, model, firmware, stats }
    if (route === 'GET /history/{id}') return handleGetHistoryById(event, id);

    // Get stats about the data population
    // -> { totalIds, totalUploads }
    if (route === 'GET /globalStats') return handleGetGlobalStats(event);

    throw new Error('Route not found');
  } catch (e) {
    console.error(e);
    const errMsg = e instanceof Error ? e.message : 'Unknown error';
    return error(errMsg);
  }
};
