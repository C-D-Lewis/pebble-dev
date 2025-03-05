package phone;

import android.annotation.TargetApi;
import android.app.Notification;
import android.os.Build;
import android.service.notification.NotificationListenerService;
import android.service.notification.StatusBarNotification;
import android.util.Log;

@TargetApi(Build.VERSION_CODES.JELLY_BEAN_MR2)
public class NotificationReader extends NotificationListenerService {

    private static final String TAG = NotificationReader.class.getName();

    @TargetApi(Build.VERSION_CODES.KITKAT)
    @Override
    public void onNotificationPosted(StatusBarNotification statusBarNotification) {
        // New notification
        Notification notification = statusBarNotification.getNotification();
        Notification.Action[] actions = notification.actions;

        for(Notification.Action a : actions) {
            Log.i("tag", "" + a.title);
        }
    }

    @Override
    public void onNotificationRemoved(StatusBarNotification statusBarNotification) {

    }
}
