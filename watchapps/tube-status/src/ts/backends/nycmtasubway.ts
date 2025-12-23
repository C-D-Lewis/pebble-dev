import { TransitBackend, LineConfig, LineStatus, StatusSeverity } from './type';

interface MTAAlertEntity {
    alert?: {
        active_period?: Array<{ start: number, end: number }>;
        informed_entity?: Array<{ route_id?: string }>;
        header_text?: {
            translation?: Array<{ text: string }>;
        };
        'transit_realtime.mercury_alert'?: {
            alert_type?: string;
        };
    };
}

interface RouteIssue {
    type: string;
    descriptions: string[];
    severity: StatusSeverity;
}

/**
 * NYC MTA Subway transit backend
 */
class NycMtaSubwayBackend extends TransitBackend {
    readonly id = 'nyc';
    readonly name = 'NYC MTA Subway';

    private readonly apiUrl = 'https://api-endpoint.mta.info/Dataservice/mtagtfsfeeds/camsys%2Fsubway-alerts.json';

    protected lineConfigs: LineConfig[] = [
        // Broadway-Seventh Avenue Line
        { index: 0, name: "1 Train", color: 0xEE352E, striped: false },
        { index: 1, name: "2 Train", color: 0xEE352E, striped: false },
        { index: 2, name: "3 Train", color: 0xEE352E, striped: false },
        // Lexington Avenue Line
        { index: 3, name: "4 Train", color: 0x00933C, striped: false },
        { index: 4, name: "5 Train", color: 0x00933C, striped: false },
        { index: 5, name: "6 Train", color: 0x00933C, striped: false },
        { index: 6, name: "6X Train", color: 0x00933C, striped: false },
        // Flushing Line
        { index: 7, name: "7 Train", color: 0xB933AD, striped: false },
        { index: 8, name: "7X Train", color: 0xB933AD, striped: false },
        // Broadway Line
        { index: 9, name: "N Train", color: 0xFCCC0A, striped: false },
        { index: 10, name: "Q Train", color: 0xFCCC0A, striped: false },
        { index: 11, name: "R Train", color: 0xFCCC0A, striped: false },
        { index: 12, name: "W Train", color: 0xFCCC0A, striped: false },
        // Sixth Avenue Line
        { index: 13, name: "B Train", color: 0xFF6319, striped: false },
        { index: 14, name: "D Train", color: 0xFF6319, striped: false },
        { index: 15, name: "F Train", color: 0xFF6319, striped: false },
        { index: 16, name: "M Train", color: 0xFF6319, striped: false },
        // Eighth Avenue Line
        { index: 17, name: "A Train", color: 0x0039A6, striped: false },
        { index: 18, name: "C Train", color: 0x0039A6, striped: false },
        { index: 19, name: "E Train", color: 0x0039A6, striped: false },
        // Canarsie Line
        { index: 20, name: "L Train", color: 0xA7A9AC, striped: false },
        // Crosstown Line
        { index: 21, name: "G Train", color: 0x6CBE45, striped: false },
        // Nassau Street Line
        { index: 22, name: "J Train", color: 0x996633, striped: false },
        { index: 23, name: "Z Train", color: 0x996633, striped: false },
        // Shuttles
        { index: 24, name: "S Shuttle", color: 0x808183, striped: false },
        { index: 25, name: "SIR", color: 0x0039A6, striped: false },
    ];

    protected lineIdToIndex: { [key: string]: number } = {
        '1': 0, '2': 1, '3': 2,
        '4': 3, '5': 4, '6': 5, '6X': 6,
        '7': 7, '7X': 8,
        'N': 9, 'Q': 10, 'R': 11, 'W': 12,
        'B': 13, 'D': 14, 'F': 15, 'M': 16,
        'A': 17, 'C': 18, 'E': 19,
        'L': 20, 'G': 21,
        'J': 22, 'Z': 23,
        'S': 24, 'FS': 24, 'H': 24, 'GS': 24,
        'SI': 25, 'SIR': 25,
    };

