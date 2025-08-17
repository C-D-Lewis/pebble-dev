package cl_toolkit;

public class Platform {

    public static boolean isLollipopOrAbove() {
        return android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.LOLLIPOP;
    }

}
