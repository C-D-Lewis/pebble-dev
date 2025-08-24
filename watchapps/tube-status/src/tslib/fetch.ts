interface FetchOptions {
  url: string;
  responseType?: "text" | "arraybuffer";
}

function baseFetch({
  url,
  responseType,
}: FetchOptions): Promise<XMLHttpRequest> {
  return new Promise((resolve, reject) => {
    const req = new XMLHttpRequest();
    req.responseType = responseType || "";

    req.onload = () => {
      if (req.status >= 200 && req.status < 300) {
        resolve(req);
      } else {
        reject(new Error(`Request failed with status ${req.status}`));
      }
    };

    req.onerror = () => {
      reject(new Error("Network error"));
    };

    req.open("GET", url, true);
    req.send();
  });
}

/**
 * Fetch a string from a URL
 * @param url - The URL to fetch
 * @returns A promise that resolves with the string
 */
export async function fetchString(url: string): Promise<string> {
  const req = await baseFetch({ url });
  return req.responseText;
}

/**
 * Fetch a JSON object from a URL
 * @param url - The URL to fetch
 * @returns A promise that resolves with the JSON object
 */
export async function fetchJSON<T>(url: string): Promise<T> {
  const req = await baseFetch({ url });
  return JSON.parse(req.responseText);
}

/**
 * Fetch a binary file from a URL.
 * @param url - The URL to fetch
 * @returns A promise that resolves with the binary file as a Uint8Array
 */
export async function fetchBinary(url: string): Promise<Uint8Array> {
  const req = await baseFetch({ url, responseType: "arraybuffer" });
  return new Uint8Array(req.response);
}
