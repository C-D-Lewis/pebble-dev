package uk.me.chrislewis.dashboard2

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.material3.Scaffold
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import uk.me.chrislewis.dashboard2.components.ForcePortraitOrientation
import uk.me.chrislewis.dashboard2.components.GrantPermissionsSection
import uk.me.chrislewis.dashboard2.components.InformationSection
import uk.me.chrislewis.dashboard2.components.InstallSection
import uk.me.chrislewis.dashboard2.components.LinksSection
import uk.me.chrislewis.dashboard2.components.TitleAppBar
import uk.me.chrislewis.dashboard2.ui.theme.Dashboard2Theme

class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        enableEdgeToEdge()

        setContent {
            Dashboard2Theme {
                Scaffold(modifier = Modifier.fillMaxSize()) { padding ->
                    val scrollState = rememberScrollState()

                    Column(
                        modifier = Modifier
                            .fillMaxSize()
                            .verticalScroll(scrollState)
                    ) {
                        ForcePortraitOrientation()
                        TitleAppBar()
                        InformationSection()
                        InstallSection()
                        GrantPermissionsSection()
                        LinksSection()
                    }
                }
            }
        }
    }
}
