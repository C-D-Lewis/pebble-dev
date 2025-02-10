package dash;

public class Keys {

    protected static final int
            RequestTypeGetData = 24784,
            RequestTypeSetFeature = 24785,
            RequestTypeGetFeature = 24786,
            RequestTypeError = 24787,
            RequestTypeIsAvailable = 24788,

            AppKeyFeatureType = 47836,
            AppKeyFeatureState = 47837,
            AppKeyDataType = 47838,
            AppKeyDataValue = 47839,
            AppKeyUsesDashAPI = 47840,
            AppKeyAppName = 47841,
            AppKeyErrorCode = 47842,
            AppKeyLibraryVersion = 47843,

            DataTypeBatteryPercent = 678342,
            DataTypeGSMOperatorName = 678343,
            DataTypeGSMStrength = 678344,
            DataTypeWifiNetworkName = 678345,
            DataTypeStoragePercentUsed = 678346,
            DataTypeStorageFreeGBString = 678347,
            DataTypeUnreadSMSCount = 678348,
            DataTypeNextCalendarEventOneLine = 678349,
            DataTypeNextCalendarEventTwoLine = 678350,


            FeatureTypeWifi = 467822,
            FeatureTypeBluetooth = 467823,
            FeatureTypeRinger = 467824,
            FeatureTypeAutoSync = 467825,
            FeatureTypeHotSpot = 467826,
            FeatureTypeAutoBrightness = 467827,

            FeatureStateUnknown = 0,
            FeatureStateOff = 1,
            FeatureStateOn = 2,
            FeatureStateRingerLoud = 3,
            FeatureStateRingerVibrate = 4,
            FeatureStateRingerSilent = 5,

            ErrorCodeSuccess = 0,
            ErrorCodeSendingFailed = 1,
            ErrorCodeUnavailable = 2,
            ErrorCodeNoPermissions = 3,
            ErrorCodeWrongVersion = 4;

    public static String ReqKeyDataFTypeToString(int v) {
        switch (v) {
            case RequestTypeGetData:
                return "RequestTypeGetData";
            case RequestTypeSetFeature:
                return "RequestTypeSetFeature";
            case RequestTypeGetFeature:
                return "RequestTypeGetFeature";
            case RequestTypeError:
                return "RequestTypeError";
            case RequestTypeIsAvailable:
                return "RequestTypeIsAvailable";

            case AppKeyFeatureType:
                return "AppKeyFeatureType";
            case AppKeyFeatureState:
                return "AppKeyFeatureState";
            case AppKeyDataType:
                return "AppKeyDataType";
            case AppKeyDataValue:
                return "AppKeyDataValue";
            case AppKeyUsesDashAPI:
                return "AppKeyUsesDashAPI";
            case AppKeyAppName:
                return "AppKeyAppName";
            case AppKeyErrorCode:
                return "AppKeyErrorCode";
            case AppKeyLibraryVersion:
                return "AppKeyLibraryVersion";

            case DataTypeBatteryPercent:
                return "DataTypeBatteryPercent";
            case DataTypeGSMOperatorName:
                return "DataTypeGSMOperatorName";
            case DataTypeGSMStrength:
                return "DataTypeGSMStrength";
            case DataTypeWifiNetworkName:
                return "DataTypeWifiNetworkName";
            case DataTypeStoragePercentUsed:
                return "DataTypeStoragePercentUsed";
            case DataTypeStorageFreeGBString:
                return "DataTypeStorageFreeGBString";
            case DataTypeUnreadSMSCount:
                return "DataTypeUnreadSMSCount";
            case DataTypeNextCalendarEventOneLine:
                return "DataTypeNextCalendarEventOneLine";
            case DataTypeNextCalendarEventTwoLine:
                return "DataTypeNextCalendarEventTwoLine";

            case FeatureTypeWifi:
                return "FeatureTypeWifi";
            case FeatureTypeBluetooth:
                return "FeatureTypeBluetooth";
            case FeatureTypeRinger:
                return "FeatureTypeRinger";
            case FeatureTypeAutoSync:
                return "FeatureTypeAutoSync";
            case FeatureTypeHotSpot:
                return "FeatureTypeHotSpot";
            case FeatureTypeAutoBrightness:
                return "FeatureTypeAutoBrightness";

            case FeatureStateUnknown:
                return "FeatureStateUnknown";
            case FeatureStateOff:
                return "FeatureStateOff";
            case FeatureStateOn:
                return "FeatureStateOn";
            case FeatureStateRingerLoud:
                return "FeatureStateRingerLoud";
            case FeatureStateRingerVibrate:
                return "FeatureStateRingerVibrate";
            case FeatureStateRingerSilent:
                return "FeatureStateRingerSilent";

            default: return null;
        }
    }

    public static String ErrToString(int v) {
        switch(v) {
            case ErrorCodeSuccess: return "ErrorCodeSuccess";
            case ErrorCodeSendingFailed: return "ErrorCodeSendingFailed";
            case ErrorCodeUnavailable: return "ErrorCodeUnavailable";
            case ErrorCodeNoPermissions: return "ErrorCodeNoPermissions";
            case ErrorCodeWrongVersion: return "ErrorCodeWrongVersion";
            default: return "Unknown";
        }
    }

}
