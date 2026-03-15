import { expect } from 'chai';
import fetch from 'node-fetch';
import { TEST_UPLOAD } from './constants.mjs';

/** API URL */
const API_URL = 'https://muninn-api.chrislewis.me.uk';
/** Test watch token */
const TEST_WATCH_TOKEN = '0123456789abcdef0123456789abtest';

let id;

/**
 * API request helper.
 */
const apiRequest = async (path, method, body) => {
  const opts = {
    method,
    headers: { 'Content-Type': 'application/json' },
  };
  if (body) {
    opts. body = JSON.stringify(body);
  }

  const res = await fetch(`${API_URL}${path}`, opts);
  const json = await res.json();
  const status = res.status;

  return { json, status };
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
      expect(json.history).to.have.length(TEST_UPLOAD.history.length);
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
