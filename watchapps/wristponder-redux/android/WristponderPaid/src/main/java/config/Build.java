package config;

import java.util.UUID;

public class Build {
	
	// Maximum
	public static final int 
		MAX_RESPONSES = 30,
		MAX_LENGTH = 100,
		MAX_NAME_LENGTH = 30,
		TOTAL_NAMES = 12,
		MAX_FAV_OR_INCOMING_NAMES = 5;
	
	// Version number - for changelog trigger DISABLE DEBUG & CHECK UPDATE REQUIRED FLAG!
	public static final int VERSION = 27;
	public static final boolean DEBUG = true;
	public static final String
        VERSION_STRING = "3.1",
        WATCHAPP_COMPATIBLE_VERSION_STRING = "3.1";

	// Content prefixes - DO NOT CHANGE AFTER RELEASE!
	public static final String 
		PREF_CHANGELOG_PREFIX = "changelog",	//Released
		PREFIX_FAVOURITE_NUMBER = "favourite";	//Released
	
	// Paths - DO NOT CHANGE AFTER RELEASE!
	public static final String 
		RESPONSE_DATABASE_NAME = "db.txt",	//Released
		INCOMING_DATABASE_NAME = "outgoing_db.txt",	//Released
		DEBUG_LOG_NAME = "Wristponder-debug.txt";	//Released
	
	// App Ids
	public static final String 
		PBW_FILENAME = "wristponder.pbw",
		APPSTORE_UID = "52ff6a14432d1cef0000007c";
	
	// UUID
	public static final UUID WATCHAPP_UUID = UUID.fromString("9f37b7d8-e818-4d37-a8d2-420bd44dfc8b");
	
	// Max log size
	public static final int MAX_LOG_BYTES = 100000;     // 100kB
}
