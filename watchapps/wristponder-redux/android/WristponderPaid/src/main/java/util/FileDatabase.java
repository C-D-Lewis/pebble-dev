package util;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;

import android.content.Context;
import android.util.Log;

import config.Build;

/*
 * A line by line text database, sorted and committed after each change.
 */
public class FileDatabase {
	//Configuration
	private static final boolean DEBUG = false;
	public static final String TAG = FileDatabase.class.getName();
	
	//Constants
	public static final String
	//Must align with 'pos' in itemSelectedCallback
		SORT_ALPHABETIC_ASC = "atoz",
		SORT_ALPHABETIC_DESC = "ztoa";
	
	//Data storage
	private ArrayList<String> items = new ArrayList<String>();
	private String 
		dbPath = "path",
		sortMode = "atoz";
	
	//Other members
	Context context;
	
	public FileDatabase(Context c, String path, String sortModeConstant) {
		dbPath = path;
		context = c;
		sortMode = sortModeConstant;
		load(context.getFilesDir() + dbPath);	//Load internal
	}
	
	public boolean put(String newItem) {
		try {
			//Add to memory
			items.add(newItem);
			
			//Commit internally
			return commit();
		} catch (Exception e) {
			log("Error writing to file!");
			e.printStackTrace();
			return false;
		}
	}
	
	public boolean remove(String query) {
		try {
			if(contains(query)) {
				log("Removing " + query + " from database.");
				
				//Remove from memory
				items.remove(items.indexOf(query));
				
				//Save new list internally
				return commit();
			} else {
				log("Tried to remove non-existant item. How did that happen?");
				return false;
			}
		} catch (Exception e) {
			log("Error removing item!");
			e.printStackTrace();
			return false;
		}
	}
	
	public boolean contains(String query) {
		log("Beginning search for " + query);
		
		for(String s : items) {
			//If a valid item
			Log.d(TAG, "Testing: " + s);
			
			//Check it doesn't match known items
			if(query.equals(s)) {
				log("Match: " + s + " -> " + query);
				return true;
			}
		}
		
		//Failed
		log("Search for " + query + " failed!");
		return false;
	}

	public boolean load(String path) {
		log("Loading from " + path);
		
		try {
			if(items.size() > 0) {
				log("Clearing list...");
				items.clear();
			}
		
			File dbFile = new File(path);
			BufferedReader br = new BufferedReader(new FileReader(dbFile));
			
			String s = null;
			s = br.readLine();
			while(s != null) {
				log("Found: " + s);
				if(items.size() < Build.MAX_RESPONSES) {
					items.add(s);
				} else {
					log("Ignoring item " + s + " as it is more than MAX_RESPONSES.");
				}
				
				//Get next
				s = br.readLine();
			}
			
			//Done
			br.close();
			
			//Commit to app memory
			return commit();
		} catch (Exception e) {
			log("Error opening file!");
			e.printStackTrace();
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
			for(String s : items) {
				fw.write(s + "\n");
			}
			fw.flush();
			fw.close();
			
			return true;
		} catch (Exception e) {
			log("Error saving.");
			e.printStackTrace();
			return false;
		}
	}
	
	public boolean commit() {
		try {
			log("Committing...");
			
			//Sort
			if(sortMode.equals(SORT_ALPHABETIC_ASC)) {
				Collections.sort(items, String.CASE_INSENSITIVE_ORDER);
			} else if(sortMode.equals(SORT_ALPHABETIC_DESC)) {
				//Convert
				String[] arr = new String[items.size()];
				for(int i = 0; i < items.size(); i++) {
					arr[i] = items.get(i);
				}
				
				//Sort
				Arrays.sort(arr, Collections.reverseOrder());
				
				//Re-add
				items.clear();
				for(String s : arr) {
					items.add(s);
				}
			} else {
				Collections.sort(items, String.CASE_INSENSITIVE_ORDER);
			}
			
			//Write new state to internal
			File dbFile = new File(context.getFilesDir() + dbPath);
			
			//Overwrite
			if(dbFile.exists()) {
				dbFile.delete();
			}
			
			//Write items
			FileWriter fw = new FileWriter(dbFile, false);
			for(String s : items) {
				fw.write(s + "\n");
			}
			fw.flush();
			fw.close();
			
			return true;
		} catch (Exception e) {
			log("Error committing.");
			e.printStackTrace();
			return false;
		}
	}
	
	public boolean moveUp(int position) {
		//Read out
		String[] arr = new String[items.size()];

		//Move
		String temp = arr[position - 1];
		arr[position - 1] = arr[position];
		arr[position] = temp;
		
		//re-add
		items.clear();
		for(String s : arr) {
			items.add(s);
		}
		
		return commit();
	}
	
	public boolean moveDown(int position) {
		//Read out
		String[] arr = new String[items.size()];

		//Move
		String temp = arr[position + 1];
		arr[position + 1] = arr[position];
		arr[position] = temp;
		
		//re-add
		items.clear();
		for(String s : arr) {
			items.add(s);
		}
		
		return commit();
	}
	
	public String[] getStringArray() {
		String[] output = new String[items.size()];
		
		for(int i = 0; i < items.size(); i++) {
			output[i] = items.get(i);
		}
		
		return output;
	}
	
	public ArrayList<String> getArrayList() {
		return items;
	}
	
	private void log(String message) {
		if(DEBUG) {
			Log.d(TAG, message);
		}
	}
	
	public void setSortMode(String sortModeConstant) {
		sortMode = sortModeConstant;
	}

}
