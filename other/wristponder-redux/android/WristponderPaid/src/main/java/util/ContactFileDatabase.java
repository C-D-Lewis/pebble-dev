package util;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Comparator;

import android.content.Context;
import android.util.Log;

/*
 * A line by line text database of key-value pairs, sorted and committed after each change.
 */
public class ContactFileDatabase {

	//Configuration
	private static final boolean DEBUG = true;
	public static final String TAG = ContactFileDatabase.class.getName();
	
	//Data storage
	private ArrayList<String> keys = new ArrayList<String>();
	private ArrayList<String> values = new ArrayList<String>();
	private String dbPath = "path";
	
	//Other members
	Context context;
	
	public ContactFileDatabase(Context c, String p) {
		dbPath = p;
		context = c;
		try {
			load(context.getFilesDir() + "/" + dbPath);	//Load internal
		} catch (Exception e) {
			//Does not yet exist, init to empty file
//			e.printStackTrace();
			Log.d(TAG, "Outgoing DB does not yet exist.");
			commit();
		}
	
	}
	
	public boolean put(String newKey, String newValue) {
		try {
			//Add to memory
			if(contains(newKey)) {
				//Update the value corresponding to the key
				values.set(keys.indexOf(newKey), "" + newValue);
			} else {
				//Add the new key
				keys.add(newKey);
				values.add(newValue);
			}
			
			//Commit internally
			return commit();
		} catch (Exception e) {
			log("Error writing to file!");
			e.printStackTrace();
			return false;
		}
	}
	
	public String get(String key) {
		if(contains(key)) {
			return values.get(keys.indexOf(key));
		} else {
			return null;
		}
	}
	
	/**
	 * Returns top 5 addresses
	 * @return
	 */
	public String[] getTopFive() {
		ArrayList<String> resultKeys = new ArrayList<String>();
		ArrayList<String> resultValues = new ArrayList<String>();
		
		//Append
		String[] collection = new String[keys.size()];
		for(int i = 0; i < keys.size(); i++) {
			collection[i] = values.get(i) + ":" + keys.get(i);
		}
		
		//Sort - 11 is 'less' than 1
		Arrays.sort(collection, new Comparator<String>() {

			@Override
			public int compare(String lhs, String rhs) {
				//Get int
				int lValue = Integer.parseInt(lhs.substring(0, lhs.indexOf(':')));
				int rValue = Integer.parseInt(rhs.substring(0, rhs.indexOf(':')));
				
				if(lValue == rValue) {
					return 0;
				} else if(lValue < rValue) {
					return 1;
				} else {
					return -1;
				}
			}
		});
		
		//Split
		for(int i = 0; i < keys.size(); i++) {
			String s = collection[i];
			String key = s.substring(s.indexOf(':') + 1);
			String value = s.substring(0, s.indexOf(':'));
			resultKeys.add(key);
			resultValues.add(value);
		}
		
		for(int i = 0; i < resultKeys.size(); i++) {
			Log.d(TAG, "Top " + i + " KV: " + resultKeys.get(i) + ":" + resultValues.get(i));
		}
		
		//Finally
		String[] results = new String[5];
		//Get all available
		int i = 0;
		if(resultKeys.size() > 0) {
			for(i = 0; i < 5 && i < resultKeys.size(); i++) {
				results[i] = resultKeys.get(i);
				Log.d(TAG, "Slot " + i + "/5" + " is now " + results[i]);
			}
		}
		//Fill in blanks
		while(i < 5) {
			results[i] = null;
			i++;
		}
		return results;
	}
	
	public boolean remove(String key) {
		try {
			if(contains(key)) {
				log("Removing " + key + " from database.");
				
				//Remove from memory
				keys.remove(keys.indexOf(key));
				values.remove(keys.indexOf(key));
				
				//Save new list internally
				return commit();
			} else {
				log("Tried to remove non-existant item. How did that happen?");
				return false;
			}
		} catch (Exception e) {
			log("Error removing item!");
//			e.printStackTrace();
			return false;
		}
	}
	
	public boolean contains(String key) {
		log("Beginning search for " + key);
		
		for(String k : keys) {
			//If a valid item
			log("Testing: " + k);
			
			//Check it doesn't match known items
			if(key.equals(k)) {
				log("Match: " + k + " -> " + key);
				return true;
			}
		}
		
		//Failed
		log("Search for " + key + " failed!");
		return false;
	}

	public boolean load(String path) {
		log("Loading from " + path);
		
		try {
			if(keys.size() > 0) {
				log("Clearing list...");
				keys.clear();
			}
			
			if(values.size() > 0) {
				values.clear();
			}
		
			File dbFile = new File(path);
			BufferedReader br = new BufferedReader(new FileReader(dbFile));
			
			String s = null;
			s = br.readLine();
			while(s != null) {
				keys.add(s.substring(0, s.indexOf(':')));
				values.add(s.substring(s.indexOf(':') + 1));
				
				log("Loaded K:V: " + s.substring(0, s.indexOf(':')) + ":" + s.substring(s.indexOf(':') + 1));
				
				//Get next
				s = br.readLine();
			}
			
			//Done
			br.close();
			
			//Commit to app memory
			return commit();
		} catch (Exception e) {
			log("Error opening file!");
//			e.printStackTrace();
			return false;
		}
	}
	
	public boolean save(String path) {
		try {
			log("Saving to " + path);
			
			//Write new state
			File dbFile = new File(path);
			
			//Overwrite
			if(dbFile.exists()) {
				dbFile.delete();
			}
			
			//Write items
			FileWriter fw = new FileWriter(dbFile, false);
			int index = 0;
			for(String k : keys) {
				fw.write(k + ":" + values.get(index) + "\n");	//Format: KEY:VALUE
				index++;
			}
			fw.flush();
			fw.close();
			
			return true;
		} catch (Exception e) {
			log("Error saving.");
//			e.printStackTrace();
			return false;
		}
	}
	
	private boolean commit() {
		try {
			log("Committing...");
			
			//Write new state to internal
			File dbFile = new File(context.getFilesDir() + "/" + dbPath);
			
			//Overwrite
			if(dbFile.exists()) {
				dbFile.delete();
			}
			
			//Write items
			FileWriter fw = new FileWriter(dbFile, false);
			int index = 0;
			for(String k : keys) {
				fw.write(k + ":" + values.get(index) + "\n");	//Format: KEY:VALUE
				Log.d(TAG, "Committing: " + k + ":" + values.get(index));
				index++;
			}
			fw.flush();
			fw.close();
			
			return true;
		} catch (Exception e) {
			log("Error committing.");
//			e.printStackTrace();
			return false;
		}
	}
	
	public boolean clear() {
		keys.clear();
		values.clear();
		
		return commit();
	}
	
	public int getSize() {
		return keys.size();	//The two should be the same size
	}
	
	private void log(String message) {
		if(DEBUG) {
			Log.d(TAG, message);
		}
	}
}