    /**
     * Map MTA alert type to StatusSeverity
     */
    protected mapSeverity(alertType: string): StatusSeverity {
        if (alertType.includes('No Scheduled Service') ||
            alertType.includes('Suspension') ||
            alertType.includes('No Service')) {
            return StatusSeverity.Severe;
        } else if (alertType.includes('Delay') ||
            alertType.includes('Service Change') ||
            alertType.includes('Planned Work')) {
            return StatusSeverity.Warning;
        } else if (alertType.includes('Extra Service')) {
            return StatusSeverity.Good;
        }
        return StatusSeverity.Warning;
    }

    /**
     * Check if an alert is currently active based on its active periods
     * MTA sometimes returns alerts with future active periods only as planned work
     */
    private isAlertActive(activePeriods: Array<{ start: number, end: number }>): boolean {
        if (!activePeriods || activePeriods.length === 0) return true;

        const now = Math.floor(Date.now() / 1000);

        for (const period of activePeriods) {
            if (now >= period.start && now <= period.end) {
                return true;
            }
        }

        return false;
    }

    /**
     * Shorten alert type for display
     */
    private formatAlertType(type: string): string {
        if (type === 'No Scheduled Service') return 'No Service';
        if (type.startsWith('Planned - ')) return type.substring(10);
        if (type === 'Station Notice') return 'Notice';
        return type || 'Service Alert';
    }

    /**
     * Fetch current line statuses from MTA API
     */
    async fetchLines(): Promise<LineStatus[] & { configMapping: number[] }> {
        try {
            const data = await PebbleTS.fetchJSON<{ entity: MTAAlertEntity[] }>(this.apiUrl);

            if (!data.entity) {
                console.warn('No entities in response');
                return this.emptyResult();
            }

            const routeIssues: { [routeId: string]: RouteIssue } = {};

            for (const entity of data.entity) {
                if (!entity.alert) continue;

                const { alert } = entity;
                if (alert.active_period && !this.isAlertActive(alert.active_period)) {
                    continue; // Not active
                }

                const impactedRoutes = new Set<string>();
                if (alert.informed_entity) {
                    for (const informed of alert.informed_entity) {
                        if (informed.route_id) {
                            impactedRoutes.add(informed.route_id);
                        }
                    }
                }

                if (impactedRoutes.size === 0) continue;

                // TODO: Handle localization
                // Using the first translation text available, probably English
                const alertText = alert.header_text?.translation?.[0]?.text || '';
                const alertType = alert['transit_realtime.mercury_alert']?.alert_type || '';
                const severity = this.mapSeverity(alertType);

                for (const routeId of impactedRoutes) {
                    // Initialize route if this is the first alert
                    if (!routeIssues[routeId]) {
                        routeIssues[routeId] = { type: alertType, descriptions: [], severity };
                    }
                    // If this alert is more severe, override type/severity
                    else if (severity > routeIssues[routeId].severity) {
                        routeIssues[routeId].type = alertType;
                        routeIssues[routeId].severity = severity;
                    }
                    // Collect all alert descriptions for this route
                    if (alertText) {
                        routeIssues[routeId].descriptions.push(alertText);
                    }
                }
            }

            const result: Array<LineStatus & { configIndex: number }> = [];

            for (const [routeId, issue] of Object.entries(routeIssues)) {
                const configIndex = this.lineIdToIndex[routeId];
                if (configIndex === undefined) {
                    console.log(`Unknown route ID: ${routeId}`);
                    continue;
                }

                if (issue.severity === StatusSeverity.Good) {
                    continue;
                }

                const status = this.formatAlertType(issue.type);
                const reason = this.truncateReason(issue.descriptions.join(' | '));

                result.push({
                    index: result.length,
                    configIndex,
                    status,
                    severity: issue.severity,
                    reason,
                });
            }

            if (result.length === 0) {
                return this.emptyResult();
            }

            // Add config mapping (maps sequential display indices to config indices)
            const typedResult = result as unknown as LineStatus[] & { configMapping: number[] };
            typedResult.configMapping = result.map(r => r.configIndex);
            return typedResult;

        } catch (error) {
            console.log(`Error in fetchLines: ${error}`);
            throw error;
        }
    }

    /**
     * Create empty result with configMapping
     */
    private emptyResult(): LineStatus[] & { configMapping: number[] } {
        const empty = [] as unknown as LineStatus[] & { configMapping: number[] };
        empty.configMapping = [];
        return empty;
    }
}

/**
 * Export singleton instance
 */
export const nycMtaSubwayBackend = new NycMtaSubwayBackend();
