package cl_toolkit;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.URL;

/**
 * Web data related helpers
 * @author Chris Lewis
 */
public class Web {

	/**
	 * Download a JSON file and create it as an object
	 * @param inUrl	URL of the .json file
	 * @return		The file as a JSONObject
	 */
	public static JSONObject downloadJSON(String inUrl) {
		try {
			URL url = new URL(inUrl); 
			HttpURLConnection urlConnection = (HttpURLConnection)url.openConnection();

			//Read file
			BufferedReader br = new BufferedReader(new InputStreamReader(urlConnection.getInputStream()));
			StringBuilder builder = new StringBuilder();
			String buffer = br.readLine();
			while(buffer != null) {
				builder.append(buffer);
				buffer = br.readLine();
			}

			//Parse results
			return new JSONObject(builder.toString());
		} catch(Exception e) {
			e.printStackTrace();

			JSONObject errObj = new JSONObject();
			try {
				errObj.put("error", "Error downloading JSON!");
			} catch (JSONException e1) {
				e1.printStackTrace();
			}
			return errObj;
		}
	}

	public static String downloadHTML(String inUrl) {
		try {
			URL url = new URL(inUrl); 
			HttpURLConnection urlConnection = (HttpURLConnection)url.openConnection();

			//Read file
			BufferedReader br = new BufferedReader(new InputStreamReader(urlConnection.getInputStream()));
			StringBuilder builder = new StringBuilder();
			String buffer = br.readLine();
			while(buffer != null) {
				builder.append(buffer);

				buffer = br.readLine();
			}

			//Parse results
			return builder.toString();
		} catch(Exception e) {
			e.printStackTrace();
			return "error";
		}
	}

}
