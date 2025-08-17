package cl_toolkit;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.util.HashMap;

/**
 * Key:Value database in a file, safe and easy to use. Nothing should be uncommitted.
 * @author Chris Lewis
 */
public class FileMap {

	private final String DELIMITER = ":";

	private File file;

	/**
	 * Default constructor
	 */
	public FileMap(String path) {
		try {
			file = new File(path);

			//Make a new one
			if(!file.exists()) {
				FileWriter fw = new FileWriter(file);
				fw.flush();
				fw.close();
			}
		} catch(Exception e) {
			System.err.println("Error opening file " + path);
			e.printStackTrace();
		}
	}

	/**
	 * Put a value in the db
	 * @param key	Key String
	 * @param value	Value String
	 * @return		true if successful, else false
	 */
	public boolean put(String key, String value) {
		try {
			//Load into hashmap
			HashMap<String, String> map = new HashMap<String, String>();
			BufferedReader br = new BufferedReader(new FileReader(file));
			String next = br.readLine();
			while(next != null) {
				String currentKey = next.substring(0, next.indexOf(DELIMITER));
				String currentValue = next.substring(next.indexOf(DELIMITER) + 1);

				//Add existing to map
				map.put(currentKey, currentValue);

				//Finally
				next = br.readLine();
			}

			//Add new
			map.put(key, value);

			//Save all
			br.close();

			FileWriter fw = new FileWriter(file);
			for(String k : map.keySet()) {
				fw.write(k + DELIMITER + map.get(k));
				fw.write("\n");
			}

			//Finally
			fw.flush();
			fw.close();
			return true;
		} catch(Exception e) {
			System.err.println("Error putting " + file.getAbsolutePath());
			e.printStackTrace();
			return false;
		}
	}

	/**
	 * Get a value
	 * @param key	Key String
	 * @return		Value if found, else null
	 */
	public String get(String key) {
		try {
			//Open file
			BufferedReader br = new BufferedReader(new FileReader(file));

			//Find the key
			String next = br.readLine();
			while(next != null) {
				//If it is found
				if(next.contains(key)) {
					br.close();
					return next.substring(next.indexOf(key + DELIMITER) + (key.length() + 1));
				}

				//Get next
				next = br.readLine();
			}

			//Failed
//			System.err.println("Could not find " + key + " in file " + file.getAbsolutePath());
			br.close();
			return null;
		} catch(Exception e) {
			System.err.println("Error getting " + key + " from " + file.getAbsolutePath());
			e.printStackTrace();
			return null;
		}
	}
	
	/**
	 * Get a HashMap of all contained keys and values
	 * @return	HashMap of all contained keys and values
	 */
	public HashMap<String, String> getHashMap() {
		HashMap<String, String> result = new HashMap<String, String>();
		
		try {
			//Open file
			BufferedReader br = new BufferedReader(new FileReader(file));

			//Find the key
			String next = br.readLine();
			while(next != null) {
				//Split and add
				result.put(next.substring(0, next.indexOf(DELIMITER)), next.substring(next.indexOf(DELIMITER) + 1));

				//Get next
				next = br.readLine();
			}

			br.close();
		} catch(Exception e) {
			System.err.println("Error getting hashmap from " + file.getAbsolutePath());
			e.printStackTrace();
			return null;
		}
		
		//Finally
		return result;
	}
	
}