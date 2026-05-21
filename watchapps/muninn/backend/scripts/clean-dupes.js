import { DynamoDBClient } from '@aws-sdk/client-dynamodb';
import { DynamoDBDocumentClient, DeleteCommand } from '@aws-sdk/lib-dynamodb';
import { readFile } from 'fs/promises';

const client = new DynamoDBClient({ region: 'us-east-1' });
const docClient = DynamoDBDocumentClient.from(client);
const TABLE_NAME = 'ids';

const main = async () => {
  // From db-find-duplicate-ids.sh
  const DUPLICATES = JSON.parse(await readFile('./duplicates.json', 'utf-8'));

  for (const item of DUPLICATES) {
    const [firstId, ...rest] = item.ids;
    console.log(`Keeping ID: ${firstId} for token: ${item.watchToken}`);

    for (const id of rest) {
      try {
        await docClient.send(
          new DeleteCommand({
            TableName: TABLE_NAME,
            Key: { id: id }
          }),
        );
        console.log(`Successfully deleted duplicate ID: ${id}`);
      } catch (err) {
        console.error(`Failed to delete ID ${id}:`, err);
      }
    }
  }
};

main();
