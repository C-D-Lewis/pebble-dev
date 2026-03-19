import { expect } from 'chai';
import { GetCommand, PutCommand, QueryCommand } from '@aws-sdk/lib-dynamodb';
import { handleGetGlobalStats, handleGetHistoryById, handlePostHistory, handlePostId } from '../src/handlers.ts';
import { TEST_UPLOAD, TEST_WATCH_TOKEN } from './constants.mjs';
import { ScanCommand } from '@aws-sdk/client-dynamodb';

/** Mock origin value */
const MOCK_ORIGIN = 'http://localhost:8080';
/** Expected assembled response headers */
const RES_HEADERS = {
  'Content-Type': 'application/json',
  'Access-Control-Allow-Origin': MOCK_ORIGIN,
};

let mockScanResponse;
let mockQueryResponse;
let mockGetResponse;
let mockPutResponse;

const mockDocClient = {
  send: async (req) => {
    if (req instanceof ScanCommand) return mockScanResponse;
    if (req instanceof QueryCommand) return mockQueryResponse;
    if (req instanceof GetCommand) {
      // Hacky mechanism to get multiple responses in one test
      if (mockGetResponse.next) {
        const first = JSON.parse(JSON.stringify(mockGetResponse));
        mockGetResponse = mockGetResponse.next;
        return first;
      }
      return mockGetResponse;
    }
    if (req instanceof PutCommand) return mockPutResponse;

    throw new Error('Unexpected client command');
  },
};

beforeEach(() => {
  mockScanResponse = null;
  mockQueryResponse = null;
  mockGetResponse = null;
  mockPutResponse = null;
});

describe('Unit tests', () => {
  describe('handlePostId', () => {
    it('should handle valid request for existing ID', async () => {
      const id = '123456';
      mockQueryResponse = { Items: [{ id }] };

      const mockBody = { watchToken: TEST_WATCH_TOKEN };
      const res = await handlePostId(mockDocClient, mockBody);

      expect(res.statusCode).to.equal(200);
      expect(res.body).to.deep.equal(JSON.stringify({ id }));
    });

    it('should handle valid request for new ID', async () => {
      const id = '456789';
      mockQueryResponse = { Items: [{ id }] };
      mockGetResponse = { Item: undefined };

      const mockBody = { watchToken: TEST_WATCH_TOKEN };
      const res = await handlePostId(mockDocClient, mockBody);

      expect(res.statusCode).to.equal(200);
      expect(res.body).to.have.length('{"id":"123456"}'.length);
    });

    it('should try again if ID generated conflicts', async () => {
      const id = '123456';
      mockQueryResponse = {
        Items: [],
      };
      mockGetResponse = {
        Item: { id },
        next: {
          // Try again
          Item: { id },
          // And again
          next: { Item: undefined },
        },
      };

      const mockBody = { watchToken: TEST_WATCH_TOKEN };
      const res = await handlePostId(mockDocClient, mockBody);

      expect(res.statusCode).to.equal(200);
      expect(res.body).to.have.length('{"id":"123456"}'.length);

      // Be a new ID always
      expect(JSON.parse(res.body).id).to.not.equal(id);
    });

    it('should return Bad Request for bad watchToken', async () => {
      const mockBody = { watchToken: 'invalid' };
      const res = await handlePostId(mockDocClient, mockBody);

      expect(res.statusCode).to.equal(400);
      expect(res.body).to.deep.equal(JSON.stringify({ error: 'Bad Request' }));
    });
  });

  describe('handlePostHistory', () => {
    it('should handle valid request', async () => {
      const id = '123456';
      mockGetResponse = { Item: { id } };

      const mockBody = { id, ...TEST_UPLOAD };
      const res = await handlePostHistory(mockDocClient, mockBody);

      expect(res.statusCode).to.equal(200);
      expect(res.body).to.deep.equal(JSON.stringify({ success: true }));
    });

    it('should return Bad Request for invalid request', async () => {
      const mockBody = { id: 'foobar' };
      const res = await handlePostHistory(mockDocClient, mockBody);

      expect(res.statusCode).to.equal(400);
      expect(res.body).to.deep.equal(JSON.stringify({ error: 'Bad Request' }));
    });

    it('should return Not Found for bad ID', async () => {
      mockGetResponse = { Item: undefined };

      const mockBody = { id: 'notfou', ...TEST_UPLOAD };
      const res = await handlePostHistory(mockDocClient, mockBody);

      expect(res.statusCode).to.equal(404);
      expect(res.body).to.deep.equal(JSON.stringify({ error: 'Not Found' }));
    });
  });

  describe('handleGetHistory', () => {
    const mockEvent = {
      headers: { origin: MOCK_ORIGIN },
    };

    it('should handle valid request', async () => {
      const id = '123456';
      const historyRow = { updatedAt: Date.now(), ...TEST_UPLOAD };
      mockGetResponse = {
        Item: { id },
        next: { Item: historyRow },
      };

      const res = await handleGetHistoryById(mockDocClient, mockEvent, id);

      expect(res.statusCode).to.equal(200);
      expect(res.body).to.deep.equal(JSON.stringify(historyRow));
      expect(res.headers).to.deep.equal(RES_HEADERS);
    });

    it('should return Bad Request for missing ID', async () => {
      const mockEvent = {
        headers: { origin: MOCK_ORIGIN },
      };
      const res = await handleGetHistoryById(mockDocClient, mockEvent, undefined);

      expect(res.statusCode).to.equal(400);
      expect(res.body).to.deep.equal(JSON.stringify({ error: 'Bad Request' }));
    });

    it('should return Not Found for invalid ID', async () => {
      const id = 'foobar';
      mockGetResponse = { Item: undefined };

      const res = await handleGetHistoryById(mockDocClient, mockEvent, id);

      expect(res.statusCode).to.equal(404);
      expect(res.body).to.deep.equal(JSON.stringify({ error: 'Not Found' }));
    });

    it('should return Not Found for ID with no history saved', async () => {
      const id = '123456';
      mockGetResponse = {
        Item: { id },
        next: {
          Item: { history: [] },
        },
      };

      const res = await handleGetHistoryById(mockDocClient, mockEvent, id);

      expect(res.statusCode).to.equal(404);
      expect(res.body).to.deep.equal(JSON.stringify({ error: 'Not Found' }));
    });
  });

  describe('handleGetGlobalStats', () => {
    it('should handle valid request', async () => {
      mockScanResponse = { Items: [{}], Count: 1 };

      const mockEvent = {
        headers: { origin: MOCK_ORIGIN },
      };
      const res = await handleGetGlobalStats(mockDocClient, mockEvent);

      expect(res.statusCode).to.equal(200);
      expect(res.body).to.deep.equal(JSON.stringify({ totalUploads: 1 }));
      expect(res.headers).to.deep.equal(RES_HEADERS);
    });
  });
});
console.log(process.env)