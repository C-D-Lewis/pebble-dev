package config;

public class Keys {
	
	// Preference keys
	public static final String
		PREF_FIRST_RUN = "first-run",
		PREF_CHANGELOG_PREFIX = "changelog",
		PREF_CONFIGURE_BASE = "conf",
		PREF_FIND_PHONE_RUNNING = "find-phone-running",
//        PREF_INVERT = "invert",  // Removed in v2.0
//        PREF_ROOT_DISABLE = "pref_key_root_disable", // Removed when PREF_KEY_DATA_ENABLED was introduced to make data toggle explicit opt-in
        PREF_KEY_ADMIN = "pref_key_admin",
        PREF_KEY_VERSION = "pref_key_version",
		PREF_KEY_DATA_ENABLED = "pref_key_data_enabled",
		PREF_KEY_ROOT_ACCESS = "pref_key_root_access",
        PREF_KEY_QUICK_LAUNCH_TYPE = "pref_key_quick_launch_type",
        PREF_KEY_QUICK_LAUNCH_ENABLED = "pref_key_quick_launch_enabled",
        PREF_KEY_CHARGE_NOTIFICATION_ENABLED = "pref_key_charge_notification_enabled",
        PREF_KEY_CHARGE_NOTIFICATION_LAST_VALUE = "pref_key_charge_notification_last_value",
        PREF_KEY_FIND_PHONE_FILE = "pref_key_find_phone_file",
        PREF_KEY_FIND_PHONE_TITLE = "pref_key_find_phone_title",
        PREF_VALUE_FIND_PHONE_DEFAULT = "pref_value_find_phone_no_file";


	// AppMessage keys - mirror pebble/src/c/types.h
	public static final int
		AppKeyGSMName = 0,
		AppKeyGSMPercent = 1,
		AppKeyWifiName = 2,
		AppKeyBatteryPercent = 3,
		AppKeyStorageFreeGBMajor = 4,
		AppKeyStorageFreeGBMinor = 5,
		AppKeyToggleWifi = 6,
		AppKeyToggleData = 7,
		AppKeyToggleBluetooth = 8,
		AppKeyToggleRinger = 9,
		AppKeyToggleSync = 10,
		AppKeyToggleWifiAP = 11,
		AppKeyToggleFindPhone = 12,
		AppKeyToggleLockPhone = 13,
		AppKeyToggleAutoBrightness = 14,
		AppKeyToggleOrderString = 15,
		AppKeyIsLollipop = 16,
        AppKeyStoragePercent = 17,
        AppKeyQuickLaunchEnabled = 18,
        AppKeyQuickLaunchType = 19,
        AppKeyVersion = 20,

        ToggleTypeWifi = 0,
        ToggleTypeData = 1,
        ToggleTypeBluetooth = 2,
        ToggleTypeRinger = 3,
        ToggleTypeSync = 4,
        ToggleTypeWifiAP = 5,
        ToggleTypeFindPhone = 6,
        ToggleTypeLockPhone = 7,
        ToggleTypeAutoBrightness = 8,

		ToggleStateWaiting = 0,
		ToggleStateOff = 1,
		ToggleStateOn = 2,
		ToggleStateLoud = 3,
		ToggleStateVibrate = 4,
		ToggleStateSilent = 5,
		ToggleStateBrightnessAutomatic = 6,
		ToggleStateBrightnessManual = 7,
        ToggleStateLocked = 8,

		ErrorCodeNoRoot = 0,
		ErrorCodeNoDeviceAdmin = 1,
        ErrorCodeLockSuccess = 2,
		ErrorCodeDataNotEnabled = 3,
        ErrorCodeWrongVersion = 40,  // 4 clashes with AppKeyStorageFreeGBMajor

		MessageTypeRequestAll = 543,
		MessageTypeToggle = 544;

}
