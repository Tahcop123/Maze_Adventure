#ifndef _FPVIEW_H
#define _FPVIEW_H

#include "inc.h"

// Initialize first person view
void InitFPView(void);

// Update FP view (rotation, etc.)
void UpdateFPView(float dt);

// Render first person view using raycasting
void DrawFPView(void);

// Toggle between topdown and first person
void ToggleFPMode(void);

// Rotate view by angle (radians)
void RotateView(float angle);

#endif
