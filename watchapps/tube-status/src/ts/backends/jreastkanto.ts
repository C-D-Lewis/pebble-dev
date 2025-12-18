import { TransitBackend, LineConfig, LineStatus, StatusSeverity } from './type';

interface SimpleLineStatus {
    statusType: string;
    reason?: string;
}

/**
 * JR East Kanto transit backend
 */
class JrEastKantoBackend extends TransitBackend {
    readonly id = 'jreastkanto';
    readonly name = 'JR East Kanto';

    private readonly baseUrl = 'https://traininfo.jreast.co.jp/train_info';

    protected lineConfigs: LineConfig[] = [
        { index: 0, name: "Yamanote", color: 0x92BE44, striped: true },
        { index: 1, name: "Ueno-Tokyo", color: 0x306FBF, striped: false },
        { index: 2, name: "Shonan-Shinjuku", color: 0xE02E3C, striped: true },
        { index: 3, name: "Sotetsu", color: 0x306FBF, striped: false },
        { index: 4, name: "Tokaido", color: 0xF19D56, striped: true },
        { index: 5, name: "Keihin-Tohoku", color: 0x4AA4DE, striped: true },
        { index: 6, name: "Yokosuka", color: 0x3172B8, striped: true },
        { index: 7, name: "Nambu", color: 0xFBE54D, striped: true },
        { index: 8, name: "Yokohama", color: 0x92BE44, striped: true },
        { index: 9, name: "Ito", color: 0x4D8448, striped: false },
        { index: 10, name: "Sagami", color: 0x3A8488, striped: false },
        { index: 11, name: "Tsurumi", color: 0xFBE64D, striped: true },
        { index: 12, name: "Utsunomiya", color: 0xF19D56, striped: true },
        { index: 13, name: "Takasaki", color: 0xF19D56, striped: true },
        { index: 14, name: "Saikyo", color: 0x4DA986, striped: true },
        { index: 15, name: "Kawagoe", color: 0xA7A9AB, striped: false },
        { index: 16, name: "Musashino", color: 0xDA6529, striped: true },
        { index: 17, name: "Joetsu", color: 0x4DA9CD, striped: false },
        { index: 18, name: "Shin-Etsu", color: 0x85BF41, striped: false },
        { index: 19, name: "Agatsuma", color: 0x3A8488, striped: false },
        { index: 20, name: "Karasuyama", color: 0xEF8D3C, striped: false },
        { index: 21, name: "Hachiko", color: 0x9F9D96, striped: false },
        { index: 22, name: "Nikko", color: 0xEF8D3C, striped: false },
        { index: 23, name: "Ryomo", color: 0xF7D849, striped: false },
        { index: 24, name: "Chuo Rapid", color: 0xDA6529, striped: true },
        { index: 25, name: "Chuo-Sobu Local", color: 0xFBE64D, striped: true },
        { index: 26, name: "Chuo", color: 0x306FBF, striped: false },
        { index: 27, name: "Itsukaichi", color: 0xDA6529, striped: true },
        { index: 28, name: "Ome", color: 0xDA6529, striped: true },
        { index: 29, name: "Koumi", color: 0x4D8448, striped: false },
        { index: 30, name: "Joban", color: 0x306FBF, striped: false },
        { index: 31, name: "Joban Rapid", color: 0x57BE8E, striped: true },
        { index: 32, name: "Joban Local", color: 0x9E9E9F, striped: true },
        { index: 33, name: "Suigun", color: 0x4D8448, striped: false },
        { index: 34, name: "Mito", color: 0x306FBF, striped: false },
        { index: 35, name: "Sobu Rapid", color: 0x3172B8, striped: true },
        { index: 36, name: "Sobu", color: 0xF7D849, striped: false },
        { index: 37, name: "Keiyo", color: 0xBE2D2F, striped: true },
        { index: 38, name: "Uchibo", color: 0x306FBF, striped: false },
        { index: 39, name: "Kashima", color: 0x9E6327, striped: false },
        { index: 40, name: "Kururi", color: 0x58C0A8, striped: false },
        { index: 41, name: "Sotobo", color: 0xDE3C3E, striped: false },
        { index: 42, name: "Togane", color: 0xA42D36, striped: false },
        { index: 43, name: "Narita", color: 0x54B889, striped: false },
        { index: 44, name: "Monorail", color: 0x133581, striped: true },
    ];

