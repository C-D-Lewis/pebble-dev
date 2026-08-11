package uk.me.chrislewis.dashboard2

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.material3.Scaffold
import androidx.compose.ui.Modifier
import uk.me.chrislewis.dashboard2.components.ForcePortraitOrientation
import uk.me.chrislewis.dashboard2.components.InformationSection
import uk.me.chrislewis.dashboard2.components.InstallSection
import uk.me.chrislewis.dashboard2.components.LinksSection
import uk.me.chrislewis.dashboard2.components.TitleAppBar
import uk.me.chrislewis.dashboard2.ui.theme.Dashboard2Theme

private const val TAG = "MainActivity"

class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        enableEdgeToEdge()

        setContent {
            Dashboard2Theme {
                Scaffold(modifier = Modifier.fillMaxSize()) { padding ->
                    Column(modifier = Modifier.fillMaxSize()) {
                        ForcePortraitOrientation()
                        TitleAppBar()
                        InformationSection()
                        InstallSection()
                        LinksSection()
//                        GrantPermissionsSection(::getLocationPermission)
                    }
                }
            }
        }
    }
}
