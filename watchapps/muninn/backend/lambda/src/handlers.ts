import {
  DynamoDBDocumentClient,
} from '@aws-sdk/lib-dynamodb';
import {
  generateId,
  badRequest,
  error,
  success,
  validatePostHistoryBody,
  cors,
  notFound,
} from './util.ts';
import type {
  GetHistoryResponse,
  ApiGwEvent,
  PostHistoryBody,
  PostIdBody,
  DbDocument
} from './types.ts';
import {
  getId,
  getIfIdExists,
  saveId,
  saveHistory,
  getHistoryData,
  getAggregations,
} from './db.ts';

/**
 * Handle POST /id route.
 *
 * @param {DynamoDBDocumentClient} docClient - DynamoDB document client.
 * @param {PostIdBody} body - Request body.
 * @returns {Promise<{ id: string }>} - Response with the generated ID.
 */
export const handlePostId = async (docClient: DynamoDBDocumentClient, body: PostIdBody) => {
  const { watchToken } = body;
  if (!watchToken || watchToken.length !== 32) return badRequest('Invalid watchToken');

  // Check if the watch token is already has an ID
  const existingId = await getId(docClient, watchToken);
  if (existingId) return success({ id: existingId });

  // Re-roll IDs until a new one is generated
  let id = generateId();
  let exists = await getIfIdExists(docClient, id);
  let attempts = 100;
  while (exists && attempts > 0) {
    id = generateId();
    exists = await getIfIdExists(docClient, id);
    attempts--;
  }
  if (attempts === 0) return error('Failed to generate unique ID');
  console.log(`Generated ID ${id} with ${attempts} attempts remaining`);

  // Store it in DynamoDB with the watch token
  await saveId(docClient, id, watchToken);

  return success({ id });
};

/**
 * Handle POST /history route.
 *
 * @param {DynamoDBDocumentClient} docClient - DynamoDB document client.
 * @param {PostHistoryBody} body - Request body.
 * @returns {Promise<{ success: boolean }>} - Response with the success status.
 */
export const handlePostHistory = async (docClient: DynamoDBDocumentClient, body: PostHistoryBody) => {
  const { id, history, platform, model, firmware, stats } = body;

  if (!validatePostHistoryBody(body)) return badRequest('Invalid request body');

  // Check if the ID exists in DynamoDB
  const exists = await getIfIdExists(docClient, id);
  if (!exists) return notFound('id not found');

  // Store the history data in DynamoDB (replace previous for this ID)
  const updatedAt = Date.now();
  const doc: DbDocument = {
    updatedAt,
    id,
    history,
    platform,
    model,
    firmware,
    stats,
  };
  await saveHistory(docClient, doc);
  return success({ success: true });
};

/**
 * Handle GET /history/{id} route.
 *
 * @param {DynamoDBDocumentClient} docClient - DynamoDB document client.
 * @param {ApiGwEvent} event - Lambda event object.
 * @param {string} id - ID from the path parameters.
 * @returns {Promise<{ history: HistoryItem[] }>} - Response with the history data.
 */
export const handleGetHistoryById = async (
  docClient: DynamoDBDocumentClient,
  event: ApiGwEvent,
  id?: string,
) => {
  if (!id) return badRequest('id is required');

  // Check if the ID exists in DynamoDB
  const exists = await getIfIdExists(docClient, id);
  if (!exists) return notFound('id not found', cors(event));

  // Get the history data from DynamoDB
  const found = await getHistoryData(docClient, id);
  if (!found || found.history.length === 0) return notFound(
    'history not found for this id',
    cors(event),
  );

  const body: GetHistoryResponse = {
    updatedAt: found.updatedAt,
    history: found.history,
    platform: found.platform,
    model: found.model,
    firmware: found.firmware,
    stats: found.stats,
  };
  return success(body, cors(event));
};

/**
 * Handle GET /globalStats route.
 *
 * @param {DynamoDBDocumentClient} docClient - DynamoDB document client.
 * @param {ApiGwEvent} event - Lambda event.
 * @returns {Promise<GetGlobalStatsResponse>} - Response with the stats data.
 */
export const handleGetGlobalStats = async (
  docClient: DynamoDBDocumentClient,
  event: ApiGwEvent,
) => {
  const aggregations = await getAggregations(docClient);

  return success(aggregations, cors(event));
};
