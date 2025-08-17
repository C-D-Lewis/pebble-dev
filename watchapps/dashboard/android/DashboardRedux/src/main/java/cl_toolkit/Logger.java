package cl_toolkit;

import android.annotation.SuppressLint;
import android.util.Log;

import java.io.File;
import java.io.FileOutputStream;
import java.io.FileWriter;
import java.io.PrintWriter;
import java.sql.Date;
import java.text.SimpleDateFormat;

/**
 * File system synchronised logger
 * @author Chris Lewis
 */
public class Logger {	

	private static final String TAG = Logger.class.getName();
	
	/**
	 * Levels available
	 */
	public static final String
		INFO = "[I]",
		ERROR = "[E]",
		DEBUG = "[D]";
	
	private File file;
	
	/**
	 * Create a new log
	 * @param path			Path to output file location
	 * @param maxSizeBytes	Max size in bytes
	 */
	public Logger(String path, int maxSizeBytes) {
		file = new File(path);
		
		//Delete when too large
		if(file.exists()) {
			long length = file.length();
			if(length > maxSizeBytes) {
				file.delete();
				file = new File(path);
				log(TAG, "Fresh new log file! (Old one was " + length + "B", Logger.INFO);
			}
		}
	}
	
	/**
	 * Start a new session in the log file
	 */
	public void startNewSession() {
		try {
			FileWriter fw = new FileWriter(file, true);
			
			fw.write("\n" + "===== New AppMessage session: " + getTimestamp() + " =====\n");
			fw.write("Size is: " + file.length() + "B " + "\n");
			
			fw.close();
		} catch(Exception e) {
			System.err.println("Error starting new log session!");
			e.printStackTrace();
		}
	}

	/**
	 * Add a log event to the log file
	 * @param TAG			TAG of the logging class
	 * @param message		Message to log
	 * @param levelConstant	Constant value to show the level
	 */
	public void log(String TAG, String message, String levelConstant) {
		try {
			FileWriter fw = new FileWriter(file, true);
			
			String complete = "[" + getTimestamp() + "] " + levelConstant + " [" + TAG + "] " + message;
			
			//Log to file
			fw.write(complete + "\n");
			
			//Log to ADB
			if(levelConstant.equals(INFO)) {
				Log.i(TAG, complete);
			} else if(levelConstant.equals(DEBUG)) {
				Log.d(TAG, complete);
			} else if(levelConstant.equals(ERROR)) {
				Log.e(TAG, complete);
			}
			
			//Finally
			fw.flush();
			fw.close();
		} catch(Exception e) {
			System.err.println("Error writing to log file!");
			e.printStackTrace();
		}
	}
	
	/**
	 * Log a stack trace
	 * @param e	The Exception to print
	 */
	public void logStackTrace(Exception e) {
		PrintWriter pw;
		try {
			pw = new PrintWriter(new FileOutputStream(file, true));
			e.printStackTrace(pw);
			pw.flush();
			pw.close();
		} catch (Exception e1) {
			e1.printStackTrace();
		}
	}

	/**
	 * Get a timestamp string
	 * @return time as a String
	 */
	@SuppressLint("SimpleDateFormat")
	private String getTimestamp() {
		Date date = new Date(System.currentTimeMillis());
		SimpleDateFormat sdf = new SimpleDateFormat("dd/MM HH:mm:ss.SSS");
		return sdf.format(date);
	}
	
}
