package uk.me.chrislewis.dashboard2

import android.Manifest
import android.content.pm.PackageManager
import android.os.Bundle
import android.widget.Toast
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.material3.Scaffold
import androidx.compose.ui.Modifier
import androidx.core.content.ContextCompat
import uk.me.chrislewis.dashboard2.components.ForcePortraitOrientation
import uk.me.chrislewis.dashboard2.components.GrantPermissionsSection
import uk.me.chrislewis.dashboard2.components.TitleAppBar
import uk.me.chrislewis.dashboard2.components.WatchappInstallButtonSection
import uk.me.chrislewis.dashboard2.features.WiFi
import uk.me.chrislewis.dashboard2.ui.theme.Dashboard2Theme

private const val TAG = "MainActivity"

class MainActivity : ComponentActivity() {
    private fun showToast(message: String) {
        Toast.makeText(this, message, Toast.LENGTH_LONG).show()
    }

    private val requestPermissionLauncher = registerForActivityResult(
        ActivityResultContracts.RequestPermission()
    ) { isGranted: Boolean ->
        if (isGranted) {
            val ssid = WiFi.getWifiSSID(this)
            showToast("SSID: $ssid")
        } else {
            showToast("Location permission is required to see the Wi-Fi network name.")
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        enableEdgeToEdge()

        setContent {
            Dashboard2Theme {
                Scaffold(modifier = Modifier.fillMaxSize()) { padding ->
                    Column (modifier = Modifier.fillMaxSize()) {
                        ForcePortraitOrientation()
                        TitleAppBar()
                        WatchappInstallButtonSection()
                        GrantPermissionsSection(::getLocationPermission)
                    }
                }
            }
        }
    }

    private fun getLocationPermission() {
        val perm = Manifest.permission.ACCESS_FINE_LOCATION

        if (ContextCompat.checkSelfPermission(this, perm) == PackageManager.PERMISSION_GRANTED) {
            val ssid = WiFi.getWifiSSID(this)
            showToast("SSID: $ssid")
            return
        }

        requestPermissionLauncher.launch(perm)
    }
}
