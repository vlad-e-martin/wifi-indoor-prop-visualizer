const express = require('express');
const path = require('path');
const fs = require('fs');

const repoRoot = path.resolve(__dirname, '../../');
console.log(`[i] Found the repository root directory: ${repoRoot}`);

console.log("[i] Attempting to load rf_simulator N-API binary");

// Load the compiled C++ N-API binary from cmake-js' output build directory
const rfSimulatorPath = path.join(repoRoot, 'build', 'Release', 'rf_simulator.node');

// prevent silent crashes if the path is wrong
if (!fs.existsSync(rfSimulatorPath)) {
    console.error(`\n[!] CRITICAL ERROR: Could not find the compiled binary.`);
    console.error(`[!] Looked in: ${rfSimulatorPath}`);
    console.error(`[!] NOTE: Please run 'npm run build' first to generate the .node file.\n`);
    process.exit(1);
}

const rfSimulator = require(rfSimulatorPath);
console.log("[i] Successfully loaded rf_simulator N-API binary");

const app = express();
const PORT = process.env.PORT || 3000;

// Middleware to parse incoming JSON payloads
app.use(express.json({ limit: '10mb' }));

// Serve the frontend web files from the public/ directory
app.use(express.static(path.join(__dirname, 'public')));

// Define the simulation endpoint
app.post('/api/simulate', (req, res) => {
    try {
        const { layout, txX, txY, txZ, freq, power, gridW, gridH, res: resolution } = req.body;

        const simulator = new rfSimulator.IndoorSimulator(layout, 3.0);
        const heatmapFloat64Array = simulator.generateHeatmap(
            txX, 
            txY, 
            txZ, 
            freq, 
            power, 
            gridW, 
            gridH, 
            resolution
        );

        // Re-format the output as a JS array
        const serializedHeatmap = Array.from(heatmapFloat64Array);
        res.json(serializedHeatmap);

    } catch (error) {
        console.error("Simulation failed on the backend:", error);
        res.status(500).json({ error: error.message });
    }
});

// Start the server to await calls to run a simulation
app.listen(PORT, () => {
    console.log(`=========================================`);
    console.log(` RF Simulator Server running!            `);
    console.log(` Access UI at: http://localhost:${PORT}  `);
    console.log(`=========================================`);
});