    protected lineIdToIndex: { [key: string]: number } = {
        'yamanoteline': 0,
        'ueno-tokyoline': 1,
        'shonan-shinjukuline': 2,
        'sotetsuline': 3,
        'tokaidoline': 4,
        'keihin-tohokuline': 5,
        'yokosukaline': 6,
        'nambuline': 7,
        'yokohamaline': 8,
        'itoline': 9,
        'sagamiline': 10,
        'tsurumiline': 11,
        'utsunomiyaline': 12,
        'takasakiline': 13,
        'saikyoline': 14,
        'kawagoeline': 15,
        'musashinoline': 16,
        'joetsuline': 17,
        'shin-etsuline_kanto': 18,
        'agatsumaline': 19,
        'karasuyamaline': 20,
        'hachikoline': 21,
        'nikkoline': 22,
        'ryomoline': 23,
        'chuoline_rapidservice': 24,
        'chuo_sobuline_local': 25,
        'chuoline': 26,
        'itsukaichiline': 27,
        'omeline': 28,
        'koumiline': 29,
        'jobanline': 30,
        'jobanline_rapidservice': 31,
        'jobanline_local': 32,
        'suigunline': 33,
        'mitoline': 34,
        'sobuline_rapidservice': 35,
        'sobuline': 36,
        'keiyoline': 37,
        'uchiboline': 38,
        'kashimaline': 39,
        'kururiline': 40,
        'sotoboline': 41,
        'toganeline': 42,
        'naritaline': 43,
        'monorail': 44,
    };

    /**
     * Map JR East status strings to StatusSeverity
     */
    protected mapSeverity(status: string): StatusSeverity {
        if (status.includes('Delays') || status.includes('Notice')) {
            return StatusSeverity.Warning;
        }
        if (status.includes('Disruptions')) {
            return StatusSeverity.Severe;
        }
        return StatusSeverity.Good;
    }

    /**
     * Extracts the line status containers from HTML
     */
    private extractLines(html: string): string[] {
        const parentContainerRegex = /<div class="rosenBox">[\s\S]*?<div class="rosen_infoBox">[\s\S]*?<\/div>\s*<\/div>/g;
        return html.match(parentContainerRegex) || [];
    }

    /**
     * Parse status and reason for a single line
     */
    private parseSimpleLineStatus(boxContent: string): SimpleLineStatus | null {
        const statusMatch = boxContent.match(/<div class="status (normal|delay|info|adjust)">/);
        if (!statusMatch) return null;

        const statusType = statusMatch[1];
        const statusTextMatch = boxContent.match(/<p class="status_Text">(.*?)<\/p>/);
        const reason = statusTextMatch ? statusTextMatch[1] : undefined;

        return { statusType, reason };
    }

    /**
     * Map status type to status description
     */
    private getStatusDescription(statusType: string): string {
        const statusMap: { [key: string]: string } = {
            normal: 'Good Service',
            delay: 'Delays',
            info: 'Notice',
            adjust: 'Disruptions',
        };
        return statusMap[statusType] || statusType;
    }

