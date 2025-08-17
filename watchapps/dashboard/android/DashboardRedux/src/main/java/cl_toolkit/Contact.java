package cl_toolkit;

import android.content.ContentResolver;
import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.provider.ContactsContract.PhoneLookup;

/**
 * Common Android Contacts helpers
 * @author Chris Lewis
 */
public class Contact {

	/**
	 * Get the last Contact who sent the user an SMS
	 * @param context	Context object
	 * @return	Name of last Contact or if unknown, the phone number
	 */
	public static String getLastSMSName(Context context, int maxLength) {
		try {
			Cursor cursor = context.getContentResolver().query(Uri.parse("content://sms/inbox"), null, null, null, null);
			cursor.moveToFirst();
	
			String sender = "";
			for(int idx=0;idx<cursor.getColumnCount();idx++)
			{
				if(cursor.getColumnName(idx).equals("address")) {
					sender = cursor.getString(idx);
					break;
				}
			}
			cursor.close();
	
			String name = getContactName(context, sender, maxLength);
			return name != null ? name : sender;
		} catch(Exception e) {
			e.printStackTrace();
			// There may be no SMS at all on this device!
			return "Unknown";
		}
	}

	/**
	 * Get the last SMS address that was received
	 * @param context	Context object
	 * @return			Last received SMS address
	 */
	public static String getLastSMSNumber(Context context) {
		try {
			Cursor cursor = context.getContentResolver().query(Uri.parse("content://sms/inbox"), null, null, null, null);
			cursor.moveToFirst();
	
			String sender = "";
			for(int idx=0;idx<cursor.getColumnCount();idx++)
			{
				if(cursor.getColumnName(idx).equals("address")) {
					sender = cursor.getString(idx);
					break;
				}
			}
			cursor.close();
	
			return sender != null ? sender : "NOTFOUND";
		} catch(Exception e) {
			e.printStackTrace();
			return "SMSADDRFAIL";
		}
	}

	/**
	 * Get a Contact number by phone number
	 * @param context		Context object
	 * @param phoneNumber	SMS address to use as a query
	 * @return				Contact name if found, else the SMS address
	 * 
	 * http://stackoverflow.com/a/18064869
	 */
	public static String getContactName(Context context, String phoneNumber, int maxLength) {
		try {
            if(phoneNumber == null) {
                throw new NullPointerException("phoneNumber is null!");
            }

			ContentResolver cr = context.getContentResolver();
			Uri uri = Uri.withAppendedPath(PhoneLookup.CONTENT_FILTER_URI, Uri.encode(phoneNumber));
			Cursor cursor = cr.query(uri, new String[]{PhoneLookup.DISPLAY_NAME}, null, null, null);
			if (cursor == null) {
				return "CNOTFOUND";
			}
			String contactName = null;
			if(cursor.moveToFirst()) {
				contactName = cursor.getString(cursor.getColumnIndex(PhoneLookup.DISPLAY_NAME));
			}
	
			if(cursor != null && !cursor.isClosed()) {
				cursor.close();
			}
	
			if(contactName != null) {
				if(contactName.length() > maxLength) {
					return contactName.substring(0, maxLength);
				} else {
					return contactName;
				}
			} else {
				return phoneNumber;
			}
		} catch(Exception e) {
			e.printStackTrace();
			return phoneNumber != null ? phoneNumber : "GCNOTFOUND";
		}
	}
}
