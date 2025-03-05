package cl_toolkit;

public class Platform {

    /**
     * Check to see if the current platform is at least Lollipop
     */
    public static boolean isLollipopOrAbove() {
        return android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.LOLLIPOP;
    }

}
