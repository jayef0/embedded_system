#include <stdio.h>
#include <math.h>

static float Tc = 345.28;
static float X0 = 2.086902e-1;
static float F[6] = {-1.440004, -6.865265, -5.354309e-1, -3.749023, -3.521484, -7.750000};
static float Pc = 4926.1;

/* compute saturation pressure bassed on temperature */
float ref_410a_sat_pressure(float temp_C)

{
    float sat_pressure = -1;
    float temp_K = temp_C+273.15;

    /* check temperature is in valid region */
    if(temp_K >= 173)
    {
        float Tr = temp_K/Tc;
        float X = 1.0-Tr-X0;

        // COmpute powers of X
        float X2 = X*X;
        float X3 = X2*X;
        float X4 = X3*X;
        float X5 = X4*X;

        // compute Psat
        float fx = F[0] + F[1]*X + F[2]*X2 + F[3]*X3 + F[4]*X4 + F[5]*X5;
        sat_pressure = Pc*expf(fx/Tr);
    }

    return sat_pressure;


}


