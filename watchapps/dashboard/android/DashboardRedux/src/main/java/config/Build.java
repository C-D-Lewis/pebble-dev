package config;

import java.util.UUID;

/**
 * Build-time data
 */
public class Build {

	/** Android app version string */
	public static final String VERSION = "4.17";

	/** Pebble watchapp UUID */
    public static final UUID WATCH_APP_UUID = UUID.fromString("d522bc8e-65f3-4edf-9651-05e1e4567021");
    /** Log filename */
    public static final String DEBUG_LOG_NAME = "Dashboard-log.txt";
    /** Android store package name */
    public static final String PACKAGE_NAME = "com.wordpress.ninedof.dashboard";
    /** Max size of debug log before re-creating */
    public static final int DEBUG_LOG_MAX_SIZE_BYTES = 100000;	// 100kB
    /** Most recent compatible watchapp version */
    public static final String WATCH_APP_COMPATIBLE_VERSION = "4.8";
    /** Hide some log details like the protocol in production */
    public static final boolean RELEASE = true;
	/** Number of toggles - used for safe iteration over all configuration spinners (NEVER CHANGE THIS AGAIN, may cause problems with saved config on both sides) */
    public static final int NUM_TOGGLES = 9;

}
