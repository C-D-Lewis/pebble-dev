package config;

public class Keys {
	
	//Preference keys
	public static final String
		PREF_FIRST_RUN = "firstrun",
		PREF_IMPORT = "pref_key_importf",	//Note the 'f'
		PREF_EXPORT = "pref_key_exportf",
		PREF_AUTOCLOSE = "pref_key_autoclose",
		PREF_RESET = "pref_key_reset",
		PREF_CALLER_NAME = "pref_caller_name",
		PREF_TOPFIVE = "pref_key_topfive",
		PREF_SORT_STRING = "pref_key_sort_string",
		PREF_AUTOSTART = "pref_key_autostart",
		PREF_LATEST = "pref_key_latest",
		PREF_VERSION = "pref_key_version";
	
	//App keys
	public static final int
        AppKeyPacketType = 100,

        PacketTypeContactRequest = 30,
        AppKeyContactType = 31,           // Response will be numerical list with a type attached
        AppKeyContactQueryLetter = 32,
        AppKeyContactQueryTotal = 33,

        PacketTypeSMSRequest = 40,
        // AppKeyContactType
        AppKeyContactIndex = 41,
        AppKeyResponseIndex = 42,

        PacketTypeTranscriptSMS = 50,
        // AppKeyContactType
        // AppKeyContactIndex
        AppKeyTranscriptText = 51,

        PacketTypeError = 60,
        AppKeyErrorNoPermissions = 61,

        PacketTypeResponses = 70,  // Response will be numerical key list
        AppKeyTotalResponses = 71,

        PacketTypeSMSResult = 80,
        AppKeySMSResult = 81,
        AppKeySMSResultSuccess = 82,
        AppKeySMSResultFailed = 83;


    public static final int
        ContactTypeSMS = 0,
        ContactTypeCall = 1,
        ContactTypeIncoming = 2,
        ContactTypeFavorite = 3,
        ContactTypeSMSAndCall = 4,
        ContactTypeQuery = 5;
}
