package uk.me.chrislewis.dashboard2.components

import androidx.compose.foundation.Image
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.Card
import androidx.compose.material3.CardDefaults
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.text.LinkAnnotation
import androidx.compose.ui.text.SpanStyle
import androidx.compose.ui.text.TextLinkStyles
import androidx.compose.ui.text.buildAnnotatedString
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextDecoration
import androidx.compose.ui.unit.dp
import uk.me.chrislewis.dashboard2.R

@Composable
fun LinkItem(iconId: Int, linkText: String, linkUrl: String) {
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .clip(RoundedCornerShape(12.dp))
            .background(MaterialTheme.colorScheme.surfaceBright),
        verticalAlignment = Alignment.CenterVertically
    ) {
        Image(
            painter = painterResource(id = iconId),
            contentDescription = "Icon",
            modifier = Modifier.size(48.dp)
                .padding(8.dp)

        )
        Text(
            text = buildAnnotatedString {
                append(linkText)
                addLink(
                    url = LinkAnnotation.Url(
                        url = linkUrl,
                        styles = TextLinkStyles(
                            style = SpanStyle(
                                color = MaterialTheme.colorScheme.onSurface,
                                textDecoration = TextDecoration.Underline
                            )
                        )
                    ),
                    start = 0,
                    end = length
                )
            },
            style = MaterialTheme.typography.bodyLarge,
            modifier = Modifier.padding(0.dp, 8.dp)
        )
    }
}

@Composable
fun LinksSection() {
    Card(
        modifier = Modifier
            .fillMaxWidth()
            .padding(16.dp, 8.dp),
        colors = CardDefaults.cardColors(
            containerColor = MaterialTheme.colorScheme.surfaceVariant
        )
    ) {
        Column(
            verticalArrangement = Arrangement.spacedBy(8.dp),
            modifier = Modifier
                .fillMaxWidth()
                .padding(16.dp)
        ) {
            Text(
                text = "Links",
                style = MaterialTheme.typography.titleMedium,
                fontWeight = FontWeight.Bold
            )
            LinkItem(
                R.drawable.github,
                "Source on GitHub",
                "https://github.com/C-D-Lewis/pebble-dev/tree/master/watchapps/dashboard2"
            )
            LinkItem(
                R.drawable.github,
                "Feature request tracker",
                "https://github.com/C-D-Lewis/pebble-dev/issues/18"
            )
            LinkItem(
                R.drawable.discord,
                "Give feedback on Discord",
                "https://discord.com/channels/221364737269694464/1478019710187470888"
            )
        }
    }
}
