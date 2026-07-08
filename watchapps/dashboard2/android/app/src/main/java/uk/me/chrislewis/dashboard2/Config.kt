package uk.me.chrislewis.dashboard2

object Config {
    /**
     * Get toggle order as a string
     *
     * TODO: Use persist array and construct.
     */
    fun getToggleOrderString(): String {
        // 18 chars for 9 toggles
        //   [type, state] pairs
        //     types: 0:Nothing, 1:AutoSync, ...
        //     states: 0:off, 1:on, ...
        return "100000000000000000"
    }
}