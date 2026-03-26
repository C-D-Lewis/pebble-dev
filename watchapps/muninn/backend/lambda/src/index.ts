import { DynamoDBClient } from '@aws-sdk/client-dynamodb';
import { DynamoDBDocumentClient } from '@aws-sdk/lib-dynamodb';
import { error } from './util.ts';
import type { ApiGwEvent, EventBridgeEvent } from './types.ts';
import {
  handlePostId,
  handlePostHistory,
  handleGetHistoryById,
  handleGetGlobalStats,
} from './handlers.ts';
import { updateAggregations } from './aggregations.ts';

/**
 * Determine if this is an API GW event.
 *
 * @param {object} event - Input event.
 * @returns {boolean} true if API GW event.
 */
const isApiGwEvent = (event: any): event is ApiGwEvent => !!event.routeKey;

/**
 * Determine if this is an EventBridge event.
 *
 * @param {object} event - Input event.
 * @returns {boolean} true if EB event.
 */
const isEbEvent = (event: any): event is EventBridgeEvent => !!event['detail-type'];

/**
 * Handle trigger from API Gateway.
 *
 * @param {ApiGwEvent} event - Lambda event from API GW.
 * @param {DynamoDBDocumentClient} docClient - DynamoDB document client.
 * @returns {Promise<LambdaResponse>} Response to the request.
 */
const handleApiRequest = async (event: ApiGwEvent, docClient: DynamoDBDocumentClient) => {
  const route = event.routeKey;
  const body = event.body ? JSON.parse(event.body) : {};
  const { id } = event.pathParameters || {};
  console.log({ route, body, id });

  if (route === 'POST /id') return handlePostId(docClient, body);
  if (route === 'POST /history') return handlePostHistory(docClient, body);
  if (route === 'GET /history/{id}') return handleGetHistoryById(docClient, event, id);
  if (route === 'GET /globalStats') return handleGetGlobalStats(docClient, event);

  throw new Error('Route not found');
};

/**
 * Handle trigger from EventBridge (i.e: daily).
 *
 * @param {EventBridgeEvent} event - EventBridge event.
 * @param {DynamoDBDocumentClient} docClient - DynamoDB document client.
 */
const handleEventBridgeEvent = async (
  event: EventBridgeEvent,
  docClient: DynamoDBDocumentClient
) => {
  console.log(event);
  if (event['detail-type'] === 'DailyAggregation') {
    return updateAggregations(docClient);
  }

  console.log('Nothing to do!');

  // Cleanup?
};

/**
 * Main Lambda handler function.
 *
 * @param {ApiGwEvent | EventBridgeEvent} event - Lambda event object.
 * @returns Promise<LambdaResponse> - Response object as required by API Gateway or EventBridge.
 */
export const handler = async (event: ApiGwEvent | EventBridgeEvent) => {
  try {
    const client = new DynamoDBClient({});
    const docClient = DynamoDBDocumentClient.from(client);

    if (isApiGwEvent(event)) return handleApiRequest(event, docClient);
    if (isEbEvent(event)) return handleEventBridgeEvent(event, docClient);
    
    throw new Error('Unknown trigger');
  } catch (e) {
    console.error(e);
    const errMsg = e instanceof Error ? e.message : 'Unknown error';
    return error(errMsg);
  }
};