    /**
     * Build mapping of line IDs to their statuses from HTML
     */
    private buildLineIdStatusMap(pageHtml: string, extractLineId: boolean = true): Array<{ lineId: string; status: SimpleLineStatus } | null> {
        const boxes = this.extractLines(pageHtml);
        return boxes.map((boxContent) => {
            let lineId: string | null = null;

            if (extractLineId) {
                const lineIdMatch = boxContent.match(/lineid=([^"&<>]+)/);
                if (!lineIdMatch) return null;
                lineId = lineIdMatch[1];
            }

            const status = this.parseSimpleLineStatus(boxContent);
            if (!status) return null;

            return { lineId: lineId!, status };
        });
    }

    /**
     * Parse train line data from HTML
     * Extracts boxes from both Japanese and English, maintaining index alignment
     */
    private parseLineData(...args: { mode: string, html: string }[]): Array<LineStatus & { configIndex: number }> {
        const jpHtml = args.find(arg => arg.mode === '')?.html || '';
        const enHtml = args.find(arg => arg.mode === 'e')?.html || '';

        // Parse both languages' boxes
        const jpBoxes = this.buildLineIdStatusMap(jpHtml, true);
        const enBoxes = this.buildLineIdStatusMap(enHtml, false);

        if (jpBoxes.length !== enBoxes.length) {
            console.warn(`Box count mismatch: ${jpBoxes.length} JP boxes, ${enBoxes.length} EN boxes, returning only JP data.`);
            return jpBoxes
                .filter(Boolean)
                .map((box, i) => {
                    if (!box?.lineId) return null;
                    const configIndex = this.lineIdToIndex[box.lineId];
                    if (configIndex === undefined) return null;

                    const status = this.getStatusDescription(box.status.statusType);
                    return {
                        index: i,
                        configIndex,
                        status,
                        severity: this.mapSeverity(status),
                        reason: this.truncateReason(box.status.reason || ''),
                    };
                })
                .filter((line): line is NonNullable<typeof line> => line !== null);
        }

        const result: Array<LineStatus & { configIndex: number }> = [];

        // Combine data ensuring line IDs from Japanese version are used
        const maxLength = Math.max(jpBoxes.length, enBoxes.length);
        for (let i = 0; i < maxLength; i++) {
            const jpBox = jpBoxes[i];
            const enBox = enBoxes[i];

            // Skip if no Japanese box, as line IDs are essential
            if (!jpBox) {
                console.warn(`Skipping index ${i} due to missing Japanese box.`);
                continue;
            }

            const configIndex = this.lineIdToIndex[jpBox.lineId];
            if (configIndex === undefined) {
                continue;
            }

            // Use English status and reason if available, fall back to Japanese
            const statusType = enBox ? enBox.status.statusType : jpBox.status.statusType;
            const status = this.getStatusDescription(statusType);
            const reason = (enBox?.status.reason || jpBox.status.reason || '');

            result.push({
                index: result.length,
                configIndex,
                status,
                severity: this.mapSeverity(status),
                reason: this.truncateReason(reason),
            });
        }

        return result;
    }

    /**
     * Fetch current line statuses from JR East website
     */
    async fetchLines(): Promise<LineStatus[] & { configMapping: number[] }> {
        // Fetch both Japanese and English pages
        const [result1, result2] = await Promise.all(['', 'e'].map(async (mode) => ({
            mode,
            html: await PebbleTS.fetchString(`${this.baseUrl}/${mode}/kanto.aspx`)
        })));

        const linesWithIssues = this.parseLineData(result1, result2);

        // Filter only lines with issues
        const filtered = linesWithIssues.filter(line => line.status !== 'Good Service');

        // Build result with sequential display indices
        const result: Array<LineStatus & { configIndex?: number }> = filtered.map((line, i) => ({
            index: i,
            status: line.status,
            severity: line.severity,
            reason: line.reason,
            configIndex: line.configIndex,
        }));

        // Add config mapping
        const typedResult = result as LineStatus[] & { configMapping: number[] };
        typedResult.configMapping = result.map(r => r.configIndex!);
        return typedResult;
    }
}

/**
 * Export singleton instance
 */
export const jrEastKantoBackend = new JrEastKantoBackend();
