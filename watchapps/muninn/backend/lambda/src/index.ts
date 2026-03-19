import { DynamoDBClient } from '@aws-sdk/client-dynamodb';
import { DynamoDBDocumentClient } from '@aws-sdk/lib-dynamodb';
import { error } from './util.ts';
import type { LambdaEvent } from './types.ts';
import {
  handlePostId,
  handlePostHistory,
  handleGetHistoryById,
  handleGetGlobalStats,
} from './handlers.ts';

/**
 * Main Lambda handler function.
 *
 * @param {LambdaEvent} event - Lambda event object.
 * @returns Promise<LambdaResponse> - Response object as required by API Gateway.
 */
export const handler = async (event: LambdaEvent) => {
  const client = new DynamoDBClient({});
  const docClient = DynamoDBDocumentClient.from(client);

  const route = event.routeKey;
  const body = event.body ? JSON.parse(event.body) : {};
  const { id } = event.pathParameters || {};
  console.log({ route, body, id });

  try {
    // Send watch token, get unique short ID
    if (route === 'POST /id') return handlePostId(docClient, body);

    // Store history data for a given ID previously exchanged
    if (route === 'POST /history') return handlePostHistory(docClient, body);

    // Get history data for a given ID
    if (route === 'GET /history/{id}') return handleGetHistoryById(docClient, event, id);

    // Get stats about the data population
    if (route === 'GET /globalStats') return handleGetGlobalStats(docClient, event);

    throw new Error('Route not found');
  } catch (e) {
    console.error(e);
    const errMsg = e instanceof Error ? e.message : 'Unknown error';
    return error(errMsg);
  }
};
