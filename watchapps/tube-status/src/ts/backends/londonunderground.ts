import { TransitBackend, LineConfig, LineStatus, StatusSeverity } from './type';

// API data type
type TfLApiResult = {
  id: string;
  lineStatuses: {
    statusSeverity: number;
    statusSeverityDescription: string;
    reason?: string;
  }[];
};

/**
 * London Underground / TfL transit backend
 */
class LondonUndergroundBackend extends TransitBackend {
  readonly id = 'tfl';
  readonly name = 'London Underground';

  private readonly MODES = ['tube', 'dlr', 'elizabeth-line', 'overground'];
  private readonly apiUrl = `https://api.tfl.gov.uk/Line/Mode/${this.MODES.join(',')}/Status`;

  protected lineConfigs: LineConfig[] = [
    { index: 0, name: "Bakerloo", color: 0xB36305, striped: false },
    { index: 1, name: "Central", color: 0xE32017, striped: false },
    { index: 2, name: "Circle", color: 0xFFD300, striped: false },
    { index: 3, name: "District", color: 0x00782A, striped: false },
    { index: 4, name: "DLR", color: 0x00AFAD, striped: true },
    { index: 5, name: "Elizabeth", color: 0x9364CD, striped: true },
    { index: 6, name: "Hammersmith & City", color: 0xF3A9BB, striped: false },
    { index: 7, name: "Jubilee", color: 0xA0A5A9, striped: false },
    { index: 8, name: "Liberty", color: 0x686868, striped: true },
    { index: 9, name: "Lioness", color: 0xFEAF3F, striped: true },
    { index: 10, name: "Metropolitan", color: 0x9B0056, striped: false },
    { index: 11, name: "Mildmay", color: 0x0055FF, striped: true },
    { index: 12, name: "Northern", color: 0x000000, striped: false },
    { index: 13, name: "Piccadilly", color: 0x003688, striped: false },
    { index: 14, name: "Suffragette", color: 0x55FF00, striped: true },
    { index: 15, name: "Victoria", color: 0x0098D4, striped: false },
    { index: 16, name: "Waterloo & City", color: 0x95CDBA, striped: false },
    { index: 17, name: "Weaver", color: 0xA12860, striped: true },
    { index: 18, name: "Windrush", color: 0xE32017, striped: true },
  ];

  protected lineIdToIndex: { [key: string]: number } = {
    'bakerloo': 0,
    'central': 1,
    'circle': 2,
    'district': 3,
    'dlr': 4,
    'elizabeth': 5,
    'hammersmith-city': 6,
    'jubilee': 7,
    'liberty': 8,
    'lioness': 9,
    'metropolitan': 10,
    'mildmay': 11,
    'northern': 12,
    'piccadilly': 13,
    'suffragette': 14,
    'victoria': 15,
    'waterloo-city': 16,
    'weaver': 17,
    'windrush': 18,
  };

  /**
   * Map TfL severity codes to StatusSeverity enum
   * TfL severity codes:
   * 20 - No Service
   * 19 - Information
   * 18 - No Issues
   * 17 - Issues Reported
   * 16 - Not Running
   * 15 - Diverted
   * 14 - Change of Frequency
   * 13 - No Step Free Access
   * 12 - Exit Only
   * 11 = Part Closed
   * 10 = Good Service
   * 9 = Minor Delays
   * 6 = Severe Delays
   * 5 = Part Closure
   * 4 = Planned Closure
   * 3 = Part Suspended
   * 2 = Suspended
   * 1 = Closed
   * 0 = Suspended
   */
  protected mapSeverity(severityCode: number): StatusSeverity {
    switch (severityCode) {
      case 18:
      case 10:
        return StatusSeverity.Good;
      case 17:
      case 14:
      case 13:
      case 12:
      case 11:
      case 9:
      case 5:
        return StatusSeverity.Warning;
      case 20:
      case 19:
      case 16:
      case 15:
      case 6:
      case 4:
      case 3:
      case 2:
      case 1:
      case 0:
        return StatusSeverity.Severe;
      default:
        return StatusSeverity.Warning;
    }
  }

  /**
   * Fetch current line statuses from TfL API
   */
  async fetchLines(): Promise<LineStatus[] & { configMapping: number[] }> {
    try {
      const data = await PebbleTS.fetchJSON<TfLApiResult[]>(this.apiUrl);

      if (!data || !Array.isArray(data)) {
        console.warn('No valid data from TfL API');
        return this.emptyResult();
      }

      const filtered = data
        .filter((line: TfLApiResult) => {
          return line.lineStatuses &&
            line.lineStatuses.length > 0
        })
        .map((line: TfLApiResult): LineStatus | null => {
          const lineIndex = this.lineIdToIndex[line.id];
          if (lineIndex === undefined) {
            return null; // Skip unknown lines
          }

          const status = line.lineStatuses[0];
          return {
            index: lineIndex,
            status: status.statusSeverityDescription || 'Unknown',
            severity: this.mapSeverity(status.statusSeverity),
            reason: status.reason || '',
          };
        })
        .filter((line): line is LineStatus => line !== null)
        .filter((line: LineStatus) => line.severity !== StatusSeverity.Good);

      const result = filtered as LineStatus[] & { configMapping: number[] };
      result.configMapping = filtered.map(line => line.index);
      return result;
    } catch (error) {
      console.log(`Error fetching London lines: ${error}`);
      return this.emptyResult();
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
export const londonUndergroundBackend = new LondonUndergroundBackend();
