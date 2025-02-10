package data;

import android.content.ContentUris;
import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.provider.CalendarContract;
import android.util.Log;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;

/**
 * Adapted from http://www.grokkingandroid.com/androids-calendarcontract-provider/
 */
public class CalendarManager {

    private static final String TAG = CalendarManager.class.getName();

    public static final long MILLIS_PER_HOUR = 1000 * 60 * 60L;
    public static final int HOURS_AHEAD = 24;

    private static final String[] INSTANCE_PROJECTION = new String[]{
            CalendarContract.Instances.EVENT_ID,
            CalendarContract.Instances.BEGIN,
            CalendarContract.Instances.END,
            CalendarContract.Instances.TITLE,
            CalendarContract.Instances.ALL_DAY,
            CalendarContract.Instances.DISPLAY_COLOR,
    };

    /**
     * http://www.programcreek.com/java-api-examples/index.php?source_dir=clockwise-master/ustwo-clockwise-wearable/src/main/java/com/ustwo/clockwise/data/calendar/CalendarWatchFaceHelper.java
     */
    public static Event getNextCalendarEvent(Context context) {
        Uri.Builder builder = CalendarContract.Instances.CONTENT_URI.buildUpon();
        long queryStartMs = System.currentTimeMillis();
        long queryEndMs = queryStartMs + (MILLIS_PER_HOUR * HOURS_AHEAD);
        ContentUris.appendId(builder, queryStartMs);
        ContentUris.appendId(builder, queryStartMs + queryEndMs);

        final Cursor cursor = context.getContentResolver().query(builder.build(), INSTANCE_PROJECTION, null, null, null);
        if (cursor == null) {
            Log.e(TAG, "Error getting Calendar events.");
            return null;
        }

        // Build events in next hours
        ArrayList<Event> events = new ArrayList<Event>();
        while (cursor.moveToNext()) {
            Event e = new Event();
            e.title = cursor.getString(cursor.getColumnIndex(CalendarContract.Instances.TITLE));
            e.start = cursor.getLong(cursor.getColumnIndex(CalendarContract.Instances.BEGIN));
            e.end = cursor.getLong(cursor.getColumnIndex(CalendarContract.Instances.END));
            if(e.isSoon()) {
                events.add(e);
            }
        }
        cursor.close();

        if(events.size() > 0) {
            // Compare and pick next upcoming
            Collections.sort(events, new Comparator<Event>() {

                @Override
                public int compare(Event lhs, Event rhs) {
                    return Long.compare(lhs.start, rhs.start);
                }

            });

            return events.get(0);
        } else {
            return null;
        }
    }



}