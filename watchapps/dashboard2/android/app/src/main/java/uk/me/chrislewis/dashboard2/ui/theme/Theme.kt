package uk.me.chrislewis.dashboard2.ui.theme

import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.darkColorScheme
import androidx.compose.runtime.Composable
import androidx.compose.ui.graphics.Color

private val DarkColorScheme = darkColorScheme(
    primary = Color(0xFFAA0000),  // DarkCandyAppleRed
    secondary = Color(0xFF750000),

    onPrimary = Color(0xFFDDDDDD),

    surfaceVariant = Color(0xFF222222),
)

@Composable
fun Dashboard2Theme(
    content: @Composable () -> Unit
) {
    MaterialTheme(
        colorScheme = DarkColorScheme,
        typography = Typography,
        content = content
    )
}