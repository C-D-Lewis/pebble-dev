import { expect } from 'chai';
import fetch from 'node-fetch';

/** API URL */
const API_URL = 'https://muninn-api.chrislewis.me.uk';
/** Test watch token */
const TEST_WATCH_TOKEN = '0123456789abcdef0123456789abtest';
/** Test upload body */
const TEST_UPLOAD = {
  history: [
    { ts: 1772798405, cp: 80, r: 8, res: 8 },
    { ts: 1772776805, cp: 82, r: 8, res: 8 },
    { ts: 1772755205, cp: 84, r: 8, res: 8 },
    { ts: 1772733605, cp: 86, r: 8, res: 8 },
    { ts: 1772712005, cp: 88, r: 8, res: 8 },
    { ts: 1772690405, cp: 90, r: 8, res: 8 },
    { ts: 1772668805, cp: 92, r: 8, res: 8 },
    { ts: 1772647205, cp: 94, r: 8, res: 8 },
  ],
  platform: 'TEST',
  model: 'TEST',
  firmware: '0.0.0',
  stats: {
    count: 0,
    totalDays: 0,
    allTimeRate: 0,
    lastWeekRate: 0,
    numCharges: 0,
    mtbc: 0,
  },
};


let id;

/**
 * API request helper.
 */
const apiRequest = async (path, method, body) => {
  const opts = { method, headers: { 'Content-Type': 'application/json' } };
  if (body) {
    opts. body = JSON.stringify(body);
  }

  const res = await fetch(`${API_URL}${path}`, opts);
  return {
    json: await res.json(),
    status: res.status,
  };
};

describe('API', () => {
  describe('POST /id', () => {
    it('should return 200', async () => {
      const { status, json } = await apiRequest(
        '/id',
        'POST',
        { watchToken: TEST_WATCH_TOKEN },
      );

      expect(status).to.equal(200);
      expect(json.id).to.have.length(6);

      id = json.id;
    });

    it('should return 400 for a bad request (invalid token)', async () => {
      const { status, json } = await apiRequest('/id', 'POST', { watchToken: 'wrong' });

      expect(status).to.equal(400);
      expect(json).to.deep.equal({ error: 'Bad Request' });
    });
  });

  describe('POST /history', () => {
    it('should return 200', async () => {
      const { status, json } = await apiRequest(
        '/history',
        'POST',
        { id, ...TEST_UPLOAD },
      );

      expect(status).to.equal(200);
      expect(json).to.deep.equal({ success: true });
    });

    it('should return 404 for id not found', async () => {
      const { status, json } = await apiRequest(
        '/history',
        'POST',
        { id: 'foofoo', ...TEST_UPLOAD },
      );

      expect(status).to.equal(404);
      expect(json).to.deep.equal({ error: 'Not Found' });
    });

    it('should return 400 for bad request (wrong format)', async () => {
      const { status, json } = await apiRequest(
        '/history',
        'POST',
        { id, ...TEST_UPLOAD, platform: 1 },
      );

      expect(status).to.equal(400);
      expect(json).to.deep.equal({ error: 'Bad Request' });
    });
  });

  describe('GET /history/{id}', () => {
    it('should return 200', async () => {
      const { status, json } = await apiRequest(`/history/${id}`, 'GET');

      expect(status).to.equal(200);
      expect(json.updatedAt).to.be.greaterThan(Date.now() - 60000);
      expect(json.history).to.deep.equal(TEST_UPLOAD.history);
      expect(json.firmware).to.be.equal(TEST_UPLOAD.firmware);
      expect(json.platform).to.be.equal(TEST_UPLOAD.platform);
      expect(json.model).to.be.equal(TEST_UPLOAD.model);
      expect(json.stats).to.deep.equal(TEST_UPLOAD.stats);
    });

    it('should return 404 for id not found', async () => {
      const { status, json } = await apiRequest('/history/foofoo', 'GET');

      expect(status).to.equal(404);
      expect(json).to.deep.equal({ error: 'Not Found' });
    });
  });

  describe('GET /globalStats', () => {
    it('should return 200', async () => {
      const { status, json } = await apiRequest('/globalStats', 'GET');

      expect(status).to.equal(200);
      expect(json.totalUploads).to.be.a('number');
    });
  });
});
