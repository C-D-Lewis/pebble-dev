package uk.me.chrislewis.dashboard2.components

import android.annotation.SuppressLint
import android.app.Activity
import android.content.pm.ActivityInfo
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.ui.platform.LocalContext

@SuppressLint("SourceLockedOrientationActivity")
@Composable
fun ForcePortraitOrientation() {
    val context = LocalContext.current

    LaunchedEffect(Unit) {
        val activity = context as? Activity
        activity?.requestedOrientation = ActivityInfo.SCREEN_ORIENTATION_PORTRAIT
    }
}
