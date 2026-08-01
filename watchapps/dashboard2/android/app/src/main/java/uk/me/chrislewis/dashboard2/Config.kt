package uk.me.chrislewis.dashboard2

import uk.me.chrislewis.dashboard2.features.FindPhone

object Config {
    /**
     * Get toggle order as a string. Construct based on current state
     */
    fun getToggleOrderString(): String {
        // 2 chars per toggles - [TYPE,STATE]
        //   types: 1:FindPhone, ...
        //   states: 0:off, 1:on, ...
        return "1${if (FindPhone.isPlaying()) "1" else "0"}"
    }
}