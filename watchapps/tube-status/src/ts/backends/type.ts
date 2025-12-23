const MAX_REASON_LENGTH = 512;

/**
 * Status severity levels
 */
export enum StatusSeverity {
  Good = 0,
  Warning = 1,
  Severe = 2
}

/**
 * Line configuration
 */
export type LineConfig = {
  index: number;
  name: string;
  color: number;      // Hex color as uint32
  striped: boolean;   // Renders a visual difference, can be used to indicate special lines
};

/**
 * Line status data
 */
export type LineStatus = {
  index: number;
  status: string;
  severity: StatusSeverity;
  reason: string;
};

/**
 * Base abstract class for transit backends
 */
export abstract class TransitBackend {
  abstract readonly id: string;
  abstract readonly name: string;

  protected abstract lineConfigs: LineConfig[];
  protected abstract lineIdToIndex: { [key: string]: number };


  getLineConfigs(): LineConfig[] {
    return this.lineConfigs;
  }

  abstract fetchLines(): Promise<LineStatus[] & { configMapping: number[] }>;

  protected abstract mapSeverity(...args: any[]): StatusSeverity;

  protected truncateReason(reason: string): string {
    if (reason.length > MAX_REASON_LENGTH) {
      return reason.substring(0, MAX_REASON_LENGTH - 4) + '...';
    }
    return reason;
  }
}
