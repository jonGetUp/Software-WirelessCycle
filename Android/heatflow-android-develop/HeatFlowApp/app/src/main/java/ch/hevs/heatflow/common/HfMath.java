package ch.hevs.heatflow.common;

public final class HfMath {

    /**
     *  Constant from datasheet TE-PTF temperature sensors: a = 5.775E-07
     */
    private final static double _a              = 5.775E-07f;

    /**
     * Constant from datasheet TE-PTF temperature sensors: b = 3.9083E-03
     */
    private final static double _b              = 3.9083E-03f;

    /**
     * Resistance at zero degree C [Ohm]: R0 = 1E3
     */
    private final static double _R0             = 1E3f;

    /**
     * Measurement resistance in voltage divider [Ohm]: RM = 1.02E4
     */
    private final static double _RM             = 1.02E4f;

    /**
     * Reference voltage in voltage divider [V]: Vref = 2.5
     */
    private final static double _Vref           = 2.5f;

    /**
     * Temperature dependant sensitivity constant [V/(deg*W/m^2)]: Sc = 7.30E-8
     */
    private final static double _Sc             = 7.30E-8f;

    /**
     * Temperature independent sensitivity constant [V/(W/m^2)]: S0 = 59.87E-6
     */
    private final static double _S0             = 59.87E-6;

    /**
     * Reference temperature [degree C]: T0 = 22.5
     */
    private final static double _T0             = 22.5;

    /**
     * Sensitivity of heat flow sensor [V/(W/m^2)]:
     */


    /**
     * Compute the temperature form the voltage measured by the ADS using the following formula:
     *   RT = VT * RM/(Vref-VT)                      (Resistance of temperature sensor [Ohm])
     *   T = (b - sqrt(b*b-4*a*(RT/R0-1)))/(2*a)     (Temperature [degree C])
     * Where VT is
     *
     * @param   voltage the voltage from the ADS1118 in [V]
     * @return  The temperature measured by the sensor...
     */
    public static double TemperatureFromVoltage(float voltage) {

        // RT = VT * RM/(Vref-VT)
        double RT = voltage * _RM / (_Vref - voltage);

        // T = (b - sqrt(b*b-4*a*(RT/R0-1)))/(2*a)
        double T = (_b - Math.sqrt(_b*_b - 4*_a*(RT/_R0-1)))/(2*_a);

        return T;
    }

    /**
     * Compute the Heatflow from the measured voltage and temperature using the following formula:
     *   S = S0 + (T-T0)*Sc;    (Sensitivity of heat flow sensor [V/(W/m^2)])
     *   Q = VQ/S;              (Heat flow to be displayed (W/m^2)
     * @param VQ    The voltage measured by the ADS
     * @param T1    One of the measured temperature
     * @param T2    The other one
     * @return  The heatflow resulting...
     */
    public static double HeatflowFromTemperature(float VQ, double T1, double T2) {
        double T  = Math.max(T1, T2);
        double S  = _S0 + (T - _T0) * _Sc;
        double Q  = VQ / S;

        return Q;
    }


}
