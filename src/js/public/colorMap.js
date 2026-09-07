// src/js/public/colorMap.js

/**
 * Maps a dBm signal strength to an RGBA color string.
 * @param {number} dbm - The received power in dBm.
 * @returns {string} - A CSS-compatible hsla() color string.
 */
export function dbmToColor(dbm) {
    const maxSignal = -30; // Strongest expected signal (Blue)
    const minSignal = -90; // Noise floor / Dead zone (Red)

    // Clamp the value to ensure we don't exceed our bounds
    const clampedDbm = Math.max(minSignal, Math.min(maxSignal, dbm));

    // Calculate how strong the signal is as a percentage (0.0 to 1.0)
    const percent = (clampedDbm - minSignal) / (maxSignal - minSignal);

    // Map the percentage directly to the HSL color wheel
    const hue = percent * 240;

    // Use 0.7 opacity so the white background doesn't wash it out
    return `hsla(${hue}, 100%, 50%, 0.7)`;
}