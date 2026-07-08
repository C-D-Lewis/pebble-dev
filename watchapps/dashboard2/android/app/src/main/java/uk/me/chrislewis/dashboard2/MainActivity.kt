package uk.me.chrislewis.dashboard2

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import uk.me.chrislewis.dashboard2.components.ForcePortraitOrientation
import uk.me.chrislewis.dashboard2.components.GrantPermissionsSection
import uk.me.chrislewis.dashboard2.components.TestFeatureSection
import uk.me.chrislewis.dashboard2.components.TitleAppBar
import uk.me.chrislewis.dashboard2.components.WatchappInstallButtonSection
import uk.me.chrislewis.dashboard2.ui.theme.Dashboard2Theme

private const val TAG = "MainActivity"

class MainActivity : ComponentActivity() {
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
                        GrantPermissionsSection()
                        TestFeatureSection()
                    }
                }
            }
        }
    }
}
