/**
 * Send a message to the watch app
 * @param message - The message object to send
 * @returns A promise that resolves when the message is sent, rejects on failure with error
 */
export function sendAppMessage(message: Record<string, any>): Promise<void> {
  return new Promise((resolve, reject) => {
    Pebble.sendAppMessage(message, resolve, reject);
  });
}
