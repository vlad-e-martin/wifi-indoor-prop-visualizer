// src/js/simulation.js

import { dbmToColor } from './colorMap.js';

// Simulation & Scaling Configuration
const PIXELS_PER_METER = 20; // TODO: Request floorplan dimensions from user
const RESOLUTION_M = 0.5;    // Calculate signal every half-meter
const GRID_WIDTH = 100;      // 50x50 physical meter simulation grid
const GRID_HEIGHT = 100;

/**
 * Executes the C++ simulation via the backend API and renders the result.
 */
export async function runSimulation(activeWallsData, routerX, routerY, mainCanvas, simulateBtn) {
    if (activeWallsData.length === 0) {
        alert("Please load or draw a floor plan first.");
        return;
    }
    if (!routerX || !routerY) {
        alert("Please click on the workspace to place the router.");
        return;
    }

    // 1. Scale the pixel walls down to physical meters for the C++ engine
    const scaledWalls = activeWallsData.map(wall => ({
        x1: wall.x1 / PIXELS_PER_METER,
        y1: wall.y1 / PIXELS_PER_METER,
        x2: wall.x2 / PIXELS_PER_METER,
        y2: wall.y2 / PIXELS_PER_METER,
        material: wall.material
    }));

    // 2. Prepare the payload
    const simParams = {
        layout: JSON.stringify(scaledWalls),
        txX: parseFloat(routerX) / PIXELS_PER_METER, 
        txY: parseFloat(routerY) / PIXELS_PER_METER,
        txZ: 1.5,       // Standard router height
        freq: 2.4,      // 2.4 GHz
        power: 20.0,    // 20 dBm (100mW)
        gridW: GRID_WIDTH,
        gridH: GRID_HEIGHT,
        res: RESOLUTION_M
    };

    try {
        if (simulateBtn) {
            simulateBtn.disabled = true;
            simulateBtn.innerText = "Simulating...";
        }

        // 3. Post to the Node.js Express server
        const response = await fetch('/api/simulate', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(simParams)
        });

        if (!response.ok) throw new Error("Server failed to run simulation.");
        
        // 4. Receive the flat array and render it
        const heatmapArray = await response.json();
        renderHeatmap(heatmapArray, mainCanvas);

    } catch (error) {
        console.error("Simulation Error:", error);
        alert("An error occurred while running the simulation.");
    } finally {
        if (simulateBtn) {
            simulateBtn.disabled = false;
            simulateBtn.innerText = "Run Simulation";
        }
    }
}

/**
 * Renders the flat dBm array to an offscreen canvas and injects it into Fabric.js
 */
function renderHeatmap(heatmapArray, mainCanvas) {
    // Create off-screen HTML5 canvas to start drawing the heatmap onto
    const offscreenCanvas = document.createElement('canvas');
    offscreenCanvas.width = GRID_WIDTH * RESOLUTION_M * PIXELS_PER_METER;
    offscreenCanvas.height = GRID_HEIGHT * RESOLUTION_M * PIXELS_PER_METER;
    const ctx = offscreenCanvas.getContext('2d', { alpha: true });

    const rectPixelSize = RESOLUTION_M * PIXELS_PER_METER;

    // Convert received power levels into a colored heatmap
    for (let i = 0; i < heatmapArray.length; i++) {
        const gridX = i % GRID_WIDTH;
        const gridY = Math.floor(i / GRID_WIDTH);
        
        ctx.fillStyle = dbmToColor(heatmapArray[i]); 
        ctx.fillRect(
            gridX * rectPixelSize, 
            gridY * rectPixelSize, 
            rectPixelSize, 
            rectPixelSize
        );
    }

    // Convert the painted canvas into a Fabric.js Image
    fabric.Image.fromURL(offscreenCanvas.toDataURL(), function(img) {
        img.set({
            left: 0, 
            top: 0,
            selectable: false, 
            evented: false,
            isHeatmapLayer: true // Custom tag for easy cleanup
        });
        
        // Remove the previous heatmap if the user is running a second simulation
        const oldHeatmap = mainCanvas.getObjects().find(o => o.isHeatmapLayer);
        if (oldHeatmap) {
            mainCanvas.remove(oldHeatmap);
        }

        // Add to workspace and push it to the bottom so that the floorplan remains visible
        mainCanvas.add(img);
        img.sendToBack(); 
        
        mainCanvas.renderAll();
    });
}