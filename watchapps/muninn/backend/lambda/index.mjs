import { DynamoDBClient } from '@aws-sdk/client-dynamodb';
import { DynamoDBDocumentClient, PutCommand, GetCommand, QueryCommand } from '@aws-sdk/lib-dynamodb';

const IDS_TABLE_NAME = 'ids';
const HISTORY_TABLE_NAME = 'history';

const client = new DynamoDBClient({});
const docClient = DynamoDBDocumentClient.from(client);

const generateId = () => Math.random().toString(36).substring(2, 8);

const badRequest = (message) => ({ statusCode: 400, body: JSON.stringify({ message }) });

const error = (message) => ({ statusCode: 500, body: JSON.stringify({ message }) });

const success = (data) => ({ statusCode: 200, body: JSON.stringify(data) });

const validateHistory = (history) => {
  return history.every(p => {
    return typeof p.timestamp === 'number' &&
      typeof p.chargePerc === 'number' &&
      typeof p.timeDiff === 'number' &&
      typeof p.chargeDiff === 'number' &&
      typeof p.rate === 'number' &&
      typeof p.result === 'number';
  });
};

const handlePostId = async (body) => {
  const { watchToken } = body;
  if (!watchToken || watchToken.length !== 32) return badRequest('Invalid watchToken');

  // Check if the watch token is already used (regardless of id)
  const items = await docClient.send(
    new QueryCommand({
      TableName: IDS_TABLE_NAME,
      IndexName: 'WatchTokenIndex',
      KeyConditionExpression: 'watchToken = :token',
      ExpressionAttributeValues: { ':token': watchToken },
    })
  );
  if (items.Items.length > 0) return badRequest('watchToken already exists');

  // Check in ids DynamoDB table if it is already used
  let id = generateId();
  let existing = await docClient.send(
    new GetCommand({ TableName: IDS_TABLE_NAME, Key: { id } })
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
      new PutCommand({ TableName: IDS_TABLE_NAME, Item: { id, watchToken } })
    );
  } catch (e) {
    console.error(e);
    return error('Failed to store ID');
  }

  return success({ id });
};

const handlePostHistory = async (body) => {
  const { id, history } = body;
  if (!id) return badRequest('id is required');
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
      new PutCommand({ TableName: HISTORY_TABLE_NAME, Item: { id, history } })
    );
    return success({ success: true });
  } catch (e) {
    console.error(e);
    return error('Failed to store history');
  }
};

const handleGetHistory = async (id) => {
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

    return success({ history: found.Item.history });
  } catch (e) {
    console.error(e);
    return error('Failed to get history');
  }
}

export const handler = async (event) => {
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
    if (route === 'GET /history/{id}') return handleGetHistory(id);

    throw new Error('Route not found');
  } catch (e) {
    console.error(e);
    return error(e.message || 'Unknown error');
  }
};
