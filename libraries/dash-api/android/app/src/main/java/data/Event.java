package data;

import java.sql.Timestamp;
import java.util.Date;

import static data.CalendarManager.HOURS_AHEAD;
import static data.CalendarManager.MILLIS_PER_HOUR;

public class Event {

    public String title;
    public long start, end;

    public String getOneLineForm() {
        String dateString = timestampToDate(start);
        String time = dateString.substring(11, 16);
        return time + " " + title;
    }

    public String getTwoLineForm() {
        String dateString = timestampToDate(start);
        String time = dateString.substring(11, 16);
        String date = dateString.substring(4, 10);
        return date + " " + time + "\n" + title;
    }

    private static String timestampToDate(long timestamp) {
        Timestamp stamp = new Timestamp(timestamp);
        Date date = new Date(stamp.getTime());
        return date.toString();
    }

    public boolean isSoon() {
        long now = System.currentTimeMillis();
        return (start - now) < (MILLIS_PER_HOUR * HOURS_AHEAD);
    }

}
