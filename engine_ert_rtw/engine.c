/*
 * File: engine.c
 *
 * Code generated for Simulink model 'engine'.
 *
 * Model version                  : 1.1
 * Simulink Coder version         : 23.2 (R2023b) 01-Aug-2023
 * C/C++ source code generated on : Fri Apr 17 13:14:10 2026
 *
 * Target selection: ert.tlc
 * Embedded hardware selection: Intel->x86-64 (Windows64)
 * Code generation objectives:
 *    1. Execution efficiency
 *    2. RAM efficiency
 * Validation result: Not run
 */

#include "engine.h"

/* External inputs (root inport signals with default storage) */
ExtU rtU;

/* External outputs (root outports fed by signals with default storage) */
ExtY rtY;

/* Real-time model */
static RT_MODEL rtM_;
RT_MODEL *const rtM = &rtM_;

/* Model step function */
void engine_step(void)
{
  /* Product: '<Root>/Product' incorporates:
   *  Inport: '<Root>/In2'
   */
  if (rtU.In2) {
    /* Outport: '<Root>/Out1' incorporates:
     *  Inport: '<Root>/In1'
     */
    rtY.Out1 = rtU.In1;
  } else {
    /* Outport: '<Root>/Out1' */
    rtY.Out1 = 0U;
  }

  /* End of Product: '<Root>/Product' */
}

/* Model initialize function */
void engine_initialize(void)
{
  /* (no initialization code required) */
}

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
