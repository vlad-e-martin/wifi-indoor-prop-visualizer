// src/js/public/app.js

import { initDrawingCanvas, clearAllLines, getWallCoordinates, 
    setActiveMaterial, WALL_TYPES } from './drawing.js';

import { runSimulation } from './simulation.js';

// DOM Elements
const imageUpload = document.getElementById('imageUpload');
const jsonUpload = document.getElementById('jsonUpload');
const drawingModal = document.getElementById('drawingModal');
const saveWallsBtn = document.getElementById('saveWallsBtn');

// Material UI Elements
const materialSelector = document.getElementById('materialSelector');
const materialTracker = document.getElementById('activeMaterialTracker');
const matButtons = materialSelector.querySelectorAll('.mat-btn');

const step2Container = document.getElementById('step2-container');
const step3Container = document.getElementById('step3-container');

// Router UI Elements
const routerXInput = document.getElementById('routerX');
const routerYInput = document.getElementById('routerY');

// Simulate Elements
const simulateBtn = document.getElementById('runSimBtn');

// State to hold walls globally in the app
let activeWallsData = [];
let mainCanvas = null;
let routerPhantom = null;
let routerPin = null;

// Main Workspace Setup + Router Placement Logic
function initMainWorkspace() {
    if (mainCanvas) return; // Already initialized

    mainCanvas = new fabric.Canvas('mainWorkspaceCanvas', {
        selection: false,
        hoverCursor: 'crosshair'
    });

    // Create the semi-transparent hover icon
    routerPhantom = new fabric.Circle({
        radius: 8, fill: '#3b82f6', opacity: 0.5,
        originX: 'center', originY: 'center',
        selectable: false, evented: false, visible: false
    });
    mainCanvas.add(routerPhantom);

    // Create the solid placed icon
    routerPin = new fabric.Circle({
        radius: 8, fill: '#1d4ed8', stroke: '#ffffff', strokeWidth: 2,
        originX: 'center', originY: 'center',
        selectable: false, evented: false, visible: false,
        shadow: new fabric.Shadow({ color: 'rgba(0,0,0,0.5)', blur: 4 })
    });
    mainCanvas.add(routerPin);

    // Hover effect: Phantom follows mouse
    mainCanvas.on('mouse:move', function(o) {
        if (!routerPhantom) return;
        const ptr = mainCanvas.getPointer(o.e);
        routerPhantom.set({ left: ptr.x, top: ptr.y, visible: true });
        mainCanvas.renderAll();
    });

    // Hide phantom if mouse leaves canvas
    mainCanvas.on('mouse:out', function() {
        if (!routerPhantom) return;
        routerPhantom.set({ visible: false });
        mainCanvas.renderAll();
    });

    // Click: Place the permanent router pin and update UI
    mainCanvas.on('mouse:down', function(o) {
        if (!routerPin) return;
        const ptr = mainCanvas.getPointer(o.e);
        
        routerPin.set({ left: ptr.x, top: ptr.y, visible: true });
        
        // Ensure pins always stay on top of the walls
        mainCanvas.bringToFront(routerPin);
        mainCanvas.bringToFront(routerPhantom);
        
        // Update the Sidebar UI
        routerXInput.value = Math.round(ptr.x);
        routerYInput.value = Math.round(ptr.y);
        
        mainCanvas.renderAll();
    });
}

function renderLayoutToWorkspace() {
    initMainWorkspace();

    // Clear pre-existing floor plan but keep the router placement
    const objects = mainCanvas.getObjects().filter(obj => obj !== routerPhantom && obj !== routerPin);
    objects.forEach(obj => mainCanvas.remove(obj));

    // Reset router pin if layout changes
    routerPin.visible = false;
    routerXInput.value = "";
    routerYInput.value = "";

    // Find the bounding box of the complete floor plan
    let maxX = 0;
    let maxY = 0;
    activeWallsData.forEach(wall => {
        maxX = Math.max(maxX, wall.x1, wall.x2);
        maxY = Math.max(maxY, wall.y1, wall.y2);
    });

    // Resize canvas to fit the floor plan
    mainCanvas.setDimensions({
        width: maxX + 100,
        height: maxY + 100
    });

    // Set canvas background color to white
    mainCanvas.setBackgroundColor('#ffffff', mainCanvas.renderAll.bind(mainCanvas));

    // Draw walls
    activeWallsData.forEach(wall => {
        // Fallback to Drywall if material is missing for some reason
        const config = WALL_TYPES[wall.material] || WALL_TYPES.DRYWALL; 
        
        const line = new fabric.Line([wall.x1, wall.y1, wall.x2, wall.y2], {
            strokeWidth: config.thickness,
            fill: config.color,
            stroke: config.color,
            originX: 'center', originY: 'center',
            selectable: false, evented: false
        });
        mainCanvas.add(line);
    });

    // Ensure pins are layered correctly
    mainCanvas.bringToFront(routerPin);
    mainCanvas.bringToFront(routerPhantom);
    
    // Force Fabric to recalculate physical mouse boundaries
    mainCanvas.calcOffset();
    mainCanvas.renderAll();
}

