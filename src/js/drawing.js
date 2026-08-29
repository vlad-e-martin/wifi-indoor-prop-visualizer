// src/js/drawing.js

// Define Material Configs (Tailwind-like colors for consistency)
const WALL_TYPES = {
    CONCRETE: { name: "Reinforced Concrete", color: "#4b5563", thickness: 10 }, // slate-600
    BRICK:    { name: "Brick / Masonry",     color: "#b91c1c", thickness: 8 },  // red-700
    WOOD:     { name: "Solid Wood",          color: "#92400e", thickness: 5 },  // amber-900
    DRYWALL:  { name: "Drywall w/ Studs",    color: "#0f172a", thickness: 3 },  // slate-900
};

// Set the active material (default to DRYWALL)
let activeMaterialKey = "DRYWALL";

let canvas;
let isDrawing = false;
let currentLine = null;

export function initDrawingCanvas(imageUrl, modalElement) {
    // Unhide the modal before initializing the canvas
    // so Fabric.js can accurately measure the DOM dimensions.
    modalElement.classList.remove('hidden');

    // Give the browser 100ms to actually render the modal to the screen
    setTimeout(() => {
        // Initialize canvas
        if (!canvas) {
            canvas = new fabric.Canvas('drawCanvas', { 
                selection: false,
                preserveObjectStacking: true // Keeps image at the bottom
            });
            setupDrawingLogic();
        }
    
        console.log("Attempting to load image from Blob URL:", imageUrl);

        // 2. Load the image with an error-handling callback
        fabric.Image.fromURL(imageUrl, function(img, isError) {
            // Debugging check
            if (isError || !img) {
                console.error("Fabric.js failed to load the image.");
                alert("Could not render the image on the canvas.");
                return;
            }

            console.log("Image loaded successfully. Original size:", img.width, "x", img.height);

            const maxWidth = window.innerWidth * 0.8;
            const maxHeight = window.innerHeight * 0.7; 
            const scale = Math.min(maxWidth / img.width, maxHeight / img.height, 1);
            
            // Set new dimensions
            canvas.setDimensions({
                width: (img.width * scale),
                height: (img.height * scale)
            });
            
            img.set({
                scaleX: scale,
                scaleY: scale,
                originX: 'left',
                originY: 'top',
                selectable: false,
                evented: false, // Allows clicks to pass through to the canvas
                isFloorPlanImage: true // Tag it so we don't accidentally delete it
            });
            
            canvas.add(img);
            canvas.sendToBack(img);
            canvas.calcOffset(); // Recalculate mouse boundaries
            canvas.renderAll();

            // Debug visual: Give the canvas a red border so we know exactly where it is
            canvas.getElement().parentNode.style.border = "4px solid red";
            
            console.log("Image successfully rendered on canvas.");
        });
    }, 100); // 100ms delay to ensure DOM is fully ready
}

// Allow the active material to be updated while the user interacts with the canvas
export function setActiveMaterial(materialKey) {
    if (WALL_TYPES[materialKey]) {
        activeMaterialKey = materialKey;
        return WALL_TYPES[materialKey]; // Return the config for the UI tracker
    }
    return null;
}

function setupDrawingLogic() {
    // Helper function to cleanly start a new line
    function beginLine(x, y) {
        const points = [x, y, x, y];
        // Use active material's properties for the new line
        const config = WALL_TYPES[activeMaterialKey];
        
        currentLine = new fabric.Line(points, {
            strokeWidth: config.thickness, // Dynamic thickness to match material
            fill: config.color,           // Dynamic color to match material
            stroke: config.color,
            originX: 'center',
            originY: 'center',
            selectable: false,
            evented: false,
            materialType: activeMaterialKey // Store dynamic property for easy extraction later
        });
        
        canvas.add(currentLine);
    }

    // Handle click & chaining of lines
    canvas.on('mouse:down', function(o){
        const pointer = canvas.getPointer(o.e);

        if (!isDrawing) {
            // Start the very first line
            isDrawing = true;
            beginLine(pointer.x, pointer.y);
        } else {
            // Finish and lock the current line
            currentLine.setCoords(); 
            
            // Optional cleanup: Prevent zero-length dots if the user double-clicks
            if (Math.abs(currentLine.x1 - currentLine.x2) < 5 && Math.abs(currentLine.y1 - currentLine.y2) < 5) {
                canvas.remove(currentLine);
            }

            // IMMEDIATELY start the next line connected to this endpoint
            beginLine(pointer.x, pointer.y);
        }
    });

    // Stretch the active line
    canvas.on('mouse:move', function(o) {
        if (!isDrawing || !currentLine) return;
        
        const pointer = canvas.getPointer(o.e);
        currentLine.set({ x2: pointer.x, y2: pointer.y });
        canvas.renderAll();
    });

    // Listen for the Escape key to break the chain
    window.addEventListener('keydown', function(e) {
        if (e.key === 'Escape' && isDrawing) {
            // Delete the actively stretching segment
            canvas.remove(currentLine); 
            isDrawing = false;
            currentLine = null;
            canvas.renderAll();
            console.log("Drawing chain canceled via Escape key.");
        }
    });
}

export function clearAllLines() {
    if (!canvas) return;
    // Clear all objects which are not the background image
    const objects = canvas.getObjects().filter(obj => !obj.isFloorPlanImage);
    objects.forEach(obj => canvas.remove(obj));
    isDrawing = false; 
    currentLine = null;
}

export function getWallCoordinates() {
    if (!canvas) return [];
    
    // Clean up an unfinished line if the user clicks Save while drawing
    if (isDrawing && currentLine) {
        canvas.remove(currentLine);
        isDrawing = false;
    }

    const wallObjects = canvas.getObjects('line');

    // Extract coordinates and material ID
    return wallObjects.map(line => ({
        x1: Math.round(line.x1),
        y1: Math.round(line.y1),
        x2: Math.round(line.x2),
        y2: Math.round(line.y2),
        material: line.materialType // C++ backend needs material info for attenuation calculations
    }));
}