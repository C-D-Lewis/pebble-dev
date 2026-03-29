import { expect } from 'chai';
import { GetCommand, PutCommand, QueryCommand } from '@aws-sdk/lib-dynamodb';
import {
  handleGetGlobalStats,
  handleGetHistoryById,
  handlePostHistory,
  handlePostId,
} from '../src/handlers.ts';
import { aggregateAllByKey } from '../src/aggregations.ts';
import { TEST_UPLOAD, TEST_WATCH_TOKEN } from './constants.mjs';
import { ScanCommand } from '@aws-sdk/lib-dynamodb';
import { describe } from 'node:test';

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
  describe('API Handlers', () => {
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
      it('should handle valid request with no aggregation data yet', async () => {
        mockGetResponse = { Item: undefined };

        const mockEvent = {
          headers: { origin: MOCK_ORIGIN },
        };
        const res = await handleGetGlobalStats(mockDocClient, mockEvent);

        expect(res.statusCode).to.equal(200);
        expect(res.body).to.deep.equal(
          JSON.stringify({
            historyCount: 0,
            models: [],
            platforms: [],
            updatedAt: -1,
          }),
        );
        expect(res.headers).to.deep.equal(RES_HEADERS);
      });

      it('should handle valid request with mock aggregation data', async () => {
        const testMetadataRow = {
          historyCount: 2,
          models: [{ name: 'pebble_2_duo_black', count: 19, avgBatteryLife: 23, avgRate: 5 }],
          platforms:[{name: 'flint', count: 32, avgBatteryLife: 21, avgRate: 6 }],
          updatedAt: Date.now(),
        };
        mockGetResponse = {
          Item: testMetadataRow,
        };

        const mockEvent = {
          headers: { origin: MOCK_ORIGIN },
        };
        const res = await handleGetGlobalStats(mockDocClient, mockEvent);

        expect(res.statusCode).to.equal(200);
        expect(res.body).to.deep.equal(JSON.stringify(testMetadataRow));
        expect(res.headers).to.deep.equal(RES_HEADERS);
      });
    });
  });

  describe('Aggregations', () => {
    describe('aggregateAllByKey', () => {
      it('should aggregate some example data by model field', async () => {
        const mockRows = [{
          stats: { allTimeRate: 10 },
          model: 'pebble_time_2_silver_blue',
        }, {
          stats: { allTimeRate: 10 },
          model: 'pebble_time_2_black_gray',
        }, {
          stats: { allTimeRate: 20 },
          model: 'pebble_time_steel_silver',
        }, {
          stats: { allTimeRate: 40 },
          model: 'pebble_time_steel_silver',
        }];
        const res = aggregateAllByKey(mockRows, 'model');

        const expected = [{
          groupName: 'Pebble Time 2',
          names: ['pebble_time_2_silver_blue', 'pebble_time_2_black_gray'],
          count: 2,
          avgBatteryLife: 10,
          avgRate: 10,
        }, {
          groupName: 'Pebble Time Steel',
          names: ['pebble_time_steel_silver'],
          count: 2,
          avgBatteryLife: 4,
          avgRate: 30,
        }];
        
        expect(res).to.deep.equal(expected);
      });

      it('should aggregate all models correctly', async () => {
        const mockRows = [{
          stats: { allTimeRate: 10 },
          model: 'pebble_2_duo_black',
        }, {
          stats: { allTimeRate: 10 },
          model: 'pebble_2_hr_charcoal_red',
        }, {
          stats: { allTimeRate: 10 },
          model: 'pebble_time_red',
        }, {
          stats: { allTimeRate: 10 },
          model: 'pebble_time_2_black_gray',
        }, {
          stats: { allTimeRate: 40 },
          model: 'pebble_time_round_silver_20',
        }, {
          stats: { allTimeRate: 20 },
          model: 'pebble_time_steel_silver',
        }];
        const res = await aggregateAllByKey(mockRows, 'model');

        const expected = [{
          avgBatteryLife: 10,
          avgRate: 10,
          count: 1,
          groupName: 'Pebble 2 Duo',
          names: [
            'pebble_2_duo_black'
          ],
        }, {
          avgBatteryLife: 10,
          avgRate: 10,
          count: 1,
          groupName: 'Pebble 2',
          names: [
            'pebble_2_hr_charcoal_red',
          ]
        }, {
          avgBatteryLife: 10,
          avgRate: 10,
          count: 1,
          groupName: 'Pebble Time',
          names: [
            'pebble_time_red',
          ],
        }, {
          avgBatteryLife: 10,
          avgRate: 10,
          count: 1,
          groupName: 'Pebble Time 2',
          names: [
            'pebble_time_2_black_gray',
          ],
        }, {
          avgBatteryLife: 3,
          avgRate: 40,
          count: 1,
          groupName: 'Pebble Time Round',
          names: [
            'pebble_time_round_silver_20',
          ],
        }, {
          avgBatteryLife: 5,
          avgRate: 20,
          count: 1,
          groupName: 'Pebble Time Steel',
          names: [
            'pebble_time_steel_silver',
          ],
        }];
        
        expect(res).to.deep.equal(expected);
      });

      it('should aggregate some example data by platform field', async () => {
        const mockRows = [
          {
            stats: { allTimeRate: 10 },
            platform: 'basalt',
          },
          {
            stats: { allTimeRate: 20 },
            platform: 'emery',
          },
          {
            stats: { allTimeRate: 40 },
            platform: 'emery',
          },
        ];
        const res = aggregateAllByKey(mockRows, 'platform');

        const expected = [
          {
            groupName: 'basalt',
            names: ['basalt'],
            count: 1,
            avgBatteryLife: 10,
            avgRate: 10,
          },
          {
            groupName: 'emery',
            names: ['emery'],
            count: 2,
            avgBatteryLife: 4,
            avgRate: 30,
          },
        ];
        
        expect(res).to.deep.equal(expected);
      });
    });
  });
});
