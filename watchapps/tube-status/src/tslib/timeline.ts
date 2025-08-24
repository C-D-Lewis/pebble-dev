/**
 * Get the user's timeline token for this app.
 * @returns A promise that resolves with the token, rejects on failure with error
 */
export function getTimelineToken(): Promise<string> {
  return new Promise((resolve, reject) => {
    Pebble.getTimelineToken(resolve, reject);
  });
}