// UI State Management
function unlockNextSteps() {
    // Remove the classes that grey out and disable clicks
    step2Container.classList.remove('opacity-50', 'pointer-events-none');
    step3Container.classList.remove('opacity-50', 'pointer-events-none');
    // Render floorplan and allow router placement
    renderLayoutToWorkspace();
}

// Handle File Upload
imageUpload.addEventListener('change', function(e) {
    const file = e.target.files[0];
    if (!file) return;

    // Define allowed MIME types for floor plans
    const validImageTypes = ['image/jpeg', 'image/png', 'image/webp'];

    // Check if the uploaded file's type is in our allowed list
    if (!validImageTypes.includes(file.type)) {
        alert("Invalid file type. Please upload a valid image (JPG, PNG, or WEBP).");
        // Clear the input so the user can select a new file
        e.target.value = ''; 
        return; 
    }

    // Enforce maximum file size (attempting to prevent browser lag)
    const maxSizeInBytes = 20 * 1024 * 1024; // 20 MB
    if (file.size > maxSizeInBytes) {
        alert("File is too large. Please upload an image smaller than 20MB.");
        e.target.value = '';
        return;
    }

    const reader = new FileReader();
    reader.onload = function(f) {
        const base64Data = f.target.result;
        // Pass image into the drawing module for rendering
        initDrawingCanvas(base64Data, drawingModal);
    };
    reader.readAsDataURL(file);
    
    // Clear the input after loading so the user can re-upload files as needed
    e.target.value = '';
});

// Handle Material Selection Clicks (Segmented Control)
matButtons.forEach(button => {
    button.addEventListener('click', (e) => {
        const selectedMaterialKey = e.target.getAttribute('data-material');
        
        // Update state in drawing.js and get config
        const config = setActiveMaterial(selectedMaterialKey);
        
        if (config) {
            // Update the Tracking Text Box
            materialTracker.innerHTML = `Currently Drawing: <strong>${config.name}</strong> (Color: <span style="color:${config.color}">■</span>, Thickness: ${config.thickness}px)`;
            
            // Update button visual state (managed via Tailwind rings)
            matButtons.forEach(btn => btn.classList.remove('active', 'ring-2', 'ring-white'));
            e.target.classList.add('active', 'ring-2', 'ring-white');
        }
    });
});

// Clear Lines
clearLinesBtn.addEventListener('click', () => {
    clearAllLines();
});

jsonUpload.addEventListener('change', function(e) {
    const file = e.target.files[0];
    if (!file) return;

    if (file.type !== "application/json" && !file.name.endsWith('.json')) {
        alert("Please upload a valid JSON layout file.");
        e.target.value = '';
        return;
    }

    const reader = new FileReader();
    reader.onload = function(f) {
        try {
            const parsedData = JSON.parse(f.target.result);
            
            // Basic validation to ensure it's our layout format
            if (!Array.isArray(parsedData) || (parsedData.length > 0 && !('material' in parsedData[0]))) {
                throw new Error("Invalid format");
            }
            
            activeWallsData = parsedData;
            unlockNextSteps(); // Unlocks other components of the UI and draws the saved floorplan
            
        } catch (error) {
            alert("Error parsing layout file. Ensure it was generated by this simulator.");
            console.error(error);
        }
    };
    reader.readAsText(file);
    e.target.value = ''; // Reset input
});

// Save Walls from the Drawing modal, move on to next steps
saveWallsBtn.addEventListener('click', () => {
    // Extract the wall data from the canvas first!
    activeWallsData = getWallCoordinates();
    
    // Check if they actually drew anything
    if (activeWallsData.length === 0) {
        alert("You haven't drawn any walls!");
        return;
    }
    
    // Convert array to a formatted JSON string
    const jsonString = JSON.stringify(activeWallsData, null, 2);
    
    // Create a Blob and a temporary download link
    const blob = new Blob([jsonString], { type: "application/json" });
    const url = URL.createObjectURL(blob);
    
    const a = document.createElement('a');
    a.href = url;

    // The browser will select the save location for us
    a.download = "wifi_floorplan_layout.json"; // Default file name
    document.body.appendChild(a);

    // Trigger the download to the local system
    a.click();
    
    // Cleanup memory
    document.body.removeChild(a);
    URL.revokeObjectURL(url);

    // Transition the UI
    drawingModal.classList.add('hidden');
    unlockNextSteps(); // Unlock rest of the screen for next steps
});

simulateBtn.addEventListener('click', () => {
    // Pass the current app state into the simulation module
    runSimulation(
        activeWallsData, 
        routerXInput.value, 
        routerYInput.value, 
        mainCanvas, 
        simulateBtn
    );
});