package uk.me.chrislewis.dashboard2.components

import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.Card
import androidx.compose.material3.CardDefaults
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp

@Composable
fun InformationSection() {
    Card(
        modifier = Modifier
            .fillMaxWidth()
            .padding(16.dp, 8.dp),
        colors = CardDefaults.cardColors(
            containerColor = MaterialTheme.colorScheme.surfaceVariant
        )
    ) {
        Column(
            modifier = Modifier
                .fillMaxWidth()
                .padding(16.dp)
        ) {
            Text(
                text = "About",
                style = MaterialTheme.typography.titleMedium,
                fontWeight = FontWeight.Bold
            )
            Text(
                text = "This app communicates with the official Pebble app to pass messages between the Dashboard 2 watchapp and Android.",
                style = MaterialTheme.typography.bodyLarge,
                modifier = Modifier.padding(0.dp, 8.dp)
            )
            Text(
                text = "Please keep it installed for Dashboard 2 to keep working as expected.",
                style = MaterialTheme.typography.bodyLarge,
                modifier = Modifier.padding(0.dp, 8.dp)
            )
        }
    }
}
