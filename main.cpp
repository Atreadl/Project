//
// File: ert_main.cpp
//
// Code generated for Simulink model 'harvester'.
//
// Model version                  : 5.112
// Simulink Coder version         : 25.2 (R2025b) 28-Jul-2025
// C/C++ source code generated on : Tue May  5 23:34:15 2026
//
// Target selection: ert.tlc
// Embedded hardware selection: Intel->x86-64 (Windows64)
// Code generation objectives:
//    1. Execution efficiency
//    2. RAM efficiency
// Validation result: Not run
//
#include <iostream>
#include <stdio.h>              // This example main program uses printf/fflush
#include "harvester.h"                 // Model header file
#include <windows.h>

static harvester harvester_Obj;        // Instance of model class

//
// Associating rt_OneStep with a real-time clock or interrupt service routine
// is what makes the generated code "real-time".  The function rt_OneStep is
// always associated with the base rate of the model.  Subrates are managed
// by the base rate from inside the generated code.  Enabling/disabling
// interrupts and floating point context switches are target specific.  This
// example code indicates where these should take place relative to executing
// the generated code step function.  Overrun behavior should be tailored to
// your application needs.  This example simply sets an error status in the
// real-time model and returns from rt_OneStep.
//
void rt_OneStep(void);
void rt_OneStep(void)
{
  static boolean_T OverrunFlag{ false };

  // Disable interrupts here

  // Check for overrun
  if (OverrunFlag) {
    return;
  }

  OverrunFlag = true;

  // Save FPU context here (if necessary)
  // Re-enable timer or interrupt here
  // Set model inputs here

  // Step the model
  harvester_Obj.step();

  // Get model outputs here

  // Indicate task complete
  OverrunFlag = false;

  // Disable interrupts here
  // Restore FPU context here (if necessary)
  // Enable interrupts here
}

//
// The example main function illustrates what is required by your
// application code to initialize, execute, and terminate the generated code.
// Attaching rt_OneStep to a real-time clock is target specific. This example
// illustrates how you do this relative to initializing the model.
//
int_T main(int_T argc, const char *argv[])
{
  harvester_Obj.harvester_U.sEngine = true;
  harvester_Obj.harvester_U.nEngine = 2500.0;

  harvester_Obj.harvester_U.sDrum = true;          // БЫЛО НЕ ЗАДАНО!
  harvester_Obj.harvester_U.nDrum = 1500.0;        // БЫЛО НЕ ЗАДАНО!
  harvester_Obj.harvester_U.sHeader = true;        // БЫЛО НЕ ЗАДАНО!
  harvester_Obj.harvester_U.nHeader = 100.0;      // БЫЛО НЕ ЗАДАНО!
  harvester_Obj.harvester_U.sGrainConveyor = true;

  int step = 0;
  // Unused arguments
  (void)(argc);
  (void)(argv);

  // Initialize model
  harvester_Obj.initialize();


  printf("Warning: The simulation will run forever. "
         "Generated ERT main won't simulate model step behavior. "
         "To change this behavior select the 'MAT-file logging' option.\n");
  fflush((nullptr));


  for (int i = 0; i < 100; i++) {
    rt_OneStep();
    std::cout << "RPM: " << harvester_Obj.harvester_Y.totalDrop << "\n";
    step++;
    Sleep(100); // <-- ПАУЗА 100 мс. Период модели 0.1 сек = 100 мс.
  }

  // The option 'Remove error status field in real-time model data structure'
  //  is selected, therefore the following code does not need to execute.

  return 0;
}

//
// File trailer for generated code.
//
// [EOF]
//
